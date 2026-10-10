#!/usr/bin/env python3
"""Cross-check probe output from the DOS and Windows Sensible Golf executables."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def normalized_strings(probe: dict) -> dict[str, list[int]]:
    result: dict[str, list[int]] = {}
    for row in probe.get("strings", []):
        text = str(row.get("text", "")).strip()
        if not text:
            continue
        result.setdefault(text, []).append(int(row["offset"]))
    return result


def integer_index(probe: dict) -> dict[tuple[int, int], list[int]]:
    result: dict[tuple[int, int], list[int]] = {}
    for row in probe.get("integer_hits", []):
        key = (int(row["value"]), int(row["width"]))
        result.setdefault(key, []).append(int(row["offset"]))
    return result


def compare(a: dict, b: dict) -> dict:
    strings_a = normalized_strings(a)
    strings_b = normalized_strings(b)

    common_strings = []
    for text in sorted(set(strings_a) & set(strings_b), key=lambda s: (s.lower(), s)):
        common_strings.append({
            "text": text,
            "a_offsets": strings_a[text],
            "b_offsets": strings_b[text],
        })

    ints_a = integer_index(a)
    ints_b = integer_index(b)
    common_integers = []
    for key in sorted(set(ints_a) & set(ints_b)):
        value, width = key
        common_integers.append({
            "value": value,
            "width": width,
            "a_offsets": ints_a[key],
            "b_offsets": ints_b[key],
        })

    return {
        "a": {
            "path": a.get("path"),
            "sha256": a.get("sha256"),
            "format": a.get("format"),
        },
        "b": {
            "path": b.get("path"),
            "sha256": b.get("sha256"),
            "format": b.get("format"),
        },
        "common_strings": common_strings,
        "common_integer_hits": common_integers,
        "a_only_strings": sorted(set(strings_a) - set(strings_b)),
        "b_only_strings": sorted(set(strings_b) - set(strings_a)),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Cross-check two executable probe reports")
    parser.add_argument("a", type=Path)
    parser.add_argument("b", type=Path)
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    report = compare(load(args.a), load(args.b))
    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
