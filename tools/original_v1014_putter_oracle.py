#!/usr/bin/env python3
"""Execute the original Sensible Golf v1.014 club-12 flat-green putting path."""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EBX,
    UC_X86_REG_EDI,
    UC_X86_REG_EIP,
    UC_X86_REG_ESI,
    UC_X86_REG_ESP,
)

from original_v1014_oracle import (
    ADJ_X_VA,
    ADJ_Y_VA,
    BALL,
    GREEN_MODE_VA,
    LIVE_LAUNCH_VA,
    PLAYER,
    SENTINEL,
    SPECIAL_MODE_VA,
    STACK,
    build_uc,
    ri32,
    ru32,
    sample,
    setup,
    w16,
    wi32,
    wu32,
)
from original_v1014_flat_oracle import descriptor_info

TERRAIN_LOOKUP_VA = 0x409535
PUTTER_TICK_VA = 0x40A581
TICK_END_VA = 0x40AA78
SLOPE_ADJUST_VA = 0x40B965
DEFAULT_GREEN_TERRAIN_INDEX = 31  # GREEN H4, landing code 1, profile slot 6.


def complete_terrain_call(uc, terrain_index: int) -> int:
    sp = uc.reg_read(UC_X86_REG_ESP)
    ret = struct.unpack("<I", bytes(uc.mem_read(sp, 4)))[0]
    ebx = uc.reg_read(UC_X86_REG_EBX)
    uc.reg_write(UC_X86_REG_EBX, (ebx & 0xFFFF0000) | (terrain_index & 0xFFFF))
    uc.reg_write(UC_X86_REG_ESP, sp + 4)
    return ret


def compute_slope_adjustment(uc, direction: int, magnitude: int) -> tuple[int, int]:
    w16(uc, BALL + 0x26, direction)
    w16(uc, BALL + 0x2A, magnitude)
    sp = STACK + 0xF000 - 4
    wu32(uc, sp, SENTINEL)
    uc.reg_write(UC_X86_REG_ESP, sp)
    uc.emu_start(SLOPE_ADJUST_VA, SENTINEL, count=1000)
    return ri32(uc, ADJ_X_VA), ri32(uc, ADJ_Y_VA)


def run_putter_tick(
    uc,
    terrain_index: int,
    landing_code: int,
    slope_active: bool = False,
) -> None:
    stop_reason = None

    def hook(machine, address, size, user_data):
        nonlocal stop_reason
        if address == TERRAIN_LOOKUP_VA:
            stop_reason = "terrain"
            machine.emu_stop()
        elif address == TICK_END_VA:
            stop_reason = "done"
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        w16(uc, GREEN_MODE_VA, 1)
        w16(uc, SPECIAL_MODE_VA, 2 if slope_active else landing_code)
        if not slope_active:
            wi32(uc, ADJ_X_VA, 0)
            wi32(uc, ADJ_Y_VA, 0)
        uc.reg_write(UC_X86_REG_ESI, PLAYER)
        uc.reg_write(UC_X86_REG_EDI, BALL)
        uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)

        start = PUTTER_TICK_VA
        for _ in range(8):
            stop_reason = None
            uc.emu_start(start, SENTINEL, count=20000)

            if stop_reason == "terrain":
                start = complete_terrain_call(uc, terrain_index)
                continue
            if stop_reason == "done":
                return

            eip = uc.reg_read(UC_X86_REG_EIP)
            raise RuntimeError(f"putter tick stopped unexpectedly at {eip:#x}")

        raise RuntimeError("too many terrain helper passes in one putter tick")
    finally:
        uc.hook_del(token)


def run_putter(
    exe: Path,
    power: int,
    accuracy: int,
    direction: int,
    max_ticks: int,
    terrain_index: int = DEFAULT_GREEN_TERRAIN_INDEX,
    slope_direction: int = 0,
    slope_magnitude: int = 0,
) -> dict:
    uc, digest = build_uc(exe)
    surface = descriptor_info(uc, terrain_index)

    club = 12
    lie = int(surface["profile_slot"])
    profile, lower, upper = setup(
        uc, club, lie, power, accuracy, direction)

    if int(surface["landing_code"]) >= 8:
        raise ValueError(
            "flat-green putter oracle currently scopes a green descriptor with landing code < 8")

    # Keep side-effect/audio branches inert. Kinematic logic remains original.
    w16(uc, PLAYER + 0x60, 1)

    uc.emu_start(LIVE_LAUNCH_VA, SENTINEL, count=5000)

    # The update branch treats the ball as already on the green surface.
    w16(uc, BALL + 0x1C, 100)
    w16(uc, GREEN_MODE_VA, 1)

    slope_active = slope_magnitude != 0
    if slope_active:
        adj_x, adj_y = compute_slope_adjustment(
            uc, slope_direction & 0x0FFF, slope_magnitude)
    else:
        adj_x = adj_y = 0
        wi32(uc, ADJ_X_VA, 0)
        wi32(uc, ADJ_Y_VA, 0)

    samples = [sample(uc, 0)]
    rest = None

    for tick in range(1, max_ticks + 1):
        run_putter_tick(
            uc,
            terrain_index=terrain_index,
            landing_code=int(surface["landing_code"]),
            slope_active=slope_active,
        )
        row = sample(uc, tick)
        samples.append(row)

        # Kinematic rest for putting: no remaining horizontal movement.
        if row["horizontal_force"] == 0:
            rest = {"tick": tick, "x": row["x"], "y": row["y"]}
            break

    if rest is None:
        raise RuntimeError("original flat-green putt did not reach rest")

    return {
        "oracle": "original-v1.014-club12-flat-green",
        "build_sha256": digest,
        "terrain_index": terrain_index,
        "surface_name": surface["name"],
        "surface_code": int(surface["landing_code"]),
        "profile_slot": lie,
        "profile_id": profile,
        "profile_bounds": [lower, upper],
        "slope": {
            "direction": slope_direction & 0x0FFF,
            "magnitude": slope_magnitude,
            "adj_x_raw": adj_x,
            "adj_y_raw": adj_y,
        },
        "input": {
            "club": club,
            "lie": lie,
            "power": power,
            "accuracy_tick": accuracy,
            "direction": direction & 0xFFF,
        },
        "samples": samples,
        "events": {
            "landing": {"tick": 0, "x": samples[0]["x"], "y": samples[0]["y"]},
            "rest": rest,
        },
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("--power", type=int, required=True)
    ap.add_argument("--accuracy", type=int, default=63)
    ap.add_argument("--direction", type=int, default=0)
    ap.add_argument("--max-ticks", type=int, default=512)
    ap.add_argument("--terrain-index", type=int, default=DEFAULT_GREEN_TERRAIN_INDEX)
    ap.add_argument("--slope-direction", type=int, default=0)
    ap.add_argument("--slope-magnitude", type=int, default=0)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    report = run_putter(
        args.exe,
        args.power,
        args.accuracy,
        args.direction,
        args.max_ticks,
        args.terrain_index,
        args.slope_direction,
        args.slope_magnitude,
    )
    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
