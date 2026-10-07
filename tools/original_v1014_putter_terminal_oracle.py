#!/usr/bin/env python3
"""Execute original v1.014 club-12 code-8/9/10 terminal branches."""

from __future__ import annotations
import argparse,json,struct
from pathlib import Path

from original_v1014_oracle import (
    BALL,PLAYER,SENTINEL,STACK,build_uc,ru16,w16,wu32
)

START_VA=0x40A5A2
VISUAL_HELPER_VA=0x40C4C0
COUNTER_HELPER_VA=0x40A22B
EVENT_DISPATCH_VA=0x403EF2
CODE9_10_TRANSITION_VA=0x40CD00
CODE8_TRANSITION_VA=0x40CD25
TERMINAL_FLAG_VA=0x41F883
UI_GATE_VA=0x41D64B
GAME_MODE_VA=0x42558E

def skip_call(uc)->int:
    from unicorn.x86_const import UC_X86_REG_ESP
    sp=uc.reg_read(UC_X86_REG_ESP)
    ret=struct.unpack("<I",bytes(uc.mem_read(sp,4)))[0]
    uc.reg_write(UC_X86_REG_ESP,sp+4)
    return ret

def run(exe:Path, terrain_index:int)->dict:
    from unicorn import UC_HOOK_CODE
    from unicorn.x86_const import (
        UC_X86_REG_EBX,UC_X86_REG_EDI,UC_X86_REG_EDX,
        UC_X86_REG_EIP,UC_X86_REG_ESI,UC_X86_REG_ESP,
    )

    uc,digest=build_uc(exe)
    uc.mem_write(PLAYER,b"\0"*0x100)
    uc.mem_write(BALL,b"\0"*0x100)

    w16(uc,PLAYER+0x28,12)
    w16(uc,PLAYER+0x52,7)
    w16(uc,PLAYER+0x56,11)
    w16(uc,PLAYER+0x5A,0)
    w16(uc,BALL+0x1C,0)
    w16(uc,TERMINAL_FLAG_VA,0)
    uc.mem_write(UI_GATE_VA,b"\0")
    uc.mem_write(GAME_MODE_VA,b"\0")

    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_EBX,terrain_index & 0xFFFF)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xF000)

    events=[]
    transition=None

    def hook(machine,address,size,user_data):
        nonlocal transition
        if address==VISUAL_HELPER_VA:
            machine.reg_write(UC_X86_REG_EIP,skip_call(machine))
        elif address==EVENT_DISPATCH_VA:
            events.append(machine.reg_read(UC_X86_REG_EDX)&0xFFFFFFFF)
            machine.reg_write(UC_X86_REG_EIP,skip_call(machine))
        elif address==CODE9_10_TRANSITION_VA:
            transition="special"
            machine.emu_stop()
        elif address==CODE8_TRANSITION_VA:
            transition="hole"
            machine.emu_stop()

    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(START_VA,SENTINEL,count=5000)
    finally:
        uc.hook_del(token)

    if transition is None:
        raise RuntimeError(
            f"terrain {terrain_index} did not reach scoped terminal; "
            f"eip={uc.reg_read(UC_X86_REG_EIP):#x}")

    return {
        "oracle":"original-v1.014-putter-terminal",
        "build_sha256":digest,
        "terrain_index":terrain_index,
        "transition":transition,
        "player_flags":ru16(uc,PLAYER),
        "counter_52":ru16(uc,PLAYER+0x52),
        "counter_56":ru16(uc,PLAYER+0x56),
        "pause":ru16(uc,BALL+0x1C),
        "terminal_flag":ru16(uc,TERMINAL_FLAG_VA),
        "ui_gate":bytes(uc.mem_read(UI_GATE_VA,1))[0],
        "events":events,
    }

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--terrain-index",type=int,required=True)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()
    r=run(a.exe,a.terrain_index)
    text=json.dumps(r,indent=2)+"\n"
    if a.output:
        a.output.parent.mkdir(parents=True,exist_ok=True)
        a.output.write_text(text,encoding="utf-8")
    else:
        print(text,end="")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
