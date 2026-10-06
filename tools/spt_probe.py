#!/usr/bin/env python3
"""Analyse the 50-byte MAPMxx.SPT records without assigning guessed semantics."""

from __future__ import annotations

import argparse
import json
import re
import struct
from pathlib import Path


class SPTError(ValueError):
    pass


def parse_spt(data: bytes) -> list[tuple[int, int, int, int, int]]:
    if len(data) != 50:
        raise SPTError(f"expected 50 bytes, got {len(data)}")
    return [
        struct.unpack(">5H", data[offset : offset + 10])
        for offset in range(0, 50, 10)
    ]


def analyse_directory(root: Path) -> dict[str, object]:
    paths = sorted(
        (p for p in root.glob("MAPM*.SPT") if re.fullmatch(r"MAPM\d{2}\.SPT", p.name, re.I)),
        key=lambda p: int(re.search(r"(\d+)", p.name).group(1)),
    )
    if not paths:
        raise SPTError("no MAPMxx.SPT files found")

    parsed = {path.name: parse_spt(path.read_bytes()) for path in paths}
    sequences = {
        f"word{word}": sorted({tuple(record[word] for record in rows) for rows in parsed.values()})
        for word in range(5)
    }

    slot_ranges = []
    for slot in range(5):
        slot_records = [rows[slot] for rows in parsed.values()]
        slot_ranges.append({
            "slot": slot,
            "word0": [min(r[0] for r in slot_records), max(r[0] for r in slot_records)],
            "word1": [min(r[1] for r in slot_records), max(r[1] for r in slot_records)],
            "word2": [min(r[2] for r in slot_records), max(r[2] for r in slot_records)],
            "word3": [min(r[3] for r in slot_records), max(r[3] for r in slot_records)],
            "word4": [min(r[4] for r in slot_records), max(r[4] for r in slot_records)],
        })

    first_four_cluster_distances = []
    fifth_to_cluster_distances = []
    for rows in parsed.values():
        xy = [(r[2], r[3]) for r in rows]
        for i in range(4):
            for j in range(i + 1, 4):
                dx=xy[i][0]-xy[j][0]
                dy=xy[i][1]-xy[j][1]
                first_four_cluster_distances.append((dx*dx+dy*dy) ** 0.5)
        cx=sum(x for x,_ in xy[:4])/4
        cy=sum(y for _,y in xy[:4])/4
        dx=xy[4][0]-cx
        dy=xy[4][1]-cy
        fifth_to_cluster_distances.append((dx*dx+dy*dy) ** 0.5)

    return {
        "file_count": len(paths),
        "record_size": 10,
        "records_per_file": 5,
        "endianness": "big",
        "unique_sequences": sequences,
        "slot_ranges": slot_ranges,
        "distance_observations": {
            "first_four_pairwise_max": max(first_four_cluster_distances),
            "first_four_pairwise_mean": sum(first_four_cluster_distances)/len(first_four_cluster_distances),
            "fifth_to_first_four_centroid_min": min(fifth_to_cluster_distances),
            "fifth_to_first_four_centroid_mean": sum(fifth_to_cluster_distances)/len(fifth_to_cluster_distances),
        },
        "samples": {name:[list(r) for r in rows] for name,rows in list(parsed.items())[:3]},
    }


def main() -> int:
    parser=argparse.ArgumentParser(description="Analyse Sensible Golf MAPMxx.SPT records")
    parser.add_argument("root",type=Path)
    parser.add_argument("-o","--output",type=Path)
    args=parser.parse_args()
    try:
        report=analyse_directory(args.root)
    except (OSError,SPTError) as exc:
        parser.error(str(exc))
    text=json.dumps(report,indent=2)+"\n"
    if args.output:
        args.output.write_text(text,encoding="utf-8")
    else:
        print(text,end="")
    return 0


if __name__=="__main__":
    raise SystemExit(main())
