#!/usr/bin/env python3
"""Original v1.014 launch-to-rest oracle on a controlled generic flat surface."""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

from original_v1014_oracle import (
    ADJ_X_VA,
    ADJ_Y_VA,
    AIR_TICK_END_VA,
    AIR_TICK_VA,
    BALL,
    GREEN_MODE_VA,
    LIVE_LAUNCH_VA,
    PLAYER,
    SENTINEL,
    SPECIAL_MODE_VA,
    STACK,
    build_uc,
    ri32,
    sample,
    setup,
    w16,
    wi32,
    wu32,
)

TERRAIN_LOOKUP_VA = 0x409535
TERRAIN_RETURN_VA = 0x40A677
TERRAIN_POINTER_TABLE_VA = 0x41DFF7
TERRAIN_DESCRIPTOR = PLAYER + 0x2000
GROUND_ENTRY_VA = 0x40A65E
GROUND_STOP_VA = 0x40AA78


def prepare_surface(uc):
    wu32(uc, TERRAIN_POINTER_TABLE_VA, TERRAIN_DESCRIPTOR)
    w16(uc, TERRAIN_DESCRIPTOR, 0)
    w16(uc, PLAYER + 0x60, 1)


def complete_terrain_call(uc):
    from unicorn.x86_const import UC_X86_REG_EBX, UC_X86_REG_ESP

    sp = uc.reg_read(UC_X86_REG_ESP)
    ret = struct.unpack("<I", bytes(uc.mem_read(sp, 4)))[0]
    if ret != TERRAIN_RETURN_VA:
        raise RuntimeError(f"unexpected terrain return {ret:#x}")

    ebx = uc.reg_read(UC_X86_REG_EBX)
    uc.reg_write(UC_X86_REG_EBX, ebx & 0xFFFF0000)
    uc.reg_write(UC_X86_REG_ESP, sp + 4)


def run_tick(uc, tick, landing):
    from unicorn import UC_HOOK_CODE
    from unicorn.x86_const import (
        UC_X86_REG_EDI,
        UC_X86_REG_EIP,
        UC_X86_REG_ESI,
        UC_X86_REG_ESP,
    )

    current_landing = landing
    stop_reason = None

    def hook(machine, address, size, user_data):
        nonlocal current_landing, stop_reason
        if address == GROUND_ENTRY_VA and current_landing is None:
            current_landing = {
                "tick": tick,
                "x": ri32(machine, BALL),
                "y": ri32(machine, BALL + 4),
            }
        if address == TERRAIN_LOOKUP_VA:
            stop_reason = "terrain"
            machine.emu_stop()
        elif address in (AIR_TICK_END_VA, GROUND_STOP_VA):
            stop_reason = "done"
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        w16(uc, GREEN_MODE_VA, 0)
        w16(uc, SPECIAL_MODE_VA, 0)
        wi32(uc, ADJ_X_VA, 0)
        wi32(uc, ADJ_Y_VA, 0)
        uc.reg_write(UC_X86_REG_ESI, PLAYER)
        uc.reg_write(UC_X86_REG_EDI, BALL)
        uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)

        start = AIR_TICK_VA
        for _ in range(8):
            stop_reason = None
            uc.emu_start(start, SENTINEL, count=20000)
            if stop_reason == "terrain":
                complete_terrain_call(uc)
                start = TERRAIN_RETURN_VA
                continue
            if stop_reason == "done":
                return current_landing
            eip = uc.reg_read(UC_X86_REG_EIP)
            raise RuntimeError(f"tick stopped unexpectedly at {eip:#x}")
        raise RuntimeError("too many terrain helper passes in one tick")
    finally:
        uc.hook_del(token)


def run_flat(exe, club, lie, power, accuracy, direction, max_ticks):
    uc, digest = build_uc(exe)
    profile, lower, upper = setup(
        uc, club, lie, power, accuracy, direction)
    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)

    w16(uc, BALL + 0x1C, 100)
    prepare_surface(uc)

    samples = [sample(uc, 0)]
    landing = None
    rest = None

    for tick in range(1, max_ticks + 1):
        landing = run_tick(uc, tick, landing)
        row = sample(uc, tick)
        samples.append(row)
        if (
            row["height"] == 0
            and row["vertical_force"] == 0
            and row["horizontal_force"] == 0
        ):
            rest = {"tick": tick, "x": row["x"], "y": row["y"]}
            break

    if landing is None or rest is None:
        raise RuntimeError("original generic-flat shot did not reach final rest")

    return {
        "oracle": "original-v1.014-generic-flat",
        "build_sha256": digest,
        "surface_code": 0,
        "profile_id": profile,
        "profile_bounds": [lower, upper],
        "samples": samples,
        "events": {"landing": landing, "rest": rest},
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    for name in ("club", "lie", "power", "accuracy", "direction", "max-ticks"):
        ap.add_argument(f"--{name}", type=int, required=True)
    ap.add_argument("-o", "--output", type=Path)
    a = ap.parse_args()

    result = run_flat(
        a.exe, a.club, a.lie, a.power, a.accuracy,
        a.direction, a.max_ticks)
    text = json.dumps(result, indent=2) + "\n"
    if a.output:
        a.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
