#!/usr/bin/env python3
"""Execute original Sensible Golf v1.014 player launch/clear-air machine code."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

from pe_target_xrefs import parse_pe

REFERENCE_SHA256 = "3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"

IMAGE_MIN = 0x400000
PLAYER = 0x500000
BALL = 0x501000
STACK = 0x600000
SENTINEL = 0x700000

CLUB_TABLE_VA = 0x41F108
DROP_POWER_VA = 0x41D59E
SWING_ADJ_VA = 0x41F32C
GREEN_MODE_VA = 0x41EE7A
SPECIAL_MODE_VA = 0x41EF66
ADJ_X_VA = 0x41F0C5
ADJ_Y_VA = 0x41F0C9

LAUNCH_VA = 0x40C84F
PROFILE_HELPER_VA = 0x40A22B
AIR_TICK_VA = 0x40A603
AIR_TICK_END_VA = 0x40AA1C


def align_up(value: int, alignment: int = 0x1000) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


def write_u16(uc, address: int, value: int) -> None:
    uc.mem_write(address, struct.pack("<H", value & 0xFFFF))


def write_i32(uc, address: int, value: int) -> None:
    uc.mem_write(address, struct.pack("<i", int(value)))


def write_u32(uc, address: int, value: int) -> None:
    uc.mem_write(address, struct.pack("<I", value & 0xFFFFFFFF))


def read_i32(uc, address: int) -> int:
    return struct.unpack("<i", bytes(uc.mem_read(address, 4)))[0]


def read_u32(uc, address: int) -> int:
    return struct.unpack("<I", bytes(uc.mem_read(address, 4)))[0]


def build_uc(exe: Path):
    try:
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_PROT_ALL
    except ImportError as exc:
        raise RuntimeError("unicorn package is required") from exc

    data = exe.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != REFERENCE_SHA256:
        raise ValueError(f"wrong GOLFWIN.EXE build: {digest}")

    image_base, _, sections = parse_pe(data)
    if image_base != IMAGE_MIN:
        raise ValueError(f"unexpected image base {image_base:#x}")

    max_end = image_base + 0x1000
    for sec in sections:
        max_end = max(max_end, image_base + sec["rva"] + max(sec["virtual_size"], sec["raw_size"]))
    image_size = align_up(max_end - image_base)

    uc = Uc(UC_ARCH_X86, UC_MODE_32)
    uc.mem_map(image_base, image_size, UC_PROT_ALL)

    first_raw = min((s["raw_offset"] for s in sections if s["raw_offset"]), default=0x1000)
    uc.mem_write(image_base, data[:first_raw])
    for sec in sections:
        if sec["raw_size"]:
            blob = data[sec["raw_offset"]: sec["raw_offset"] + sec["raw_size"]]
            uc.mem_write(image_base + sec["rva"], blob)

    uc.mem_map(PLAYER, 0x10000, UC_PROT_ALL)
    uc.mem_map(STACK, 0x10000, UC_PROT_ALL)
    uc.mem_map(SENTINEL, 0x1000, UC_PROT_ALL)

    # Profile selection is outside this oracle. Supply its recovered output
    # (swing_adjuster) directly and make the helper side-effect-free.
    uc.mem_write(PROFILE_HELPER_VA, b"\xC3")
    return uc, digest


def club_values_from_original(uc, club_index: int) -> tuple[int, int, int]:
    if not 0 <= club_index < 13:
        raise ValueError("club must be 0..12")
    raw = bytes(uc.mem_read(CLUB_TABLE_VA + club_index * 12, 12))
    raw_v, raw_h, scale = struct.unpack("<iii", raw)
    return raw_v // 2, raw_h // 2, scale


def setup_launch(uc, club: int, power: int, direction: int, swing_adjuster: int) -> None:
    from unicorn.x86_const import UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_ESP

    if not 0 <= power <= 105:
        raise ValueError("power must be 0..105")

    vbase, hbase, scale = club_values_from_original(uc, club)
    if club == 12:
        raise ValueError("putter uses a separate launch path")

    write_u32(uc, PLAYER + 0x02, BALL)
    write_u32(uc, PLAYER + 0x12, direction & 0x0FFF)
    write_i32(uc, PLAYER + 0x2A, vbase)
    write_i32(uc, PLAYER + 0x2E, hbase)
    write_i32(uc, PLAYER + 0x32, scale)

    uc.mem_write(BALL, b"\0" * 0x2C)
    write_u16(uc, DROP_POWER_VA, power)
    write_i32(uc, SWING_ADJ_VA, swing_adjuster)
    write_u16(uc, GREEN_MODE_VA, 0)
    write_u16(uc, SPECIAL_MODE_VA, 0)
    write_i32(uc, ADJ_X_VA, 0)
    write_i32(uc, ADJ_Y_VA, 0)

    sp = STACK + 0xF000 - 4
    write_u32(uc, sp, SENTINEL)
    uc.reg_write(UC_X86_REG_ESP, sp)
    uc.reg_write(UC_X86_REG_ESI, PLAYER)
    uc.reg_write(UC_X86_REG_EDI, 0)


def sample(uc, tick: int) -> dict[str, int]:
    return {
        "tick": tick,
        "x": read_i32(uc, BALL + 0x00),
        "y": read_i32(uc, BALL + 0x04),
        "height": read_i32(uc, BALL + 0x14),
        "vertical_force": read_i32(uc, BALL + 0x08),
        "horizontal_force": read_i32(uc, BALL + 0x0C),
        "direction": read_u32(uc, BALL + 0x10) & 0x0FFF,
    }


def run_oracle(exe: Path, club: int, power: int, direction: int, swing_adjuster: int, ticks: int) -> dict:
    from unicorn.x86_const import UC_X86_REG_ESI, UC_X86_REG_EDI

    uc, digest = build_uc(exe)
    setup_launch(uc, club, power, direction, swing_adjuster)

    uc.emu_start(LAUNCH_VA, SENTINEL, count=2000)
    samples = [sample(uc, 0)]

    for tick in range(1, ticks + 1):
        current = sample(uc, tick - 1)
        if current["height"] + current["vertical_force"] - 0x2100 <= 0:
            raise RuntimeError("requested oracle length reaches landing branch")

        write_i32(uc, SWING_ADJ_VA, swing_adjuster)
        write_u16(uc, GREEN_MODE_VA, 0)
        write_u16(uc, SPECIAL_MODE_VA, 0)
        write_i32(uc, ADJ_X_VA, 0)
        write_i32(uc, ADJ_Y_VA, 0)
        uc.reg_write(UC_X86_REG_ESI, PLAYER)
        uc.reg_write(UC_X86_REG_EDI, BALL)
        uc.emu_start(AIR_TICK_VA, AIR_TICK_END_VA, count=5000)
        samples.append(sample(uc, tick))

    return {
        "oracle": "original-v1.014-machine-code",
        "build_sha256": digest,
        "scope": "live-player-launch-plus-normal-clear-air",
        "input": {
            "club": club,
            "power": power,
            "direction": direction & 0x0FFF,
            "swing_adjuster": swing_adjuster,
            "ticks": ticks,
        },
        "samples": samples,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("--club", type=int, required=True)
    ap.add_argument("--power", type=int, required=True)
    ap.add_argument("--direction", type=int, required=True)
    ap.add_argument("--swing-adjuster", type=int, required=True)
    ap.add_argument("--ticks", type=int, required=True)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()

    report = run_oracle(
        args.exe, args.club, args.power, args.direction,
        args.swing_adjuster, args.ticks,
    )
    out = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(out, encoding="utf-8")
    else:
        print(out, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
