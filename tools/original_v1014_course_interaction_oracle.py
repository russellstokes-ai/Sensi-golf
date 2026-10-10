#!/usr/bin/env python3
"""Execute a real MAPI lookup through the original v1.014 code-9 interaction branch."""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

from original_v1014_oracle import (
    BALL,
    PLAYER,
    STACK,
    build_uc,
    ri32,
    ru16,
    ru32,
    w16,
    wi32,
    wu32,
)
from original_v1014_prng_oracle import SEED0_VA, SEED1_VA

START_VA = 0x40A2F7
STOP_VA = 0x40A456
SLOPE_HELPER_VA = 0x40B965
EVENT_DISPATCH_VA = 0x403EF2
DESCRIPTOR_CAPTURE_VA = 0x40A308
LANDING_CAPTURE_VA = 0x40A374
CODE9_BRANCH_VA = 0x40A3AC

MAPBUF = 0x510000
DESC = 0x520000
MASK = 0x530000

MAP_PTR_VA = 0x41EE22
MAP_ROW_STRIDE_VA = 0x41EE06
MAP_X_EXTENT_VA = 0x41EE1A
MAP_Y_EXTENT_VA = 0x41EE1E
GREEN_MODE_VA = 0x41EE7A
DESC_BANK_PTR_VA = 0x41EE72
MASK_BANK_PTR_VA = 0x41EE76


def skip_called_function(uc) -> int:
    from unicorn.x86_const import UC_X86_REG_ESP

    sp = uc.reg_read(UC_X86_REG_ESP)
    ret = struct.unpack("<I", bytes(uc.mem_read(sp, 4)))[0]
    uc.reg_write(UC_X86_REG_ESP, sp + 4)
    return ret


def run(
    exe: Path,
    desc_path: Path,
    mask_path: Path,
    tile: int,
    x: int,
    y: int,
    seed0: int,
    seed1: int,
    direction: int,
    vforce: int,
    hforce: int,
    height: int,
) -> dict:
    from unicorn import UC_HOOK_CODE, UC_PROT_ALL
    from unicorn.x86_const import (
        UC_X86_REG_EBX,
        UC_X86_REG_EDI,
        UC_X86_REG_EIP,
        UC_X86_REG_ESI,
        UC_X86_REG_ESP,
    )

    desc = desc_path.read_bytes()
    mask = mask_path.read_bytes()
    if len(desc) != len(mask) or len(desc) % 8:
        raise ValueError("MAPI banks must have equal eight-byte-aligned sizes")
    if not (0 <= tile < len(desc) // 8):
        raise ValueError("tile outside MAPI bank")
    if not (0 <= x < 8 and 0 <= y < 4):
        raise ValueError("subcell outside 8x4 range")

    uc, digest = build_uc(exe)
    uc.mem_map(MAPBUF, 0x30000, UC_PROT_ALL)
    uc.mem_write(DESC, desc)
    uc.mem_write(MASK, mask)
    uc.mem_write(MAPBUF + 0x60, struct.pack(">H", tile & 0x3FF))

    wu32(uc, MAP_PTR_VA, MAPBUF)
    wu32(uc, DESC_BANK_PTR_VA, DESC)
    wu32(uc, MASK_BANK_PTR_VA, MASK)
    wu32(uc, MAP_ROW_STRIDE_VA, 2)
    wu32(uc, MAP_X_EXTENT_VA, 0)
    wu32(uc, MAP_Y_EXTENT_VA, 0)
    w16(uc, GREEN_MODE_VA, 0)

    uc.mem_write(PLAYER, b"\0" * 0x100)
    uc.mem_write(BALL, b"\0" * 0x100)
    wu32(uc, PLAYER + 0x02, BALL)
    w16(uc, PLAYER + 0x28, 5)

    wi32(uc, BALL + 0x00, (x * 2) << 16)
    wi32(uc, BALL + 0x04, (y * 2) << 16)
    wi32(uc, BALL + 0x08, vforce)
    wi32(uc, BALL + 0x0C, hforce)
    wu32(uc, BALL + 0x10, direction & 0xFFF)
    wi32(uc, BALL + 0x14, height)

    w16(uc, SEED0_VA, seed0)
    w16(uc, SEED1_VA, seed1)

    uc.reg_write(UC_X86_REG_ESI, PLAYER)
    uc.reg_write(UC_X86_REG_EDI, BALL)
    uc.reg_write(UC_X86_REG_ESP, STACK + 0xF000)

    state = {
        "descriptor_index": None,
        "landing_code": None,
        "code9_activated": False,
        "events": [],
        "stop": None,
    }

    def hook(machine, address, size, user_data):
        if address == DESCRIPTOR_CAPTURE_VA:
            state["descriptor_index"] = machine.reg_read(UC_X86_REG_EBX) & 0xFFFF
        elif address == LANDING_CAPTURE_VA:
            state["landing_code"] = machine.reg_read(UC_X86_REG_EBX) & 0xFFFF
        elif address == CODE9_BRANCH_VA:
            state["code9_activated"] = True
        elif address == SLOPE_HELPER_VA:
            state["stop"] = ("resume", skip_called_function(machine))
            machine.emu_stop()
        elif address == EVENT_DISPATCH_VA:
            from unicorn.x86_const import UC_X86_REG_EDX
            state["events"].append(machine.reg_read(UC_X86_REG_EDX) & 0xFFFFFFFF)
            state["stop"] = ("resume", skip_called_function(machine))
            machine.emu_stop()
        elif address == STOP_VA:
            state["stop"] = ("done", address)
            machine.emu_stop()

    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        start = START_VA
        for _ in range(16):
            state["stop"] = None
            uc.emu_start(start, 0, count=20000)
            if not state["stop"]:
                eip = uc.reg_read(UC_X86_REG_EIP)
                raise RuntimeError(f"unexpected stop at {eip:#x}")
            kind, value = state["stop"]
            if kind == "resume":
                start = value
                continue
            if kind == "done":
                break
        else:
            raise RuntimeError("too many helper resumptions")
    finally:
        uc.hook_del(token)

    if not state["code9_activated"]:
        raise RuntimeError(
            f"selected course case did not activate code-9 branch: {state}")

    return {
        "oracle": "original-v1.014-course-lookup-to-code9-interaction",
        "build_sha256": digest,
        "input": {
            "descriptor_bank": desc_path.name,
            "selector_bank": mask_path.name,
            "tile": tile,
            "x_subcell": x,
            "y_subcell": y,
            "seed0": seed0 & 0xFFFF,
            "seed1": seed1 & 0xFFFF,
            "direction": direction & 0xFFF,
            "vertical_force": vforce,
            "horizontal_force": hforce,
            "height": height,
        },
        "descriptor_index": state["descriptor_index"],
        "landing_code": state["landing_code"],
        "code9_activated": state["code9_activated"],
        "events": state["events"],
        "direction": ru32(uc, BALL + 0x10) & 0xFFF,
        "vertical_force": ri32(uc, BALL + 0x08),
        "horizontal_force": ri32(uc, BALL + 0x0C),
        "seed0": ru16(uc, SEED0_VA),
        "seed1": ru16(uc, SEED1_VA),
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("descriptor_bank", type=Path)
    ap.add_argument("selector_bank", type=Path)
    ap.add_argument("--tile", type=int, required=True)
    ap.add_argument("--x", type=int, required=True)
    ap.add_argument("--y", type=int, required=True)
    ap.add_argument("--seed0", type=lambda s: int(s, 0), default=0x1234)
    ap.add_argument("--seed1", type=lambda s: int(s, 0), default=0xABCD)
    ap.add_argument("--direction", type=lambda s: int(s, 0), default=777)
    ap.add_argument("--vforce", type=lambda s: int(s, 0), default=-50000)
    ap.add_argument("--hforce", type=lambda s: int(s, 0), default=90000)
    ap.add_argument("--height", type=lambda s: int(s, 0), default=0x40000)
    ap.add_argument("-o", "--output", type=Path)
    a = ap.parse_args()

    result = run(
        a.exe, a.descriptor_bank, a.selector_bank,
        a.tile, a.x, a.y,
        a.seed0, a.seed1, a.direction, a.vforce, a.hforce, a.height,
    )
    text = json.dumps(result, indent=2) + "\n"
    if a.output:
        a.output.parent.mkdir(parents=True, exist_ok=True)
        a.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
