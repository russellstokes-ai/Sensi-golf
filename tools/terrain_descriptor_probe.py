#!/usr/bin/env python3
"""Inspect the original v1.014 terrain descriptor pointer table.

This records structural data only. It deliberately does not assign semantic
surface names until code/runtime evidence supports them.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

from pe_target_xrefs import parse_pe

TABLE_VA = 0x41DFF7


def va_to_offset(va: int, image_base: int, sections: list[dict]) -> int:
    rva = va - image_base
    for section in sections:
        span = max(section["virtual_size"], section["raw_size"])
        if section["rva"] <= rva < section["rva"] + span:
            offset = section["raw_offset"] + (rva - section["rva"])
            if offset < section["raw_offset"] + section["raw_size"]:
                return offset
    raise ValueError(f"unbacked VA {va:#x}")


def read_bytes(data: bytes, va: int, size: int, image_base: int, sections: list[dict]) -> bytes:
    off = va_to_offset(va, image_base, sections)
    blob = data[off : off + size]
    if len(blob) != size:
        raise ValueError(f"truncated read at {va:#x}")
    return blob


def probe(exe: Path, count: int = 128) -> dict:
    data = exe.read_bytes()
    image_base, _, sections = parse_pe(data)

    rows = []
    for index in range(count):
        try:
            raw_ptr = read_bytes(data, TABLE_VA + index * 4, 4, image_base, sections)
        except ValueError:
            break
        pointer = struct.unpack("<I", raw_ptr)[0]

        row = {"index": index, "pointer": pointer, "mapped": False}
        try:
            payload = read_bytes(data, pointer, 24, image_base, sections)
        except ValueError:
            rows.append(row)
            continue

        row["mapped"] = True
        row["words_u16"] = list(struct.unpack("<12H", payload))
        row["dwords_u32"] = list(struct.unpack("<6I", payload))
        rows.append(row)

    by_first_word: dict[str, list[int]] = {}
    for row in rows:
        if row.get("mapped"):
            key = str(row["words_u16"][0])
            by_first_word.setdefault(key, []).append(row["index"])

    special = {}
    for code in (0, 8, 0x23, 0x32):
        special[str(code)] = [
            row for row in rows
            if row.get("mapped") and row["words_u16"][0] == code
        ]

    return {
        "table_va": TABLE_VA,
        "requested_count": count,
        "rows": rows,
        "indices_by_first_word": by_first_word,
        "special_first_word_groups": special,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("--count", type=int, default=128)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    report = probe(args.exe, args.count)
    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
