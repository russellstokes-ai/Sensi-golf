#!/usr/bin/env python3
"""Inspect named terrain descriptors from Sensible Golf Windows v1.014."""

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


def read_cstring(data: bytes, va: int, image_base: int, sections: list[dict], limit: int = 48) -> str:
    off = va_to_offset(va, image_base, sections)
    end = min(len(data), off + limit)
    raw = data[off:end].split(b"\0", 1)[0]
    if not raw:
        return ""
    if any(byte < 32 or byte > 126 for byte in raw):
        return ""
    return raw.decode("ascii")


def signed_u16(value: int) -> int:
    return value - 0x10000 if value & 0x8000 else value


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
            header = read_bytes(data, pointer, 6, image_base, sections)
        except ValueError:
            rows.append(row)
            continue

        landing_code, variant_raw, profile_slot = struct.unpack("<3H", header)
        row.update({
            "mapped": True,
            "landing_code": landing_code,
            "variant_raw": variant_raw,
            "variant_signed": signed_u16(variant_raw),
            "profile_slot": profile_slot,
            "name": read_cstring(data, pointer + 6, image_base, sections),
        })
        rows.append(row)

    by_landing_code: dict[str, list[int]] = {}
    by_profile_slot: dict[str, list[int]] = {}
    for row in rows:
        if not row.get("mapped"):
            continue
        by_landing_code.setdefault(str(row["landing_code"]), []).append(row["index"])
        by_profile_slot.setdefault(str(row["profile_slot"]), []).append(row["index"])

    return {
        "table_va": TABLE_VA,
        "descriptor_header": {
            "size": 6,
            "word0": "landing_code (read by landing branch)",
            "word1": "variant field (written to ball +0x22; semantics under recovery)",
            "word2": "profile_slot (written to ball +0x20 and used by swing profile selector)",
            "name": "NUL-terminated original debug/name string at +0x06",
        },
        "requested_count": count,
        "rows": rows,
        "indices_by_landing_code": by_landing_code,
        "indices_by_profile_slot": by_profile_slot,
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
