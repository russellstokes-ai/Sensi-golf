#!/usr/bin/env python3
"""Read an East Point Software EPF archive table without modifying the archive.

The EPF structure is documented at ModdingWiki. This tool deliberately only
inventories entries; decompression/extraction is a separate step so recovery
work remains easy to validate.
"""
from __future__ import annotations

import argparse
import json
import struct
from dataclasses import asdict, dataclass
from pathlib import Path

HEADER_SIZE = 11
ENTRY_SIZE = 22  # 13 filename + 1 flag + 4 compressed + 4 decompressed
SIGNATURE = b"EPFS"


class EPFError(ValueError):
    pass


@dataclass(frozen=True)
class EPFEntry:
    filename: str
    compressed: bool
    offset: int
    compressed_size: int
    decompressed_size: int


def parse_epf(data: bytes) -> list[EPFEntry]:
    if len(data) < HEADER_SIZE:
        raise EPFError("file is too small to contain an EPF header")
    if data[:4] != SIGNATURE:
        raise EPFError("invalid EPF signature")

    fat_offset = struct.unpack_from("<I", data, 4)[0]
    num_files = struct.unpack_from("<H", data, 9)[0]

    if fat_offset < HEADER_SIZE or fat_offset > len(data):
        raise EPFError("FAT offset is outside the file")
    fat_end = fat_offset + num_files * ENTRY_SIZE
    if fat_end > len(data):
        raise EPFError("FAT is truncated")

    entries: list[EPFEntry] = []
    data_offset = HEADER_SIZE
    for i in range(num_files):
        pos = fat_offset + i * ENTRY_SIZE
        raw_name = data[pos : pos + 13]
        name_bytes = raw_name.split(b"\0", 1)[0]
        try:
            filename = name_bytes.decode("ascii")
        except UnicodeDecodeError as exc:
            raise EPFError(f"entry {i} has a non-ASCII filename") from exc

        compression_flag = data[pos + 13]
        if compression_flag not in (0, 1):
            raise EPFError(f"entry {i} has invalid compression flag {compression_flag}")
        compressed_size, decompressed_size = struct.unpack_from("<II", data, pos + 14)

        next_offset = data_offset + compressed_size
        if next_offset > fat_offset:
            raise EPFError(f"entry {i} data overlaps the FAT")

        entries.append(
            EPFEntry(
                filename=filename,
                compressed=bool(compression_flag),
                offset=data_offset,
                compressed_size=compressed_size,
                decompressed_size=decompressed_size,
            )
        )
        data_offset = next_offset

    return entries


def main() -> int:
    parser = argparse.ArgumentParser(description="Inventory an East Point Software EPF archive")
    parser.add_argument("archive", type=Path)
    parser.add_argument("--json", action="store_true", help="emit JSON")
    args = parser.parse_args()

    try:
        entries = parse_epf(args.archive.read_bytes())
    except (OSError, EPFError) as exc:
        parser.error(str(exc))

    if args.json:
        print(json.dumps([asdict(e) for e in entries], indent=2))
    else:
        print(f"entries: {len(entries)}")
        for e in entries:
            mode = "LZW" if e.compressed else "raw"
            print(
                f"{e.offset:10d}  {e.compressed_size:10d} -> "
                f"{e.decompressed_size:10d}  {mode:3s}  {e.filename}"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
