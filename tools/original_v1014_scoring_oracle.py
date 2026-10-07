#!/usr/bin/env python3
"""Continuous original v1.014 scoring/counter observations.

This intentionally keeps original field names as offsets until semantics are
closed by end-to-end lifecycle evidence.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EBX,
    UC_X86_REG_EDI,
    UC_X86_REG_EDX,
    UC_X86_REG_EIP,
    UC_X86_REG_ESI,
    UC_X86_REG_ESP,
)

from original_v1014_oracle import (
    BALL,
    GREEN_MODE_VA,
    LIVE_LAUNCH_VA,
    PLAYER,
    SENTINEL,
    STACK,
    build_uc,
    ru16,
    ru32,
    setup,
    w16,
)

TERMINAL_START_VA = 0x40A5A2
VISUAL_HELPER_VA = 0x40C4C0
EVENT_DISPATCH_VA = 0x403EF2
CODE9_10_TRANSITION_VA = 0x40CD00
CODE8_TRANSITION_VA = 0x40CD25
TERMINAL_FLAG_VA = 0x41F883
UI_GATE_VA = 0x41D64B
GAME_MODE_VA = 0x42558E
PAR_TABLE_VA = 0x41E603
ORDER_POINTER_TABLE_VA = 0x41E64C
TERRAIN_LOOKUP_VA = 0x409535
PUTTER_TICK_VA = 0x40A581
TICK_END_VA = 0x40AA78


def skip_call(uc) -> int:
    sp = uc.reg_read(UC_X86_REG_ESP)
    ret = struct.unpack("<I", bytes(uc.mem_read(sp, 4)))[0]
    uc.reg_write(UC_X86_REG_ESP, sp + 4)
    return ret


def run_terminal_existing(uc, terrain_index: int) -> dict:
    w16(uc, TERMINAL_FLAG_VA, 0)
    uc.mem_write(UI_GATE_VA, b"\0")
    uc.mem_write(GAME_MODE_VA, b"\0")
    uc.reg_write(UC_X86_REG_ESI, PLAYER)
    uc.reg_write(UC_X86_REG_EDI, BALL)
    uc.reg_write(UC_X86_REG_EBX, terrain_index & 0xFFFF)
    uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)

    events: list[int] = []
    transition = None

    def hook(machine, address, size, user_data):
        nonlocal transition
        if address == VISUAL_HELPER_VA:
            machine.reg_write(UC_X86_REG_EIP, skip_call(machine))
        elif address == EVENT_DISPATCH_VA:
            events.append(machine.reg_read(UC_X86_REG_EDX) & 0xFFFFFFFF)
            machine.reg_write(UC_X86_REG_EIP, skip_call(machine))
        elif address == CODE9_10_TRANSITION_VA:
            transition = "special"
            machine.emu_stop()
        elif address == CODE8_TRANSITION_VA:
            transition = "hole"
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        uc.emu_start(TERMINAL_START_VA, SENTINEL, count=5000)
    finally:
        uc.hook_del(token)

    if transition is None:
        raise RuntimeError("terminal path did not reach scoped transition")

    return {
        "transition": transition,
        "events": events,
        "counter_52": ru16(uc, PLAYER + 0x52),
        "counter_56": ru16(uc, PLAYER + 0x56),
    }


def sequential_case(exe: Path, terrain_index: int, label: str) -> dict:
    uc, digest = build_uc(exe)
    setup(uc, 12, 6, 30, 63, 0)
    w16(uc, PLAYER + 0x52, 0)
    w16(uc, PLAYER + 0x56, 0)
    before = [ru16(uc, PLAYER + 0x52), ru16(uc, PLAYER + 0x56)]

    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)
    after_launch = [ru16(uc, PLAYER + 0x52), ru16(uc, PLAYER + 0x56)]

    terminal = run_terminal_existing(uc, terrain_index)
    after_terminal = [terminal["counter_52"], terminal["counter_56"]]

    return {
        "case": label,
        "build_sha256": digest,
        "before": before,
        "after_launch": after_launch,
        "after_terminal": after_terminal,
        "launch_delta": [
            after_launch[0] - before[0],
            after_launch[1] - before[1],
        ],
        "terminal_delta": [
            after_terminal[0] - after_launch[0],
            after_terminal[1] - after_launch[1],
        ],
        "transition": terminal["transition"],
        "events": terminal["events"],
    }




def run_putter_tick_existing(uc, terrain_index: int) -> dict:
    """Run one real original club-12 update with a controlled terrain lookup."""
    stop_reason = None
    transition = None
    events: list[int] = []

    def hook(machine, address, size, user_data):
        nonlocal stop_reason, transition
        if address == TERRAIN_LOOKUP_VA:
            sp = machine.reg_read(UC_X86_REG_ESP)
            ret = struct.unpack("<I", bytes(machine.mem_read(sp, 4)))[0]
            ebx = machine.reg_read(UC_X86_REG_EBX)
            machine.reg_write(
                UC_X86_REG_EBX,
                (ebx & 0xFFFF0000) | (terrain_index & 0xFFFF),
            )
            machine.reg_write(UC_X86_REG_ESP, sp + 4)
            machine.reg_write(UC_X86_REG_EIP, ret)
        elif address == VISUAL_HELPER_VA:
            machine.reg_write(UC_X86_REG_EIP, skip_call(machine))
        elif address == EVENT_DISPATCH_VA:
            events.append(machine.reg_read(UC_X86_REG_EDX) & 0xFFFFFFFF)
            machine.reg_write(UC_X86_REG_EIP, skip_call(machine))
        elif address == CODE9_10_TRANSITION_VA:
            transition = "special"
            stop_reason = "transition"
            machine.emu_stop()
        elif address == CODE8_TRANSITION_VA:
            transition = "hole"
            stop_reason = "transition"
            machine.emu_stop()
        elif address == TICK_END_VA:
            stop_reason = "done"
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        uc.reg_write(UC_X86_REG_ESI, PLAYER)
        uc.reg_write(UC_X86_REG_EDI, BALL)
        uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)
        uc.emu_start(PUTTER_TICK_VA, SENTINEL, count=30000)
    finally:
        uc.hook_del(token)

    if stop_reason not in ("done", "transition"):
        raise RuntimeError(
            f"putter tick stopped unexpectedly at "
            f"{uc.reg_read(UC_X86_REG_EIP):#x}"
        )

    return {
        "stop_reason": stop_reason,
        "transition": transition,
        "events": events,
        "counter_52": ru16(uc, PLAYER + 0x52),
        "counter_56": ru16(uc, PLAYER + 0x56),
    }


def real_putter_to_cup_case(exe: Path) -> dict:
    uc, digest = build_uc(exe)
    setup(uc, 12, 6, 30, 63, 0)
    w16(uc, PLAYER + 0x52, 0)
    w16(uc, PLAYER + 0x56, 0)

    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)
    w16(uc, BALL + 0x1C, 100)
    w16(uc, GREEN_MODE_VA, 1)

    trace = [{
        "stage": "after_launch",
        "counter_52": ru16(uc, PLAYER + 0x52),
        "counter_56": ru16(uc, PLAYER + 0x56),
    }]

    for tick in range(1, 4):
        row = run_putter_tick_existing(uc, 31)
        trace.append({
            "stage": f"green_tick_{tick}",
            "counter_52": row["counter_52"],
            "counter_56": row["counter_56"],
            "stop_reason": row["stop_reason"],
        })

    terminal = run_putter_tick_existing(uc, 7)
    trace.append({
        "stage": "code8_tick",
        "counter_52": terminal["counter_52"],
        "counter_56": terminal["counter_56"],
        "stop_reason": terminal["stop_reason"],
        "transition": terminal["transition"],
        "events": terminal["events"],
    })

    return {
        "case": "real_putter_ticks_to_code8",
        "build_sha256": digest,
        "trace": trace,
    }

def dump_static_tables(exe: Path) -> dict:
    uc, _ = build_uc(exe)

    # The par lookup uses a byte index into this table. Keep enough bytes to
    # include every observed index while avoiding semantic guesses.
    par_bytes = list(bytes(uc.mem_read(PAR_TABLE_VA, 128)))

    order_rows = []
    for index in range(16):
        ptr = ru32(uc, ORDER_POINTER_TABLE_VA + index * 4)
        if not (0x400000 <= ptr < 0x500000):
            order_rows.append({"index": index, "pointer": ptr, "valid": False})
            continue
        values = list(bytes(uc.mem_read(ptr, 18)))
        order_rows.append({
            "index": index,
            "pointer": ptr,
            "valid": True,
            "holes": values,
            "pars": [par_bytes[v] if v < len(par_bytes) else None for v in values],
        })

    return {
        "par_table_va": PAR_TABLE_VA,
        "par_bytes_128": par_bytes,
        "order_pointer_table_va": ORDER_POINTER_TABLE_VA,
        "order_rows": order_rows,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    rows = [
        sequential_case(args.exe, 7, "putter_code8_continuous"),
        sequential_case(args.exe, 23, "putter_code9_continuous"),
        sequential_case(args.exe, 15, "putter_code10_continuous"),
    ]

    assert rows[0]["after_launch"] == [1, 1]
    assert rows[0]["terminal_delta"] == [1, 1]
    assert rows[1]["after_launch"] == [1, 1]
    assert rows[1]["terminal_delta"] == [0, 0]
    assert rows[2]["after_launch"] == [1, 1]
    assert rows[2]["terminal_delta"] == [0, 0]

    real_putter = real_putter_to_cup_case(args.exe)
    assert real_putter["trace"][0]["counter_52"] == 1
    assert real_putter["trace"][0]["counter_56"] == 1
    for row in real_putter["trace"][1:4]:
        assert row["counter_52"] == 1, row
        assert row["counter_56"] == 1, row
    assert real_putter["trace"][4]["counter_52"] == 2
    assert real_putter["trace"][4]["counter_56"] == 2
    assert real_putter["trace"][4]["transition"] == "hole"

    report = {
        "reference": "Sensible Golf Windows v1.014",
        "continuous_cases": rows,
        "real_putter_to_cup": real_putter,
        "static_tables": dump_static_tables(args.exe),
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
