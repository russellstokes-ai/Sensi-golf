#!/usr/bin/env python3
"""Execute the original v1.014 post-hazard position recovery branch."""

from __future__ import annotations
import argparse,json,struct
from pathlib import Path

from original_v1014_oracle import (
    BALL,PLAYER,SENTINEL,STACK,build_uc,ri32,ru16,w16,wi32,wu32
)

START_VA=0x40A7E0
STOP_VA=0x40A8D6
TERRAIN_LOOKUP_VA=0x409535
HELPERS={0x4094F0,0x40C2F8,0x40B791,0x40B998}
GREEN_MODE_VA=0x41EE7A
X_EXTENT_VA=0x41EE1A
Y_EXTENT_VA=0x41EE1E

def skip_call(uc)->int:
    from unicorn.x86_const import UC_X86_REG_ESP
    sp=uc.reg_read(UC_X86_REG_ESP)
    ret=struct.unpack("<I",bytes(uc.mem_read(sp,4)))[0]
    uc.reg_write(UC_X86_REG_ESP,sp+4)
    return ret

def run(
    exe:Path,
    ball_x:int,
    ball_y:int,
    safe_x:int,
    safe_y:int,
    x_extent:int,
    y_extent:int,
    green_mode:int,
)->dict:
    from unicorn import UC_HOOK_CODE
    from unicorn.x86_const import (
        UC_X86_REG_EBX,UC_X86_REG_EDI,UC_X86_REG_EIP,
        UC_X86_REG_ESI,UC_X86_REG_ESP,
    )

    uc,digest=build_uc(exe)
    uc.mem_write(PLAYER,b"\0"*0x100)
    uc.mem_write(BALL,b"\0"*0x100)

    wi32(uc,BALL+0x00,ball_x)
    wi32(uc,BALL+0x04,ball_y)
    wi32(uc,BALL+0x14,123456)
    w16(uc,BALL+0x1C,0)
    wu32(uc,PLAYER+0x64,safe_x & 0xFFFFFFFF)
    wu32(uc,PLAYER+0x68,safe_y & 0xFFFFFFFF)
    w16(uc,PLAYER,0x8000)

    w16(uc,GREEN_MODE_VA,green_mode)
    wu32(uc,X_EXTENT_VA,x_extent & 0xFFFFFFFF)
    wu32(uc,Y_EXTENT_VA,y_extent & 0xFFFFFFFF)

    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xF000)

    def hook(machine,address,size,user_data):
        if address==TERRAIN_LOOKUP_VA:
            ret=skip_call(machine)
            ebx=machine.reg_read(UC_X86_REG_EBX)
            machine.reg_write(UC_X86_REG_EBX,(ebx&0xFFFF0000)|6)
            machine.reg_write(UC_X86_REG_EIP,ret)
        elif address in HELPERS:
            machine.reg_write(UC_X86_REG_EIP,skip_call(machine))
        elif address==STOP_VA:
            machine.emu_stop()

    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(START_VA,SENTINEL,count=20000)
    finally:
        uc.hook_del(token)

    eip=uc.reg_read(UC_X86_REG_EIP)
    if eip!=STOP_VA:
        raise RuntimeError(f"recovery stopped at {eip:#x}")

    return {
        "oracle":"original-v1.014-hazard-recovery",
        "build_sha256":digest,
        "ball_x":ri32(uc,BALL+0x00),
        "ball_y":ri32(uc,BALL+0x04),
        "player_x":ri32(uc,PLAYER+0x06),
        "player_y":ri32(uc,PLAYER+0x0A),
        "player_last_x":ri32(uc,PLAYER+0x4A),
        "player_last_y":ri32(uc,PLAYER+0x4E),
        "height":ri32(uc,BALL+0x14),
        "pause":ru16(uc,BALL+0x1C),
        "player_flags":ru16(uc,PLAYER),
    }

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    for name in ("ball-x","ball-y","safe-x","safe-y","x-extent","y-extent","green-mode"):
        ap.add_argument(f"--{name}",type=lambda s:int(s,0),required=True)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()
    r=run(
        a.exe,a.ball_x,a.ball_y,a.safe_x,a.safe_y,
        a.x_extent,a.y_extent,a.green_mode)
    text=json.dumps(r,indent=2)+"\n"
    if a.output:
        a.output.parent.mkdir(parents=True,exist_ok=True)
        a.output.write_text(text,encoding="utf-8")
    else:
        print(text,end="")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
