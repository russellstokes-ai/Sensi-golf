#!/usr/bin/env python3
"""Extract v1.014 next-hole resource templates and course order tables."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from original_v1014_oracle import build_uc, ru32

TEMPLATE_VAS = (0x41EF00, 0x41EF0B, 0x41EF16)
ORDER_POINTER_TABLE_VA = 0x41E64C
PAR_TABLE_VA = 0x41E603
ORDER_SLOTS = 8
HOLES_PER_ROUND = 18


def read_c_string(uc, address: int, limit: int = 64) -> str:
    raw = bytes(uc.mem_read(address, limit))
    return raw.split(b"\0", 1)[0].decode("ascii", errors="replace")


def mapped_order(uc, pointer: int) -> list[int] | None:
    try:
        raw = bytes(uc.mem_read(pointer, HOLES_PER_ROUND))
    except Exception:
        return None
    # Original hole identifiers are decimal resource IDs. Keep raw values;
    # validation is deliberately conservative.
    if any(v > 99 for v in raw):
        return None
    return list(raw)


def extract(exe: Path) -> dict:
    uc, digest = build_uc(exe)

    templates = [read_c_string(uc, va) for va in TEMPLATE_VAS]

    orders = []
    for slot in range(ORDER_SLOTS):
        ptr = ru32(uc, ORDER_POINTER_TABLE_VA + slot * 4)
        order = mapped_order(uc, ptr)
        orders.append({
            "slot": slot,
            "pointer": ptr,
            "hole_ids": order,
        })

    par_table = list(bytes(uc.mem_read(PAR_TABLE_VA, 100)))

    return {
        "reference": "Sensible Golf Windows v1.014",
        "build_sha256": digest,
        "resource_templates": [
            {"va": va, "template": template}
            for va, template in zip(TEMPLATE_VAS, templates)
        ],
        "order_pointer_table_va": ORDER_POINTER_TABLE_VA,
        "orders": orders,
        "par_table_va": PAR_TABLE_VA,
        "par_by_resource_id": par_table,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    result = extract(args.exe)
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
