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
    wu32,
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
CURRENT_ORDER_VA = 0x425580
CURRENT_HOLE_INDEX_VA = 0x42952B
SCORE_UPDATE_VA = 0x4125AB
SCORE_UPDATE_STATE_DONE_VA = 0x4125D5
PLAYER_COUNT_VA = 0x429527
CANONICAL_PLAYER_VA = 0x425FE2
CANONICAL_BALL_VA = 0x4287D6
TURN_ADJUST_VA = 0x40C005
TURN_ADJUST_EXIT_VA = 0x40BEF4
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



def post_hole_turn_adjust_case(exe: Path) -> dict:
    """Exercise original code8 putt then the single-player turn-adjust branch."""
    uc, digest = build_uc(exe)
    setup(uc, 12, 6, 30, 63, 0)
    w16(uc, PLAYER + 0x52, 0)
    w16(uc, PLAYER + 0x56, 0)

    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)
    w16(uc, BALL + 0x1C, 100)
    w16(uc, GREEN_MODE_VA, 1)
    for _ in range(3):
        run_putter_tick_existing(uc, 31)
    terminal = run_putter_tick_existing(uc, 7)

    before_adjust = {
        "counter_52": ru16(uc, PLAYER + 0x52),
        "counter_56": ru16(uc, PLAYER + 0x56),
        "ball_state_18": ru32(uc, BALL + 0x18),
    }

    # The shot oracle executes with scratch PLAYER/BALL addresses, while the
    # original turn-selection routine hardcodes the production arrays at
    # 0x425FE2 / 0x4287D6. Copy the exact resulting state back into those
    # canonical arrays before entering game-flow code.
    uc.mem_write(
        CANONICAL_PLAYER_VA,
        bytes(uc.mem_read(PLAYER, 0x86)),
    )
    uc.mem_write(
        CANONICAL_BALL_VA,
        bytes(uc.mem_read(BALL, 0x2C)),
    )

    # Original single-player turn-selection path.
    uc.mem_write(GAME_MODE_VA, b"\x01")
    wu32(uc, PLAYER_COUNT_VA, 1)
    uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)

    branch_trace = []
    watch = {
        0x40C022, 0x40C026, 0x40C033, 0x40C03A, 0x40C03E,
        0x40C04F, 0x40C051, 0x40C065, 0x40C06A, 0x40C06C,
        0x40C070, 0x40C074, 0x40C07F, 0x40C083, 0x40C087,
        0x40C08C, 0x40C098, TURN_ADJUST_EXIT_VA,
    }
    reached_exit = False

    def hook(machine, address, size, user_data):
        nonlocal reached_exit
        if address in watch:
            branch_trace.append({
                "address": address,
                "counter_52": ru16(machine, CANONICAL_PLAYER_VA + 0x52),
                "counter_56": ru16(machine, CANONICAL_PLAYER_VA + 0x56),
                "player_5e": ru16(machine, CANONICAL_PLAYER_VA + 0x5E),
                "ball_18": ru32(machine, CANONICAL_BALL_VA + 0x18),
            })
        if address == TURN_ADJUST_EXIT_VA:
            reached_exit = True
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        uc.emu_start(TURN_ADJUST_VA, SENTINEL, count=20000)
    finally:
        uc.hook_del(token)

    return {
        "case": "post_code8_single_player_turn_adjust",
        "build_sha256": digest,
        "terminal": terminal,
        "before_adjust": before_adjust,
        "canonical_before_adjust": {
            "counter_52": ru16(uc, CANONICAL_PLAYER_VA + 0x52)
                if not reached_exit else branch_trace[0]["counter_52"],
            "counter_56": ru16(uc, CANONICAL_PLAYER_VA + 0x56)
                if not reached_exit else branch_trace[0]["counter_56"],
        },
        "after_adjust": {
            "counter_52": ru16(uc, CANONICAL_PLAYER_VA + 0x52),
            "counter_56": ru16(uc, CANONICAL_PLAYER_VA + 0x56),
        },
        "branch_trace": branch_trace,
        "reached_turn_adjust_exit": reached_exit,
    }



def hazard_counter_case(exe: Path, terrain_index: int = 6) -> dict:
    """Run original launch -> hazard terminal -> recovery with one emulator."""
    from original_v1014_flat_oracle import prepare_surface, run_tick

    HAZARD_RECOVERY_VA = 0x40A7E0
    HAZARD_RECOVERY_STOP_VA = 0x40A8D6
    X_EXTENT_VA = 0x41EE1A
    Y_EXTENT_VA = 0x41EE1E
    RECOVERY_HELPERS = {0x4094F0, 0x40C2F8, 0x40B791, 0x40B998}

    uc, digest = build_uc(exe)
    surface = prepare_surface(uc, terrain_index)
    lie = int(surface["profile_slot"])
    setup(uc, 5, lie, 83, 63, 777)
    w16(uc, PLAYER + 0x52, 0)
    w16(uc, PLAYER + 0x56, 0)

    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)
    after_launch = [
        ru16(uc, PLAYER + 0x52),
        ru16(uc, PLAYER + 0x56),
    ]
    w16(uc, BALL + 0x1C, 100)

    landing = None
    terminal_tick = None
    for tick in range(1, 512):
        landing, _ = run_tick(uc, tick, landing, terrain_index)
        height = ru32(uc, BALL + 0x14)
        vforce = ru32(uc, BALL + 0x08)
        hforce = ru32(uc, BALL + 0x0C)
        if height == 0 and vforce == 0 and hforce == 0:
            terminal_tick = tick
            break
    if terminal_tick is None:
        raise RuntimeError("hazard case did not reach stopped state")

    after_terminal = [
        ru16(uc, PLAYER + 0x52),
        ru16(uc, PLAYER + 0x56),
    ]

    # Enter the recovered post-pause branch with permissive course extents;
    # this check is about counter ownership, not position geometry (which has
    # its own zero-tolerance recovery oracle).
    w16(uc, BALL + 0x1C, 0)
    wu32(uc, X_EXTENT_VA, 0x7FFF)
    wu32(uc, Y_EXTENT_VA, 0x7FFF)
    uc.reg_write(UC_X86_REG_ESI, PLAYER)
    uc.reg_write(UC_X86_REG_EDI, BALL)
    uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)

    reached_recovery_stop = False

    def recovery_hook(machine, address, size, user_data):
        nonlocal reached_recovery_stop
        if address == TERRAIN_LOOKUP_VA:
            ret = skip_call(machine)
            ebx = machine.reg_read(UC_X86_REG_EBX)
            machine.reg_write(
                UC_X86_REG_EBX,
                (ebx & 0xFFFF0000) | (terrain_index & 0xFFFF),
            )
            machine.reg_write(UC_X86_REG_EIP, ret)
        elif address in RECOVERY_HELPERS:
            machine.reg_write(UC_X86_REG_EIP, skip_call(machine))
        elif address == HAZARD_RECOVERY_STOP_VA:
            reached_recovery_stop = True
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, recovery_hook)
    try:
        uc.emu_start(HAZARD_RECOVERY_VA, SENTINEL, count=30000)
    finally:
        uc.hook_del(token)

    after_recovery = [
        ru16(uc, PLAYER + 0x52),
        ru16(uc, PLAYER + 0x56),
    ]

    return {
        "case": "water_hazard_counter_lifecycle",
        "build_sha256": digest,
        "terrain_index": terrain_index,
        "surface_name": surface["name"],
        "after_launch": after_launch,
        "terminal_tick": terminal_tick,
        "after_terminal": after_terminal,
        "after_recovery": after_recovery,
        "reached_recovery_stop": reached_recovery_stop,
    }



def score_update_case(
    exe: Path,
    hole_index: int,
    total_strokes: int,
    cumulative_par_before: int,
) -> dict:
    """Execute original 0x4125AB through its authoritative state writes."""
    uc, digest = build_uc(exe)
    setup(uc, 5, 4, 30, 63, 0)

    order_ptr = ru32(uc, ORDER_POINTER_TABLE_VA)
    if not (0x400000 <= order_ptr < 0x500000):
        raise RuntimeError(f"invalid recovered order pointer {order_ptr:#x}")
    if not 0 <= hole_index < 18:
        raise ValueError("hole_index must be 0..17")

    hole_id = bytes(uc.mem_read(order_ptr + hole_index, 1))[0]
    par = bytes(uc.mem_read(PAR_TABLE_VA + hole_id, 1))[0]

    wu32(uc, CURRENT_ORDER_VA, order_ptr)
    wu32(uc, CURRENT_HOLE_INDEX_VA, hole_index)
    w16(uc, PLAYER + 0x52, total_strokes)
    w16(uc, PLAYER + 0x56, total_strokes)
    w16(uc, PLAYER + 0x58, cumulative_par_before)
    w16(uc, PLAYER + 0x48, 0x7FFF)

    uc.emu_start(
        SCORE_UPDATE_VA,
        SCORE_UPDATE_STATE_DONE_VA,
        count=5000,
    )

    cumulative_after = ru16(uc, PLAYER + 0x58)
    relative_raw = ru16(uc, PLAYER + 0x48)
    relative_signed = (
        relative_raw - 0x10000
        if relative_raw & 0x8000
        else relative_raw
    )

    return {
        "case": "original_score_update",
        "build_sha256": digest,
        "hole_index": hole_index,
        "hole_id": hole_id,
        "par": par,
        "total_strokes": total_strokes,
        "cumulative_par_before": cumulative_par_before,
        "cumulative_par_after": cumulative_after,
        "relative_to_par": relative_signed,
        "counter_52_unchanged": ru16(uc, PLAYER + 0x52),
        "counter_56_unchanged": ru16(uc, PLAYER + 0x56),
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

    post_hole_adjust = post_hole_turn_adjust_case(args.exe)
    assert post_hole_adjust["before_adjust"]["counter_52"] == 2
    assert post_hole_adjust["before_adjust"]["counter_56"] == 2
    assert post_hole_adjust["reached_turn_adjust_exit"] is True
    assert post_hole_adjust["after_adjust"]["counter_52"] == 1, post_hole_adjust
    assert post_hole_adjust["after_adjust"]["counter_56"] == 1, post_hole_adjust

    hazard = hazard_counter_case(args.exe)
    assert hazard["after_launch"] == [1, 1], hazard
    # Landing-code-35 hazard stop/recovery does not mutate these counters in
    # the recovered physics/recovery path. Penalty ownership therefore remains
    # outside this fragment until the game-flow dispatcher is classified.
    assert hazard["after_terminal"] == [1, 1], hazard
    assert hazard["reached_recovery_stop"] is True, hazard
    assert hazard["after_recovery"] == [1, 1], hazard

    score_first = score_update_case(
        args.exe, hole_index=0, total_strokes=3, cumulative_par_before=0)
    assert score_first["cumulative_par_after"] == score_first["par"]
    assert score_first["relative_to_par"] == score_first["par"] - 3
    assert score_first["counter_52_unchanged"] == 3
    assert score_first["counter_56_unchanged"] == 3

    score_second = score_update_case(
        args.exe,
        hole_index=1,
        total_strokes=7,
        cumulative_par_before=score_first["par"],
    )
    assert score_second["cumulative_par_after"] == (
        score_first["par"] + score_second["par"])
    assert score_second["relative_to_par"] == (
        score_second["cumulative_par_after"] - 7)

    report = {
        "reference": "Sensible Golf Windows v1.014",
        "continuous_cases": rows,
        "real_putter_to_cup": real_putter,
        "post_hole_turn_adjust": post_hole_adjust,
        "hazard_counter_lifecycle": hazard,
        "score_update_cases": [score_first, score_second],
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
