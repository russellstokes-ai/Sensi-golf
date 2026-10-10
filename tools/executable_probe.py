#!/usr/bin/env python3
"""Metadata-first probe for Sensible Golf PC executables.

This does not decompile or alter the executable. It records reproducible
format/header facts, printable strings and locations of requested integer
constants so later reverse-engineering work can be anchored to evidence.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zlib
from pathlib import Path


MACHINE_NAMES = {
    0x014C: "i386",
    0x8664: "x86-64",
}


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def crc32_bytes(data: bytes) -> str:
    return f"{zlib.crc32(data) & 0xffffffff:08x}"


def extract_ascii_strings(data: bytes, min_length: int = 4) -> list[dict[str, object]]:
    results: list[dict[str, object]] = []
    start = None
    for i, byte in enumerate(data + b"\0"):
        printable = 32 <= byte <= 126
        if printable and start is None:
            start = i
        elif not printable and start is not None:
            if i - start >= min_length:
                results.append({
                    "offset": start,
                    "text": data[start:i].decode("ascii"),
                })
            start = None
    return results


def find_integer(data: bytes, value: int) -> list[dict[str, object]]:
    hits: list[dict[str, object]] = []
    for width, fmt in ((2, "<H"), (4, "<I")):
        if not 0 <= value < (1 << (width * 8)):
            continue
        needle = struct.pack(fmt, value)
        pos = 0
        while True:
            found = data.find(needle, pos)
            if found < 0:
                break
            hits.append({"offset": found, "width": width, "value": value})
            pos = found + 1
    return hits


def parse_pe(data: bytes, offset: int) -> dict[str, object]:
    if offset + 24 > len(data):
        return {"error": "truncated PE/COFF header"}
    machine, number_sections, timestamp, _, _, optional_size, characteristics = struct.unpack_from(
        "<HHIIIHH", data, offset + 4
    )
    optional_offset = offset + 24
    optional_magic = None
    image_base = None
    entry_point_rva = None
    if optional_size >= 2 and optional_offset + optional_size <= len(data):
        optional_magic = struct.unpack_from("<H", data, optional_offset)[0]
        if optional_size >= 32:
            entry_point_rva = struct.unpack_from("<I", data, optional_offset + 16)[0]
            if optional_magic == 0x10B and optional_size >= 32:
                image_base = struct.unpack_from("<I", data, optional_offset + 28)[0]
            elif optional_magic == 0x20B and optional_size >= 32:
                image_base = struct.unpack_from("<Q", data, optional_offset + 24)[0]

    sections = []
    section_offset = optional_offset + optional_size
    for index in range(number_sections):
        pos = section_offset + index * 40
        if pos + 40 > len(data):
            break
        raw_name = data[pos : pos + 8].split(b"\0", 1)[0]
        try:
            name = raw_name.decode("ascii")
        except UnicodeDecodeError:
            name = raw_name.hex()
        virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from(
            "<IIII", data, pos + 8
        )
        sections.append({
            "name": name,
            "virtual_size": virtual_size,
            "virtual_address": virtual_address,
            "raw_size": raw_size,
            "raw_offset": raw_offset,
        })

    return {
        "format": "PE",
        "machine": MACHINE_NAMES.get(machine, f"0x{machine:04x}"),
        "number_sections": number_sections,
        "timestamp": timestamp,
        "optional_magic": None if optional_magic is None else f"0x{optional_magic:04x}",
        "entry_point_rva": entry_point_rva,
        "image_base": image_base,
        "characteristics": f"0x{characteristics:04x}",
        "sections": sections,
    }


def parse_executable(data: bytes) -> dict[str, object]:
    result: dict[str, object] = {
        "size": len(data),
        "sha256": sha256_bytes(data),
        "crc32": crc32_bytes(data),
    }
    if len(data) < 2 or data[:2] != b"MZ":
        result["format"] = "unknown/non-MZ"
        return result

    result["dos_signature"] = "MZ"
    if len(data) < 0x40:
        result["format"] = "MZ/DOS"
        return result

    new_header_offset = struct.unpack_from("<I", data, 0x3C)[0]
    result["new_header_offset"] = new_header_offset
    if new_header_offset >= len(data):
        result["format"] = "MZ/DOS"
        return result

    signature4 = data[new_header_offset : new_header_offset + 4]
    signature2 = signature4[:2]
    if signature4 == b"PE\0\0":
        result.update(parse_pe(data, new_header_offset))
    elif signature2 == b"LE":
        result["format"] = "LE"
        result["signature"] = "LE"
        # Keep LE metadata conservative until validated against the real binary.
        result["header_available_bytes"] = min(256, len(data) - new_header_offset)
    elif signature2 == b"LX":
        result["format"] = "LX"
        result["signature"] = "LX"
    elif signature2 == b"NE":
        result["format"] = "NE"
        result["signature"] = "NE"
    else:
        result["format"] = "MZ/DOS"
    return result


def probe(path: Path, min_string: int = 5, numbers: list[int] | None = None) -> dict[str, object]:
    data = path.read_bytes()
    result = parse_executable(data)
    result["path"] = path.name
    strings = extract_ascii_strings(data, min_string)
    result["strings"] = strings
    result["string_count"] = len(strings)
    result["integer_hits"] = []
    for number in numbers or []:
        result["integer_hits"].extend(find_integer(data, number))
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description="Probe a Sensible Golf PC executable")
    parser.add_argument("executable", type=Path)
    parser.add_argument("--min-string", type=int, default=5)
    parser.add_argument("--number", type=int, action="append", default=[])
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    report = probe(args.executable, args.min_string, args.number)
    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
