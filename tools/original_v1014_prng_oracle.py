#!/usr/bin/env python3
"""Execute the original Windows v1.014 ranged PRNG routine at 0x406FFA."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from original_v1014_oracle import (
    SENTINEL,
    STACK,
    build_uc,
    ru16,
    w16,
    wu32,
)

PRNG_VA = 0x406FFA
SEED0_VA = 0x41D490
SEED1_VA = 0x41D492


def run(exe: Path, seed0: int, seed1: int, maxima: list[int]) -> dict:
    from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP

    uc, digest = build_uc(exe)
    w16(uc, SEED0_VA, seed0)
    w16(uc, SEED1_VA, seed1)

    samples = []
    for maxv in maxima:
        sp = STACK + 0xF000 - 4
        wu32(uc, sp, SENTINEL)
        uc.reg_write(UC_X86_REG_ESP, sp)
        uc.reg_write(UC_X86_REG_EAX, maxv & 0xFFFF)
        uc.emu_start(PRNG_VA, SENTINEL, count=1000)
        samples.append({
            "max": maxv & 0xFFFF,
            "value": uc.reg_read(UC_X86_REG_EAX) & 0xFFFF,
            "seed0": ru16(uc, SEED0_VA),
            "seed1": ru16(uc, SEED1_VA),
        })

    return {
        "oracle": "original-v1.014-ranged-prng-machine-code",
        "build_sha256": digest,
        "initial_seed0": seed0 & 0xFFFF,
        "initial_seed1": seed1 & 0xFFFF,
        "samples": samples,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", type=Path)
    ap.add_argument("--seed0", type=lambda x: int(x, 0), required=True)
    ap.add_argument("--seed1", type=lambda x: int(x, 0), required=True)
    ap.add_argument("--max", dest="maxima", action="append", type=lambda x: int(x, 0), required=True)
    ap.add_argument("-o", "--output", type=Path)
    a = ap.parse_args()

    result = run(a.exe, a.seed0, a.seed1, a.maxima)
    text = json.dumps(result, indent=2) + "\n"
    if a.output:
        a.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
