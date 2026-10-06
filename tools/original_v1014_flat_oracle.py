#!/usr/bin/env python3
"""Original v1.014 launch-to-rest oracle on controlled terrain descriptors."""

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
    ru16,
    ru32,
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
HOLE_BRANCH_VA = 0x40A730
GROUND_STOP_VA = 0x40AA78


def descriptor_info(uc, terrain_index: int) -> dict:
    if not 0 <= terrain_index < 128:
        raise ValueError("terrain index outside scoped table")
    pointer = ru32(uc, TERRAIN_POINTER_TABLE_VA + terrain_index * 4)
    landing_code = ru16(uc, pointer)
    variant_raw = ru16(uc, pointer + 2)
    profile_slot = ru16(uc, pointer + 4)
    raw = bytes(uc.mem_read(pointer + 6, 48)).split(b"\0", 1)[0]
    name = raw.decode("ascii", "replace")
    variant = variant_raw - 0x10000 if variant_raw & 0x8000 else variant_raw
    return {
        "terrain_index": terrain_index,
        "pointer": pointer,
        "landing_code": landing_code,
        "variant": variant,
        "profile_slot": profile_slot,
        "name": name,
    }


def prepare_surface(uc, terrain_index: int | None) -> dict:
    if terrain_index is None:
        # Legacy neutral oracle used by the original generic-flat suite.
        wu32(uc, TERRAIN_POINTER_TABLE_VA, TERRAIN_DESCRIPTOR)
        w16(uc, TERRAIN_DESCRIPTOR, 0)
        info = {
            "terrain_index": 0,
            "landing_code": 0,
            "variant": 0,
            "profile_slot": None,
            "name": "GENERIC FLAT",
        }
    else:
        info = descriptor_info(uc, terrain_index)

    # Avoid audiovisual side effects; gameplay arithmetic is untouched.
    w16(uc, PLAYER + 0x60, 1)
    return info


def complete_terrain_call(uc, terrain_index: int | None):
    from unicorn.x86_const import UC_X86_REG_EBX, UC_X86_REG_ESP

    sp = uc.reg_read(UC_X86_REG_ESP)
    ret = struct.unpack("<I", bytes(uc.mem_read(sp, 4)))[0]
    if ret != TERRAIN_RETURN_VA:
        raise RuntimeError(f"unexpected terrain return {ret:#x}")

    value = 0 if terrain_index is None else terrain_index
    ebx = uc.reg_read(UC_X86_REG_EBX)
    uc.reg_write(UC_X86_REG_EBX, (ebx & 0xFFFF0000) | (value & 0xFFFF))
    uc.reg_write(UC_X86_REG_ESP, sp + 4)


def run_tick(uc, tick, landing, terrain_index):
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
        elif address == HOLE_BRANCH_VA:
            stop_reason = "holed"
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
                complete_terrain_call(uc, terrain_index)
                start = TERRAIN_RETURN_VA
                continue
            if stop_reason == "holed":
                return current_landing, "holed"
            if stop_reason == "done":
                return current_landing, None
            eip = uc.reg_read(UC_X86_REG_EIP)
            raise RuntimeError(f"tick stopped unexpectedly at {eip:#x}")
        raise RuntimeError("too many terrain helper passes in one tick")
    finally:
        uc.hook_del(token)


def run_flat(exe, club, lie, power, accuracy, direction, max_ticks, terrain_index=None):
    uc, digest = build_uc(exe)

    surface = prepare_surface(uc, terrain_index)
    if surface["profile_slot"] is not None and lie != surface["profile_slot"]:
        raise ValueError(
            f"lie/profile slot {lie} does not match {surface['name']} "
            f"descriptor slot {surface['profile_slot']}")

    profile, lower, upper = setup(
        uc, club, lie, power, accuracy, direction)
    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)

    w16(uc, BALL + 0x1C, 100)

    samples = [sample(uc, 0)]
    landing = None
    rest = None
    holed = False

    for tick in range(1, max_ticks + 1):
        landing, terminal = run_tick(uc, tick, landing, terrain_index)
        row = sample(uc, tick)
        samples.append(row)
        if terminal == "holed":
            holed = True
            rest = {"tick": tick, "x": row["x"], "y": row["y"]}
            break
        if (
            row["height"] == 0
            and row["vertical_force"] == 0
            and row["horizontal_force"] == 0
        ):
            rest = {"tick": tick, "x": row["x"], "y": row["y"]}
            break

    if landing is None or rest is None:
        raise RuntimeError("original controlled-surface shot did not reach final rest")

    return {
        "oracle": "original-v1.014-controlled-surface",
        "build_sha256": digest,
        "terrain_index": surface["terrain_index"],
        "surface_name": surface["name"],
        "surface_code": surface["landing_code"],
        "surface_variant": surface["variant"],
        "profile_slot": lie,
        "profile_id": profile,
        "profile_bounds": [lower, upper],
        "samples": samples,
        "events": {"landing": landing, "rest": rest, "holed": holed},
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    for name in ("club", "lie", "power", "accuracy", "direction", "max-ticks"):
        ap.add_argument(f"--{name}", type=int, required=True)
    ap.add_argument("--terrain-index", type=int)
    ap.add_argument("-o", "--output", type=Path)
    a = ap.parse_args()

    result = run_flat(
        a.exe, a.club, a.lie, a.power, a.accuracy,
        a.direction, a.max_ticks, a.terrain_index)
    text = json.dumps(result, indent=2) + "\n"
    if a.output:
        a.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
