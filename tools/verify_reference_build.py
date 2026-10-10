#!/usr/bin/env python3
"""Verify a local Sensible Golf PC install against a reference manifest.

This never uploads or modifies game files. It compares relative path/basename,
byte size and CRC-32, and reports an explicit status for each expected file.
"""

from __future__ import annotations

import argparse
import json
import zlib
from pathlib import Path


def crc32_file(path: Path, chunk_size: int = 1024 * 1024) -> str:
    crc = 0
    with path.open("rb") as handle:
        while chunk := handle.read(chunk_size):
            crc = zlib.crc32(chunk, crc)
    return f"{crc & 0xffffffff:08x}"


def index_files(root: Path) -> tuple[dict[str, Path], dict[str, list[Path]]]:
    by_relative: dict[str, Path] = {}
    by_name: dict[str, list[Path]] = {}
    for path in sorted(p for p in root.rglob("*") if p.is_file()):
        rel = path.relative_to(root).as_posix().lower()
        by_relative[rel] = path
        by_name.setdefault(path.name.lower(), []).append(path)
    return by_relative, by_name


def locate(expected_path: str, by_relative: dict[str, Path], by_name: dict[str, list[Path]]) -> Path | None:
    key = expected_path.replace("\\", "/").lower()
    if key in by_relative:
        return by_relative[key]
    candidates = by_name.get(Path(key).name.lower(), [])
    return candidates[0] if len(candidates) == 1 else None


def verify(root: Path, manifest: dict) -> list[dict[str, object]]:
    by_relative, by_name = index_files(root)
    results: list[dict[str, object]] = []
    for expected in manifest["files"]:
        path = locate(expected["path"], by_relative, by_name)
        row = {"expected_path": expected["path"], "status": "missing"}
        if path is None:
            results.append(row)
            continue

        size = path.stat().st_size
        crc = crc32_file(path)
        size_ok = size == expected["size"]
        crc_ok = crc.lower() == expected["crc32"].lower()
        row.update({
            "actual_path": path.relative_to(root).as_posix(),
            "size": size,
            "crc32": crc,
            "size_ok": size_ok,
            "crc32_ok": crc_ok,
            "status": "match" if size_ok and crc_ok else "mismatch",
        })
        results.append(row)
    return results


def main() -> int:
    parser = argparse.ArgumentParser(description="Verify local PC game files against a reference build")
    parser.add_argument("root", type=Path)
    parser.add_argument("--manifest", type=Path, default=Path("reference/pc_build_5865.0.json"))
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    results = verify(args.root, manifest)

    if args.json:
        print(json.dumps(results, indent=2))
    else:
        for row in results:
            print(f'{row["status"]:8} {row["expected_path"]}')
        matches = sum(r["status"] == "match" for r in results)
        mismatches = sum(r["status"] == "mismatch" for r in results)
        missing = sum(r["status"] == "missing" for r in results)
        print(f"\nmatch={matches} mismatch={mismatches} missing={missing}")

    return 0 if all(r["status"] == "match" for r in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
