#!/usr/bin/env python3
"""Create a reproducible manifest of local original-game inputs."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


def sha256_file(path: Path, chunk_size: int = 1024 * 1024) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        while chunk := handle.read(chunk_size):
            digest.update(chunk)
    return digest.hexdigest()


def manifest(root: Path) -> list[dict[str, object]]:
    if not root.exists():
        raise FileNotFoundError(root)
    files = sorted(p for p in root.rglob("*") if p.is_file())
    return [
        {
            "path": p.relative_to(root).as_posix(),
            "size": p.stat().st_size,
            "sha256": sha256_file(p),
        }
        for p in files
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description="Hash original game inputs")
    parser.add_argument("root", type=Path)
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    try:
        rows = manifest(args.root)
    except OSError as exc:
        parser.error(str(exc))

    text = json.dumps(rows, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
