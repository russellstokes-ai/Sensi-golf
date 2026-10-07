#!/usr/bin/env python3
"""Select a real MAPI cell whose terrain descriptor has a requested landing code."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from terrain_descriptor_probe import probe as probe_descriptors


def lookup(desc: bytes, sel: bytes, tile: int, x: int, y: int) -> dict:
    base = tile * 8
    mask = 0x80 >> x
    selected = 0
    if sel[base + y] & mask:
        selected += 2
    if sel[base + y + 4] & mask:
        selected += 4
    raw = (desc[base + selected] << 8) | desc[base + selected + 1]
    descriptor = raw & 0xFF
    if descriptor >= 0x4D:
        descriptor = 4
    meta = raw >> 8
    return {
        "descriptor_index": descriptor,
        "slope_direction": (meta & 0x0F) << 8,
        "slope_magnitude": meta >> 4,
        "raw_word": raw,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("descriptor_bank", type=Path)
    ap.add_argument("selector_bank", type=Path)
    ap.add_argument("--landing-code", type=int, required=True)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    desc = args.descriptor_bank.read_bytes()
    sel = args.selector_bank.read_bytes()
    if len(desc) != len(sel) or len(desc) % 8:
        raise SystemExit("MAPI banks must have equal eight-byte-aligned sizes")

    table = probe_descriptors(args.exe, 128)
    landing_by_index = {
        int(row["index"]): int(row["landing_code"])
        for row in table["rows"]
        if row.get("mapped") and "landing_code" in row
    }

    found = None
    for tile in range(len(desc) // 8):
        for y in range(4):
            for x in range(8):
                row = lookup(desc, sel, tile, x, y)
                code = landing_by_index.get(row["descriptor_index"])
                if code == args.landing_code:
                    found = {
                        "descriptor_bank": args.descriptor_bank.name,
                        "selector_bank": args.selector_bank.name,
                        "tile": tile,
                        "x_subcell": x,
                        "y_subcell": y,
                        "landing_code": code,
                        **row,
                    }
                    break
            if found:
                break
        if found:
            break

    if not found:
        return 3

    text = json.dumps(found, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
