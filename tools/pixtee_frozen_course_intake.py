#!/usr/bin/env python3
"""Read-only, fail-closed intake of the frozen Pixtee Work Hole 1 and approved art.

Does not change the original assets, local course, menus, engine or Godot project.
It validates the canonical ZIP SHA, CRC, mandatory Work documents, source tree,
and an optional trusted Work file-hash list, then writes ONLY an audit manifest
outside the frozen source. This does not turn an HTML hole into a playable course.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
from pathlib import Path
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
CANONICAL_ART = ROOT / "spec/pixtee/PIXTEE_APPROVED_ASSETS_CANONICAL_SHA256_2026-10-12.json"
REQUIRED_WORK = ("index.html", "COURSE_PRODUCTION_STANDARD.md", "COURSE_CATALOGUE_50.csv")
CHUNK = 1024 * 1024


class IntakeError(ValueError):
    pass


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for block in iter(lambda: file.read(CHUNK), b""):
            digest.update(block)
    return digest.hexdigest()


def scan_directory(root: Path) -> list[dict]:
    """Snapshot ALL regular files, including hidden dependency and checksum files.

    Ignore nothing. Reject symlinks and directory cycles/special files rather than
    silently omitting bytes that might be part of the frozen design.
    """
    if not root.is_dir() or root.is_symlink():
        raise IntakeError(f"Frozen source must be an existing real directory: {root}")
    outputs: list[dict] = []
    for file in sorted(root.rglob("*"), key=lambda x: x.as_posix().lower()):
        if file.is_symlink():
            raise IntakeError(f"Symlinks are forbidden in a frozen source: {file}")
        if file.is_dir():
            continue
        if not file.is_file():
            raise IntakeError(f"Unsupported special file inside source: {file}")
        relative = file.relative_to(root).as_posix()
        outputs.append({"path": relative, "sha256": sha256(file), "size_bytes": file.stat().st_size})
    names = {x["path"] for x in outputs}
    if len(names) != len(outputs):
        raise IntakeError("Duplicate relative paths in frozen folder")
    return outputs


def require_folder_files(files: list[dict], required: tuple[str, ...], label: str) -> None:
    paths = {f["path"] for f in files}
    missing = sorted(set(required) - paths)
    if missing:
        raise IntakeError(f"{label} missing immutable required files: {', '.join(missing)}")


def verify_expected_sha(files: list[dict], expected_manifest: Path) -> None:
    """Optional independent snapshot from Work; JSON: files [{path,sha256}].

    This is different from SHA computed in the same operation. An independent
    Work freeze manifest is required to prove a pre-existing source hash.
    """
    if not expected_manifest.is_file():
        raise IntakeError(f"Work-provided freeze-manifest missing: {expected_manifest}")
    data = json.loads(expected_manifest.read_text(encoding="utf-8-sig"))
    records = data.get("files", [])
    if not isinstance(records, list) or not records:
        raise IntakeError("Work freeze hash list must have non-empty files [{path,sha256}]")
    expected = {}
    for rec in records:
        if rec.get("group") not in (None, "frozen-hole-01"):
            continue  # Windows archive manifest also includes approved asset-pack entries
        path, value = rec.get("path") or rec.get("relative_path"), rec.get("sha256")
        if not isinstance(path, str) or not isinstance(value, str) or len(value) != 64:
            raise IntakeError("Invalid frozen hash record")
        if path in expected:
            raise IntakeError(f"Duplicate Work frozen path {path}")
        expected[path.replace("\\", "/")] = value.lower()
    found = {rec["path"]: rec["sha256"] for rec in files}
    if expected != found:
        missing = sorted(found.keys() - expected.keys())
        unexpected = sorted(expected.keys() - found.keys())
        differing = sorted(k for k in found.keys() & expected.keys() if expected[k] != found[k])
        raise IntakeError(
            f"Frozen Work files changed or hash list incomplete: "
            f"unlisted={missing[:6]}, nonexistent={unexpected[:6]}, modified={differing[:6]}"
        )


def validate_catalogue(path: Path) -> dict:
    """Check CSV can be read; don't assume Work's unprovided column schema."""
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        rows = csv.reader(stream)
        header = next(rows, None)
        if header is None or len(header) < 1 or not any(x.strip() for x in header):
            raise IntakeError("Course catalogue has no usable header")
        count = sum(1 for row in rows if any(cell.strip() for cell in row))
    if count < 1:
        raise IntakeError("Course catalogue is empty")
    return {"header": header, "data_rows": count, "catalogue_50_rows_confirmed": count == 50}


def validate_approved_zip(path: Path, canonical: dict) -> dict:
    if not path.is_file():
        raise IntakeError(f"Approved asset ZIP not found: {path}")
    digest = sha256(path)
    if digest != canonical.get("original_sha256"):
        raise IntakeError("Asset ZIP differs from the exact owner-approved canonical SHA-256")
    if path.stat().st_size != canonical.get("size_bytes"):
        raise IntakeError("Approved asset ZIP size differs from canonical")
    try:
        with zipfile.ZipFile(path) as archive:
            corrupted = archive.testzip()
            if corrupted is not None:
                raise IntakeError(f"Asset ZIP failed CRC: {corrupted}")
            names = archive.namelist()
    except zipfile.BadZipFile as exc:
        raise IntakeError("Approved asset package isn't a valid ZIP") from exc
    if len(names) != canonical.get("zip_member_count") or sorted(names) != sorted(canonical.get("zip_members", [])):
        raise IntakeError("Asset ZIP member inventory differs from owner-approved source")
    return {"sha256": digest, "size_bytes": path.stat().st_size, "members_verified": len(names)}


def within(path: Path, parent: Path) -> bool:
    try:
        path.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


def run(course: Path, assets: Path, menus: Path | None, hashes: Path | None,
        output: Path | None) -> dict:
    if course.is_symlink() or assets.is_symlink():
        raise IntakeError("Frozen root and approved asset ZIP must not be symlinks")
    course = course.resolve()
    assets = assets.resolve()
    if not CANONICAL_ART.is_file():
        raise IntakeError("Canonical approved-art manifest not found in source control")
    if output is not None:
        # Intake may only emit an independent audit report outside ALL input roots.
        dest = output.resolve()
        if within(dest, course) or (menus and menus.is_dir() and within(dest, menus)):
            raise IntakeError("Refusing to write report inside a frozen source directory")
        if dest == assets or (hashes and dest == hashes.resolve()):
            raise IntakeError("Refusing to overwrite an approved or hash-source input")
        if dest.exists():
            raise IntakeError("Refusing to overwrite a prior freeze intake report")

    source_files = scan_directory(course)
    require_folder_files(source_files, REQUIRED_WORK, "Frozen Hole 1")
    catalogue = validate_catalogue(course / "COURSE_CATALOGUE_50.csv")
    if hashes is not None:
        verify_expected_sha(source_files, hashes.resolve())
    canonical = json.loads(CANONICAL_ART.read_text(encoding="utf-8"))
    asset_verification = validate_approved_zip(assets, canonical)
    menu_files = []
    menu_status = "NOT_PROVIDED"
    if menus:
        if menus.is_file() and not menus.is_symlink():
            menu_files = [{"path": menus.name, "sha256": sha256(menus), "size_bytes": menus.stat().st_size}]
        elif menus.is_dir() and not menus.is_symlink():
            menu_files = scan_directory(menus)
            if not menu_files:
                raise IntakeError("Provided menu directory is empty")
        else:
            raise IntakeError("Menu path is missing or symlinked")
        menu_status = "HASH_RECORDED_NOT_VISUALLY_APPROVED"

    report = {
        "schema": "pixtee.frozen-course-intake.v1",
        "result": "SOURCE_BYTES_VERIFIED_IN_PLACE_NOT_IMPORT_READY",
        "source_course_directory": str(course),
        "source_files": source_files,
        "source_file_count": len(source_files),
        "work_independent_hash_manifest_verified": hashes is not None,
        "catalogue": catalogue,
        "approved_asset_zip": str(assets),
        "approved_art": asset_verification,
        "menu_status": menu_status,
        "menu_files": menu_files,
        "native_collision_geometry_extracted": False,
        "html_course_converted_to_godot": False,
        "three_playable_holes_completed": False,
        "commercial_rights_cleared": False,
        "input_files_modified": False,
        "notes": (
            "An input hash/CRC audit is not game physics parity or legal signoff. "
            "The Work-approved HTML remains the visual authority. Produce any "
            "Godot-native course resource on a SEPARATE feature branch."
        ),
    }
    if output is not None:
        output.parent.mkdir(parents=True, exist_ok=True)
        # Never overwrite a prior intake manifest, even in racing sessions.
        with output.open("x", encoding="utf-8") as stream:
            json.dump(report, stream, indent=2)
            stream.write("\n")
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--course-dir", type=Path, required=True)
    parser.add_argument("--approved-zip", type=Path, required=True)
    parser.add_argument("--menu-path", type=Path)
    parser.add_argument("--work-freeze-manifest", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    try:
        report = run(args.course_dir, args.approved_zip, args.menu_path,
                     args.work_freeze_manifest, args.output)
    except (IntakeError, OSError, ValueError, KeyError, zipfile.BadZipFile) as exc:
        print(f"PIXTEE INTAKE BLOCKED: {exc}", file=sys.stderr)
        return 1
    print(json.dumps({
        "status": report["result"],
        "hole_1_files_hashed": report["source_file_count"],
        "independent_work_hashes_verified": report["work_independent_hash_manifest_verified"],
        "approved_zip_sha256": report["approved_art"]["sha256"],
        "menu_status": report["menu_status"],
        "three_holes_playable": False,
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
