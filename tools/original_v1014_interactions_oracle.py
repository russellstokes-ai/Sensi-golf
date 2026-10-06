#!/usr/bin/env python3
"""Execute three PRNG-driven v1.014 whole-tick interaction fragments."""

from __future__ import annotations
import argparse,json
from pathlib import Path

from original_v1014_oracle import BALL,SENTINEL,STACK,build_uc,ri32,ru16,ru32,w16,wi32,wu32
from original_v1014_prng_oracle import SEED0_VA,SEED1_VA

BRANCHES = {
    "code9": (0x40A3AC, 0x40A3CB),
    "lip":   (0x40A47A, 0x40A4A6),
    "flag":  (0x40A53A, 0x40A562),
}

def run(exe:Path, branch:str, seed0:int, seed1:int, direction:int, vforce:int, hforce:int)->dict:
    from unicorn.x86_const import UC_X86_REG_EDI,UC_X86_REG_ESP

    uc,digest=build_uc(exe)
    uc.mem_write(BALL,b"\0"*0x100)
    wu32(uc,BALL+0x10,direction & 0xFFF)
    wi32(uc,BALL+0x08,vforce)
    wi32(uc,BALL+0x0C,hforce)
    w16(uc,SEED0_VA,seed0)
    w16(uc,SEED1_VA,seed1)
    uc.mem_write(0x41F886,b"\0")

    sp=STACK+0xF000-4
    wu32(uc,sp,SENTINEL)
    uc.reg_write(UC_X86_REG_ESP,sp)
    uc.reg_write(UC_X86_REG_EDI,BALL)

    start,end=BRANCHES[branch]
    uc.emu_start(start,end,count=1000)

    return {
        "branch":branch,
        "build_sha256":digest,
        "direction":ru32(uc,BALL+0x10)&0xFFF,
        "vertical_force":ri32(uc,BALL+0x08),
        "horizontal_force":ri32(uc,BALL+0x0C),
        "seed0":ru16(uc,SEED0_VA),
        "seed1":ru16(uc,SEED1_VA),
        "marker":bool(bytes(uc.mem_read(0x41F886,1))[0]),
    }

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--branch",choices=sorted(BRANCHES),required=True)
    ap.add_argument("--seed0",type=lambda s:int(s,0),required=True)
    ap.add_argument("--seed1",type=lambda s:int(s,0),required=True)
    ap.add_argument("--direction",type=lambda s:int(s,0),required=True)
    ap.add_argument("--vforce",type=lambda s:int(s,0),required=True)
    ap.add_argument("--hforce",type=lambda s:int(s,0),required=True)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()
    r=run(a.exe,a.branch,a.seed0,a.seed1,a.direction,a.vforce,a.hforce)
    text=json.dumps(r,indent=2)+"\n"
    if a.output:a.output.write_text(text,encoding="utf-8")
    else:print(text,end="")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
