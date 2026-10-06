#!/usr/bin/env python3
"""Find x86 code references to selected ASCII strings in a PE executable.

Designed for evidence-led Sensible Golf recovery. The tool maps a string's
file offset to its runtime VA using PE sections, disassembles with objdump, and
captures nearby instructions that reference that absolute address.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path

from executable_probe import extract_ascii_strings, parse_executable


DEFAULT_TARGETS = [
    "HDBall X coord",
    "HDBall Y coord",
    "HDBall V force",
    "HDBall H force",
    "HDBall direction",
    "HDPlayer direction",
    "HDBall height",
    "HDBall pause",
    "HWSwing adjuster",
    "HWDrop Power",
    "DWball x position",
    "DWball y position",
    "DDball-hole in yards",
    "DWgreen x1",
    "DWgreen x2",
    "DWgreen y1",
    "DWgreen y2",
    "HDgreen offset",
    "DWWind amount",
    "HDWind adjustment",
    "HDWind x adj",
    "HDWind y adj",
]


def file_offset_to_va(probe: dict, file_offset: int) -> int | None:
    image_base = probe.get("image_base")
    if image_base is None:
        return None
    for section in probe.get("sections", []):
        raw_offset = int(section["raw_offset"])
        raw_size = int(section["raw_size"])
        if raw_offset <= file_offset < raw_offset + raw_size:
            rva = int(section["virtual_address"]) + (file_offset - raw_offset)
            return int(image_base) + rva
    return None


def parse_instruction_address(line: str) -> int | None:
    match = re.match(r"^\s*([0-9a-fA-F]+):", line)
    return int(match.group(1), 16) if match else None


def find_disassembly_references(
    lines: list[str], target_va: int, context: int = 8
) -> list[dict[str, object]]:
    target_hex = f"{target_va:x}".lower()
    pattern = re.compile(
        rf"(?<![0-9a-f])(?:0x)?0*{re.escape(target_hex)}(?![0-9a-f])",
        re.I,
    )
    hits = []
    for index, line in enumerate(lines):
        if not pattern.search(line):
            continue
        start = max(0, index - context)
        end = min(len(lines), index + context + 1)
        hits.append({
            "instruction_address": parse_instruction_address(line),
            "line": line.rstrip(),
            "context": [item.rstrip() for item in lines[start:end]],
        })
    return hits


def analyse(
    executable: Path,
    targets: list[str] | None = None,
    context: int = 8,
    objdump: str = "objdump",
) -> dict[str, object]:
    data = executable.read_bytes()
    probe = parse_executable(data)
    if probe.get("format") != "PE":
        raise ValueError("pe_string_xrefs requires a PE executable")

    strings = extract_ascii_strings(data, 4)
    by_text: dict[str, list[int]] = {}
    for row in strings:
        by_text.setdefault(str(row["text"]), []).append(int(row["offset"]))

    proc = subprocess.run(
        [objdump, "-d", "-Mintel", str(executable)],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    lines = proc.stdout.splitlines()

    result = {
        "executable": executable.name,
        "sha256": probe["sha256"],
        "image_base": probe.get("image_base"),
        "targets": [],
    }

    for target in targets or DEFAULT_TARGETS:
        offsets = by_text.get(target, [])
        records = []
        for offset in offsets:
            va = file_offset_to_va(probe, offset)
            records.append({
                "file_offset": offset,
                "va": va,
                "references": [] if va is None else find_disassembly_references(lines, va, context),
            })
        result["targets"].append({"text": target, "occurrences": records})
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description="Find PE xrefs to selected debug strings")
    parser.add_argument("executable", type=Path)
    parser.add_argument("--target", action="append", default=[])
    parser.add_argument("--context", type=int, default=8)
    parser.add_argument("--objdump", default="objdump")
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    try:
        report = analyse(
            args.executable,
            targets=args.target or None,
            context=args.context,
            objdump=args.objdump,
        )
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        parser.error(str(exc))

    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
