#!/usr/bin/env python3
"""Trace original v1.014 per-player +0x52/+0x56 counters across shot lifecycle."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from original_v1014_oracle import (
    LIVE_LAUNCH_VA,
    PLAYER,
    SENTINEL,
    build_uc,
    ru16,
    setup,
    w16,
)
from original_v1014_putter_terminal_oracle import run as run_putter_terminal


def launch_case(exe: Path, label: str, club: int, lie: int) -> dict:
    uc, digest = build_uc(exe)
    setup(uc, club, lie, 30, 63, 0)
    w16(uc, PLAYER + 0x52, 7)
    w16(uc, PLAYER + 0x56, 11)
    before = [ru16(uc, PLAYER + 0x52), ru16(uc, PLAYER + 0x56)]
    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)
    after = [ru16(uc, PLAYER + 0x52), ru16(uc, PLAYER + 0x56)]
    return {
        "case": label,
        "build_sha256": digest,
        "before": before,
        "after_launch": after,
        "delta": [after[0] - before[0], after[1] - before[1]],
    }


def terminal_case(exe: Path, terrain_index: int, label: str) -> dict:
    row = run_putter_terminal(exe, terrain_index)
    return {
        "case": label,
        "terrain_index": terrain_index,
        "transition": row["transition"],
        "counter_52": row["counter_52"],
        "counter_56": row["counter_56"],
        "events": row["events"],
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    rows = [
        launch_case(args.exe, "iron_launch", 5, 4),
        launch_case(args.exe, "putter_launch", 12, 6),
        terminal_case(args.exe, 7, "putter_code8"),
        terminal_case(args.exe, 23, "putter_code9"),
        terminal_case(args.exe, 15, "putter_code10"),
    ]

    # Original terminal oracle begins terminal cases with 7/11.
    assert rows[0]["delta"] == [1, 1]
    assert rows[1]["delta"] == [1, 1]
    assert [rows[2]["counter_52"], rows[2]["counter_56"]] == [8, 12]
    assert [rows[3]["counter_52"], rows[3]["counter_56"]] == [7, 11]
    assert [rows[4]["counter_52"], rows[4]["counter_56"]] == [7, 11]

    report = {
        "reference": "Sensible Golf Windows v1.014",
        "rows": rows,
        "proven": {
            "launch_delta_52": 1,
            "launch_delta_56": 1,
            "putter_code8_terminal_delta_52": 1,
            "putter_code8_terminal_delta_56": 1,
            "putter_code9_terminal_delta": [0, 0],
            "putter_code10_terminal_delta": [0, 0],
        },
    }

    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
