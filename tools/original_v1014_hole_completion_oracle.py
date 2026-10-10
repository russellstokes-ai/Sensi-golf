#!/usr/bin/env python3
"""Prove original v1.014 scored-hole activation is the zero-distance branch."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_ESP

from original_v1014_oracle import (
    BALL,
    PLAYER,
    SENTINEL,
    STACK,
    build_uc,
    ru16,
    ru32,
    setup,
    w16,
    wu32,
)

START_VA = 0x40A456
SCORED_BRANCH_AFTER_UPDATE_VA = 0x40A4E4
NONZERO_DISTANCE_CONTINUE_VA = 0x40A501
CURRENT_ORDER_VA = 0x425580
CURRENT_HOLE_INDEX_VA = 0x42952B
ORDER_POINTER_TABLE_VA = 0x41E64C
PAR_TABLE_VA = 0x41E603
TERMINAL_FLAG_VA = 0x41F883


def run_case(exe: Path, distance_state: int) -> dict:
    uc, digest = build_uc(exe)
    setup(uc, 12, 6, 30, 63, 0)

    order_ptr = ru32(uc, ORDER_POINTER_TABLE_VA)
    hole_id = bytes(uc.mem_read(order_ptr, 1))[0]
    par = bytes(uc.mem_read(PAR_TABLE_VA + hole_id, 1))[0]
    wu32(uc, CURRENT_ORDER_VA, order_ptr)
    wu32(uc, CURRENT_HOLE_INDEX_VA, 0)

    # Keep the score-update formatter on its PAR branch so no presentation
    # helpers are needed: cumulative par after completion equals total strokes.
    w16(uc, PLAYER + 0x52, par)
    w16(uc, PLAYER + 0x56, par)
    w16(uc, PLAYER + 0x58, 0)
    w16(uc, PLAYER + 0x48, 0x7FFF)
    w16(uc, PLAYER + 0x70, 0)
    w16(uc, PLAYER + 0x5A, 0)

    wu32(uc, BALL + 0x18, distance_state & 0xFFFFFFFF)
    wu32(uc, BALL + 0x14, 0)
    wu32(uc, BALL + 0x0C, 0)
    wu32(uc, BALL + 0x24, 0)
    w16(uc, TERMINAL_FLAG_VA, 0)

    uc.reg_write(UC_X86_REG_ESI, PLAYER)
    uc.reg_write(UC_X86_REG_EDI, BALL)
    uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)

    stop = None

    def hook(machine, address, size, user_data):
        nonlocal stop
        if address == SCORED_BRANCH_AFTER_UPDATE_VA:
            stop = "scored"
            machine.emu_stop()
        elif address == NONZERO_DISTANCE_CONTINUE_VA:
            stop = "continue"
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        uc.emu_start(START_VA, SENTINEL, count=20000)
    finally:
        uc.hook_del(token)

    rel_raw = ru16(uc, PLAYER + 0x48)
    rel = rel_raw - 0x10000 if rel_raw & 0x8000 else rel_raw

    return {
        "reference": "Sensible Golf Windows v1.014",
        "build_sha256": digest,
        "distance_state": distance_state,
        "stop": stop,
        "hole_id": hole_id,
        "par": par,
        "current_hole_counter_52": ru16(uc, PLAYER + 0x52),
        "total_counter_56": ru16(uc, PLAYER + 0x56),
        "cumulative_par_58": ru16(uc, PLAYER + 0x58),
        "relative_to_par_48": rel,
        "completion_counter_70": ru16(uc, PLAYER + 0x70),
        "ball_flag_24": ru16(uc, BALL + 0x24),
        "terminal_flag": ru16(uc, TERMINAL_FLAG_VA),
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    zero = run_case(args.exe, 0)
    nonzero = run_case(args.exe, 1)

    assert zero["stop"] == "scored", zero
    assert zero["cumulative_par_58"] == zero["par"], zero
    assert zero["relative_to_par_48"] == 0, zero
    assert zero["completion_counter_70"] == 1, zero
    assert zero["ball_flag_24"] == 1, zero
    assert zero["terminal_flag"] == 1, zero

    assert nonzero["stop"] == "continue", nonzero
    assert nonzero["cumulative_par_58"] == 0, nonzero
    assert nonzero["relative_to_par_48"] == 0x7FFF, nonzero
    assert nonzero["completion_counter_70"] == 0, nonzero
    assert nonzero["ball_flag_24"] == 0, nonzero

    report = {
        "zero_distance": zero,
        "nonzero_distance": nonzero,
        "conclusion": (
            "Original scored-hole state is activated by the zero-distance "
            "pre-update branch; landing code 8 is a separate terminal event."
        ),
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
