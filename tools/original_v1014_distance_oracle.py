#!/usr/bin/env python3
"""Execute and validate original v1.014 0x40B998 distance-to-hole helper."""

from __future__ import annotations
import argparse, json, math
from pathlib import Path

from unicorn.x86_const import UC_X86_REG_ESI, UC_X86_REG_ESP

from original_v1014_oracle import (
    BALL, PLAYER, SENTINEL, STACK, build_uc, ru32, w16, wu32
)

DISTANCE_VA=0x40B998
HOLE_X_VA=0x41F3B2
HOLE_Y_VA=0x41F3B4
GREEN_MODE_VA=0x41EE7A

def original(exe:Path,x_raw:int,y_raw:int,hx:int,hy:int,green:int)->dict:
    uc,digest=build_uc(exe)
    uc.mem_write(PLAYER,b"\0"*0x100)
    uc.mem_write(BALL,b"\0"*0x100)
    wu32(uc,PLAYER+2,BALL)
    wu32(uc,BALL+0,x_raw & 0xffffffff)
    wu32(uc,BALL+4,y_raw & 0xffffffff)
    w16(uc,HOLE_X_VA,hx)
    w16(uc,HOLE_Y_VA,hy)
    w16(uc,GREEN_MODE_VA,green)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.mem_write(STACK+0xF000, int(SENTINEL).to_bytes(4,"little"))
    uc.reg_write(UC_X86_REG_ESP,STACK+0xF000)
    uc.emu_start(DISTANCE_VA,SENTINEL,count=5000)
    return {"distance":ru32(uc,BALL+0x18),"build_sha256":digest}

def sar32(v:int,bits:int)->int:
    v &= 0xffffffff
    if v & 0x80000000:
        v -= 0x100000000
    return (v >> bits) & 0xffffffff

def candidate(x_raw:int,y_raw:int,hx:int,hy:int,green:int)->int:
    x=x_raw & 0xffffffff
    y=y_raw & 0xffffffff
    if green:
        x=sar32(x,1)
        y=sar32(y,1)
    xi=(x >> 16) & 0xffffffff
    yi=(y >> 16) & 0xffffffff
    dx=(xi-hx) & 0xffffffff
    dy=(yi-hy) & 0xffffffff
    # Scoped course coordinates keep these small; interpret signed deltas.
    if dx & 0x80000000: dx-=0x100000000
    if dy & 0x80000000: dy-=0x100000000
    sq=dx*dx+dy*dy
    root=math.isqrt(sq)
    return (root*6)//10 if root else 0

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()

    rows=[]
    for green in (0,1):
        hx,hy=(123,77)
        # In green mode original coordinate space is doubled before the helper halves it.
        scale=2 if green else 1
        for dx in range(-8,9):
            for dy in range(-8,9):
                xi=(hx+dx)*scale
                yi=(hy+dy)*scale
                for frac in (0,0x4000,0xFFFF):
                    x=((xi & 0xffff)<<16)|frac
                    y=((yi & 0xffff)<<16)|((frac*3)&0xffff)
                    got=original(a.exe,x,y,hx,hy,green)
                    want=candidate(x,y,hx,hy,green)
                    rows.append({
                        "green":green,"dx":dx,"dy":dy,"frac":frac,
                        "original":got["distance"],"candidate":want})
                    assert got["distance"]==want,rows[-1]

    report={
        "reference":"Sensible Golf Windows v1.014",
        "cases":len(rows),
        "zero_tolerance":True,
        "formula":"distance = floor(isqrt((integer_x-hole_x)^2+(integer_y-hole_y)^2)*6/10); green mode arithmetic-halves raw X/Y before integer extraction",
        "zero_examples":[r for r in rows if r["original"]==0][:20],
    }
    text=json.dumps(report,indent=2)+"\n"
    if a.output:
        a.output.parent.mkdir(parents=True,exist_ok=True)
        a.output.write_text(text,encoding="utf-8")
    else:
        print(text,end="")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
