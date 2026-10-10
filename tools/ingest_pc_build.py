#!/usr/bin/env python3
"""Run the complete private Step-2 ingestion pipeline for a Sensible Golf PC build.

Input may be a ZIP archive or an already-extracted directory. Original/licensed
files are copied only into the chosen local workspace. Reports contain metadata,
hashes and classifications, not copyrighted payloads.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import struct
import tempfile
import zipfile
import zlib
from pathlib import Path

from epf_extract import extract_epf
from epf_inspect import parse_epf


CRITICAL = ("GOLFDOS.EXE", "GOLF.EPF")


def sha256_file(path: Path, chunk_size: int = 1024 * 1024) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        while chunk := handle.read(chunk_size):
            digest.update(chunk)
    return digest.hexdigest()


def crc32_file(path: Path, chunk_size: int = 1024 * 1024) -> str:
    crc = 0
    with path.open("rb") as handle:
        while chunk := handle.read(chunk_size):
            crc = zlib.crc32(chunk, crc)
    return f"{crc & 0xffffffff:08x}"


def safe_extract_zip(archive: Path, destination: Path) -> None:
    destination.mkdir(parents=True, exist_ok=True)
    root = destination.resolve()
    with zipfile.ZipFile(archive) as zf:
        for info in zf.infolist():
            candidate = (destination / info.filename).resolve()
            try:
                candidate.relative_to(root)
            except ValueError as exc:
                raise ValueError(f"unsafe ZIP path: {info.filename}") from exc
        zf.extractall(destination)


def iter_files(root: Path):
    return sorted((p for p in root.rglob("*") if p.is_file()), key=lambda p: p.as_posix().lower())


def file_manifest(root: Path) -> list[dict[str, object]]:
    return [
        {
            "path": path.relative_to(root).as_posix(),
            "size": path.stat().st_size,
            "crc32": crc32_file(path),
            "sha256": sha256_file(path),
        }
        for path in iter_files(root)
    ]


def index_by_name(root: Path) -> dict[str, list[Path]]:
    result: dict[str, list[Path]] = {}
    for path in iter_files(root):
        result.setdefault(path.name.upper(), []).append(path)
    return result


def find_unique(index: dict[str, list[Path]], name: str) -> Path | None:
    matches = index.get(name.upper(), [])
    return matches[0] if len(matches) == 1 else None


def identify_executable(path: Path) -> str:
    data = path.read_bytes()[:4096]
    if len(data) < 2 or data[:2] != b"MZ":
        return "not-mz"
    if len(data) < 64:
        return "MZ"
    peoff = struct.unpack_from("<I", data, 0x3C)[0]
    if peoff + 4 <= len(data):
        sig = data[peoff : peoff + 4]
        if sig == b"PE\0\0":
            return "PE32/PE"
        if sig[:2] == b"LE":
            return "LE (DOS extender / linear executable)"
        if sig[:2] == b"NE":
            return "NE"
    return "MZ/DOS"


def classify_file(path: Path) -> str:
    head = path.read_bytes()[:32]
    ext = path.suffix.lower()
    if head.startswith(b"EPFS"):
        return "East Point EPF archive"
    if head.startswith(b"MZ"):
        return identify_executable(path)
    if head.startswith(b"HMIMIDIP"):
        return "HMI/HMP music"
    if head.startswith(b"MThd"):
        return "Standard MIDI"
    if head.startswith(b"BM"):
        return "Windows bitmap"
    if head.startswith(b"RIFF") and head[8:12] == b"WAVE":
        return "WAV audio"
    if head.startswith(b"RIFF"):
        return "RIFF container"
    if ext in {".pal", ".wnd"}:
        return "palette/graphics support data"
    if ext in {".cfg", ".ini"}:
        return "configuration"
    if ext in {".map", ".lev", ".lvl", ".crs"}:
        return "likely level/course data"
    if ext in {".pcx", ".bmp", ".gif"}:
        return "graphics"
    if ext in {".voc", ".wav", ".raw", ".snd"}:
        return "audio/sample"
    if ext in {".mid", ".hmp"}:
        return "music"
    return "unknown/binary data"


def compare_reference(actual: list[dict[str, object]], reference: dict) -> list[dict[str, object]]:
    by_rel = {str(row["path"]).lower(): row for row in actual}
    by_name: dict[str, list[dict[str, object]]] = {}
    for row in actual:
        by_name.setdefault(Path(str(row["path"])).name.lower(), []).append(row)

    checks = []
    for expected in reference.get("files", []):
        key = str(expected["path"]).replace("\\", "/").lower()
        row = by_rel.get(key)
        if row is None:
            candidates = by_name.get(Path(key).name.lower(), [])
            row = candidates[0] if len(candidates) == 1 else None
        if row is None:
            checks.append({"path": expected["path"], "status": "missing"})
            continue
        size_ok = row["size"] == expected["size"]
        crc_ok = str(row["crc32"]).lower() == str(expected["crc32"]).lower()
        checks.append({
            "path": expected["path"],
            "actual_path": row["path"],
            "status": "match" if size_ok and crc_ok else "mismatch",
            "size_ok": size_ok,
            "crc32_ok": crc_ok,
            "actual_size": row["size"],
            "actual_crc32": row["crc32"],
        })
    return checks


def write_markdown_report(report: dict, destination: Path) -> None:
    critical = report["critical_files"]
    ref = report["reference_summary"]
    epf = report["epf"]
    lines = [
        "# Sensible Golf Step 2 local ingestion report",
        "",
        f"- Input: `{report['input_name']}`",
        f"- Input type: {report['input_type']}",
        f"- Files inventoried: {report['file_count']}",
        f"- Known-reference matches: {ref['matches']}",
        f"- Known-reference mismatches: {ref['mismatches']}",
        f"- Known-reference missing: {ref['missing']}",
        "",
        "## Critical files",
        "",
    ]
    for name, info in critical.items():
        if info is None:
            lines.append(f"- **{name}: MISSING**")
        else:
            lines.append(
                f"- **{name}** — {info['size']} bytes, CRC-32 `{info['crc32']}`, "
                f"SHA-256 `{info['sha256']}`, type: {info['type']}"
            )
    lines += [
        "",
        "## GOLF.EPF",
        "",
        f"- Entries: {epf.get('entry_count', 0)}",
        f"- Compressed entries: {epf.get('compressed_count', 0)}",
        f"- Extracted entries: {epf.get('extracted_count', 0)}",
        "",
        "This report contains metadata only. Original game payloads remain in the ignored private workspace.",
        "",
    ]
    destination.write_text("\n".join(lines), encoding="utf-8")


def ingest(source: Path, workspace: Path, reference_path: Path) -> dict:
    workspace.mkdir(parents=True, exist_ok=True)
    input_root = workspace / "input"
    extracted_root = workspace / "extracted" / "epf"
    reports_root = workspace / "reports"
    reports_root.mkdir(parents=True, exist_ok=True)

    if input_root.exists():
        shutil.rmtree(input_root)

    if source.is_file():
        if not zipfile.is_zipfile(source):
            raise ValueError("input file must be a ZIP archive")
        safe_extract_zip(source, input_root)
        input_type = "zip"
        archive_sha256 = sha256_file(source)
    elif source.is_dir():
        shutil.copytree(source, input_root)
        input_type = "directory"
        archive_sha256 = None
    else:
        raise FileNotFoundError(source)

    manifest = file_manifest(input_root)
    (reports_root / "input_manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )

    reference = json.loads(reference_path.read_text(encoding="utf-8"))
    checks = compare_reference(manifest, reference)
    (reports_root / "reference_comparison.json").write_text(
        json.dumps(checks, indent=2) + "\n", encoding="utf-8"
    )

    by_name = index_by_name(input_root)
    critical_files: dict[str, dict[str, object] | None] = {}
    for name in (*CRITICAL, "GOLFWIN.EXE"):
        path = find_unique(by_name, name)
        if path is None:
            critical_files[name] = None
            continue
        critical_files[name] = {
            "path": path.relative_to(input_root).as_posix(),
            "size": path.stat().st_size,
            "crc32": crc32_file(path),
            "sha256": sha256_file(path),
            "type": classify_file(path),
        }

    missing_critical = [name for name in CRITICAL if critical_files[name] is None]
    if missing_critical:
        raise ValueError("missing critical file(s): " + ", ".join(missing_critical))

    epf_path = find_unique(by_name, "GOLF.EPF")
    assert epf_path is not None
    epf_data = epf_path.read_bytes()
    entries = parse_epf(epf_data)
    epf_inventory = [
        {
            "filename": entry.filename,
            "compressed": entry.compressed,
            "offset": entry.offset,
            "compressed_size": entry.compressed_size,
            "decompressed_size": entry.decompressed_size,
        }
        for entry in entries
    ]
    (reports_root / "epf_inventory.json").write_text(
        json.dumps(epf_inventory, indent=2) + "\n", encoding="utf-8"
    )

    if extracted_root.exists():
        shutil.rmtree(extracted_root)
    written = extract_epf(epf_path, extracted_root)
    extracted_manifest = [
        {
            "filename": path.name,
            "size": path.stat().st_size,
            "sha256": sha256_file(path),
            "type": classify_file(path),
        }
        for path in sorted(written, key=lambda p: p.name.lower())
    ]
    (reports_root / "epf_extracted_manifest.json").write_text(
        json.dumps(extracted_manifest, indent=2) + "\n", encoding="utf-8"
    )

    summary = {
        "input_name": source.name,
        "input_type": input_type,
        "archive_sha256": archive_sha256,
        "file_count": len(manifest),
        "critical_files": critical_files,
        "reference_summary": {
            "matches": sum(row["status"] == "match" for row in checks),
            "mismatches": sum(row["status"] == "mismatch" for row in checks),
            "missing": sum(row["status"] == "missing" for row in checks),
        },
        "epf": {
            "entry_count": len(entries),
            "compressed_count": sum(entry.compressed for entry in entries),
            "extracted_count": len(written),
        },
        "classification_counts": {},
    }
    for row in extracted_manifest:
        kind = str(row["type"])
        summary["classification_counts"][kind] = summary["classification_counts"].get(kind, 0) + 1

    (reports_root / "step2_report.json").write_text(
        json.dumps(summary, indent=2) + "\n", encoding="utf-8"
    )
    write_markdown_report(summary, reports_root / "step2_report.md")
    return summary


def main() -> int:
    parser = argparse.ArgumentParser(description="Run Sensible Golf Step-2 private ingestion")
    parser.add_argument("source", type=Path, help="legal game ZIP or extracted install directory")
    parser.add_argument(
        "--workspace",
        type=Path,
        default=Path("analysis/private/step2"),
        help="ignored private workspace",
    )
    parser.add_argument(
        "--reference",
        type=Path,
        default=Path("reference/pc_build_5865.0.json"),
    )
    args = parser.parse_args()

    try:
        report = ingest(args.source, args.workspace, args.reference)
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        parser.error(str(exc))

    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
