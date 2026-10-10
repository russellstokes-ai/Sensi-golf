#!/usr/bin/env python3
"""Exhaustively execute original v1.014 MAPI sub-cell lookup machine code."""

from __future__ import annotations

import argparse
import csv
import struct
from pathlib import Path

from original_v1014_oracle import build_uc, wu32, w16

LOOKUP_VA=0x409535
LOOKUP_RETURN_VA=0x40967C
MAPBUF=0x510000
DESC=0x520000
MASK=0x530000

MAP_PTR_VA=0x41EE22
MAP_ROW_STRIDE_VA=0x41EE06
MAP_X_EXTENT_VA=0x41EE1A
MAP_Y_EXTENT_VA=0x41EE1E
GREEN_MODE_VA=0x41EE7A
DESC_BANK_PTR_VA=0x41EE72
MASK_BANK_PTR_VA=0x41EE76

def run(exe:Path,desc_path:Path,mask_path:Path,out:Path)->int:
    from unicorn import UC_PROT_ALL
    from unicorn.x86_const import (
        UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,
        UC_X86_REG_EDX,UC_X86_REG_EBP,
    )

    desc=desc_path.read_bytes()
    mask=mask_path.read_bytes()
    if len(desc)!=len(mask) or len(desc)%8:
        raise ValueError("MAPI banks must have equal eight-byte-aligned sizes")

    uc,digest=build_uc(exe)
    uc.mem_map(MAPBUF,0x30000,UC_PROT_ALL)
    uc.mem_write(DESC,desc)
    uc.mem_write(MASK,mask)

    wu32(uc,MAP_PTR_VA,MAPBUF)
    wu32(uc,DESC_BANK_PTR_VA,DESC)
    wu32(uc,MASK_BANK_PTR_VA,MASK)
    wu32(uc,MAP_ROW_STRIDE_VA,2)
    wu32(uc,MAP_X_EXTENT_VA,0)
    wu32(uc,MAP_Y_EXTENT_VA,0)
    w16(uc,GREEN_MODE_VA,0)

    tile_count=len(desc)//8
    out.parent.mkdir(parents=True,exist_ok=True)
    with out.open("w",newline="",encoding="utf-8") as fh:
        writer=csv.writer(fh,lineterminator="\n")
        writer.writerow(["tile","x","y","descriptor","direction","magnitude"])
        for tile in range(tile_count):
            # The synthetic one-cell MAPM uses one big-endian tile word at +0x60.
            uc.mem_write(MAPBUF+0x60,struct.pack(">H",tile & 0x3FF))
            for y in range(4):
                for x in range(8):
                    uc.reg_write(UC_X86_REG_EAX,x*2)
                    uc.reg_write(UC_X86_REG_EBX,y*2)
                    uc.reg_write(UC_X86_REG_ECX,0)
                    uc.reg_write(UC_X86_REG_EDX,0)
                    uc.reg_write(UC_X86_REG_EBP,0)
                    uc.emu_start(LOOKUP_VA,LOOKUP_RETURN_VA,count=1000)
                    bx=uc.reg_read(UC_X86_REG_EBX)&0xFFFF
                    cx=uc.reg_read(UC_X86_REG_ECX)&0xFFFF
                    dx=uc.reg_read(UC_X86_REG_EDX)&0xFFFF
                    writer.writerow([tile,x,y,bx,cx,dx])

    print(f"build_sha256={digest}")
    print(f"cases={tile_count*32}")
    return tile_count*32

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("descriptor_bank",type=Path)
    ap.add_argument("selector_bank",type=Path)
    ap.add_argument("-o","--output",type=Path,required=True)
    a=ap.parse_args()
    run(a.exe,a.descriptor_bank,a.selector_bank,a.output)
    return 0

if __name__=="__main__":
    raise SystemExit(main())
