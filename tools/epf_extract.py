#!/usr/bin/env python3
"""Extract East Point EPF archives used by Sensible Golf.

Implements the documented EPFS container and its big-endian 9..14-bit LZW
variant. Original game files are inputs only and are never modified.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from epf_inspect import EPFError, parse_epf


class LZWError(EPFError):
    pass


class BitReader:
    def __init__(self, data: bytes):
        self.data = data
        self.bit = 0

    def read(self, width: int) -> int | None:
        if self.bit + width > len(self.data) * 8:
            return None
        value = 0
        for _ in range(width):
            byte_index = self.bit // 8
            bit_index = 7 - (self.bit % 8)
            value = (value << 1) | ((self.data[byte_index] >> bit_index) & 1)
            self.bit += 1
        return value


def decompress_epfs_lzw(data: bytes, expected_size: int | None = None) -> bytes:
    reader = BitReader(data)
    width = 9
    max_width = 14
    dictionary: dict[int, bytes] = {i: bytes([i]) for i in range(256)}
    next_code = 256
    out = bytearray()
    old_code: int | None = None

    while True:
        code = reader.read(width)
        if code is None:
            raise LZWError("compressed stream ended before EPFS EOF code")

        eof_code = (1 << width) - 1
        reset_code = eof_code - 1
        max_normal_code = eof_code - 2

        if code == eof_code:
            break

        if code == reset_code:
            dictionary = {i: bytes([i]) for i in range(256)}
            next_code = 256
            old_code = None
            # EPFS keeps the current bit width across dictionary resets.
            continue

        if old_code is None:
            if code not in dictionary:
                raise LZWError(f"invalid first LZW code {code}")
            entry = dictionary[code]
            out.extend(entry)
            old_code = code
            continue

        if code in dictionary:
            entry = dictionary[code]
        elif code == next_code:
            prev = dictionary.get(old_code)
            if prev is None:
                raise LZWError(f"invalid previous LZW code {old_code}")
            entry = prev + prev[:1]
        else:
            raise LZWError(f"LZW code {code} exceeds next dictionary code {next_code}")

        out.extend(entry)

        prev = dictionary.get(old_code)
        if prev is None:
            raise LZWError(f"invalid previous LZW code {old_code}")

        if next_code < (1 << max_width):
            dictionary[next_code] = prev + entry[:1]
            next_code += 1

        if next_code > max_normal_code and width < max_width:
            width += 1

        old_code = code

        if expected_size is not None and len(out) > expected_size:
            raise LZWError(
                f"decompressed stream exceeded expected size {expected_size}"
            )

    result = bytes(out)
    if expected_size is not None and len(result) != expected_size:
        raise LZWError(
            f"decompressed size mismatch: expected {expected_size}, got {len(result)}"
        )
    return result


def extract_epf(archive: Path, output: Path) -> list[Path]:
    raw = archive.read_bytes()
    entries = parse_epf(raw)
    output.mkdir(parents=True, exist_ok=True)
    written: list[Path] = []

    for entry in entries:
        payload = raw[entry.offset : entry.offset + entry.compressed_size]
        if len(payload) != entry.compressed_size:
            raise EPFError(f"{entry.filename}: truncated payload")

        if entry.compressed:
            content = decompress_epfs_lzw(payload, entry.decompressed_size)
        else:
            content = payload
            if len(content) != entry.decompressed_size:
                raise EPFError(
                    f"{entry.filename}: raw size mismatch "
                    f"({len(content)} != {entry.decompressed_size})"
                )

        safe_name = Path(entry.filename).name
        if not safe_name or safe_name in {".", ".."}:
            raise EPFError(f"unsafe filename {entry.filename!r}")

        destination = output / safe_name
        destination.write_bytes(content)
        written.append(destination)

    return written


def main() -> int:
    parser = argparse.ArgumentParser(description="Extract an East Point EPF archive")
    parser.add_argument("archive", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    try:
        written = extract_epf(args.archive, args.output)
    except (OSError, EPFError) as exc:
        parser.error(str(exc))

    print(f"extracted {len(written)} files to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
