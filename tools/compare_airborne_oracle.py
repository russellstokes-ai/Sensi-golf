#!/usr/bin/env python3
"""Exact comparator for original-machine-code vs portable-core airborne probes."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

FIELDS = (
    "x", "y", "height", "vertical_force",
    "horizontal_force", "direction",
)


def compare(original: dict, recovered: dict) -> dict:
    a = original.get("samples", [])
    b = recovered.get("samples", [])
    mismatches = []
    if len(a) != len(b):
        mismatches.append({
            "kind": "sample_count",
            "original": len(a),
            "recovered": len(b),
        })

    for left, right in zip(a, b):
        if left.get("tick") != right.get("tick"):
            mismatches.append({
                "kind": "tick",
                "original": left.get("tick"),
                "recovered": right.get("tick"),
            })
            continue
        for field in FIELDS:
            if int(left[field]) != int(right[field]):
                mismatches.append({
                    "kind": "value",
                    "tick": left["tick"],
                    "field": field,
                    "original": left[field],
                    "recovered": right[field],
                })

    return {
        "pass": not mismatches,
        "sample_count": min(len(a), len(b)),
        "fields": list(FIELDS),
        "tolerance": 0,
        "mismatch_count": len(mismatches),
        "mismatches": mismatches[:100],
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("original", type=Path)
    ap.add_argument("recovered", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    report = compare(
        json.loads(args.original.read_text(encoding="utf-8")),
        json.loads(args.recovered.read_text(encoding="utf-8")),
    )
    out = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(out, encoding="utf-8")
    else:
        print(out, end="")
    return 0 if report["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
