#!/usr/bin/env python3
"""Source-only dynamic probe of negative original animation-script words.

Uses SHA-authenticated Windows v1.014 x86 under Unicorn. This records
observed control paths, never asserts unverified opcode semantics, and
never stores original executable/art/sound bytes in the repository.
The partial source dispatch window is intentionally NOT a full event
interpreter; outside-window CALLs are stopped before host side effects.
"""
from __future__ import annotations
import argparse,hashlib,json,struct
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_ESP
from original_v1014_oracle import (
    build_uc, REFERENCE_SHA256, PLAYER, BALL, STACK, w16,wu32,ru16,ru32
)
ENTRY=0x40C539
WINDOW_END=0x40C5F0
SLOT=PLAYER
ASSET=BALL
OPCODES=(0x8000,0x8001,0x8002,0x8FFF,0xC000,0xE000,
         0xFFF0,0xFFF8,0xFFF9,0xFFFA,0xFFFB,0xFFFC,
         0xFFFD,0xFFFE,0xFFFF)

def original_route(exe:Path,word:int):
    uc,_=build_uc(exe)
    uc.mem_write(SLOT,bytes(0x80))
    script=bytearray(64)
    struct.pack_into("<HHHHH",script,0,7,0,0,word,0x0012)
    # original signed opcode is read from original asset+6+slot offset
    uc.mem_write(ASSET,bytes(script))
    w16(uc,SLOT,0)
    w16(uc,SLOT+6,0x1234)
    w16(uc,SLOT+8,0x5678)
    w16(uc,SLOT+0xA,0)
    wu32(uc,SLOT+0xC,ASSET)
    wu32(uc,SLOT+0x10,0)
    uc.reg_write(UC_X86_REG_ESI,SLOT)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xF000-4)
    seen=[]
    exit_address=None
    stop_reason=None
    def cb(m,address,size,_):
        nonlocal exit_address,stop_reason
        if address<ENTRY or address>=WINDOW_END:
            exit_address=address
            stop_reason="left_source_command_window_before_host_call"
            m.emu_stop()
            return
        seen.append(address)
        if len(seen)>=300:
            stop_reason="instruction_budget"
            m.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,cb)
    try:
        uc.emu_start(ENTRY,0,count=400)
    except Exception as exc:
        stop_reason="unmodeled_source_environment:"+type(exc).__name__
    finally:
        uc.hook_del(token)
    return {
        "signed_word_hex":hex(word),
        "exit_address":hex(exit_address) if exit_address is not None else None,
        "stop_reason":stop_reason or "no_captured_exit",
        "instruction_count":len(seen),
        "trace_addresses":[hex(x) for x in seen[:110]],
        "slot_status":ru16(uc,SLOT),
        "slot_height_or_frame":ru16(uc,SLOT+6),
        "slot_width":ru16(uc,SLOT+8),
        "slot_delay":ru16(uc,SLOT+0xA),
        "slot_next_offset":ru32(uc,SLOT+0x10)
    }

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Unrecognized original game executable")
    routes=[original_route(a.exe,x) for x in OPCODES]
    report={
        "schema":"pixtee.original-negative-animation-dynamic-routes-research.v1",
        "original_sha256":REFERENCE_SHA256,
        "source_entry":hex(ENTRY),
        "research_cases":len(routes),
        "samples":routes,
        "interpretation":"Observed source-only first dispatch. No candidate negative opcode is considered implemented; host calls and original visual/audio side effects are not simulated."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    for r in routes:
        print(r["signed_word_hex"],r["exit_address"],
              r["stop_reason"],"source_steps",r["instruction_count"])
    print("Original command route signatures captured; no gameplay parity claim")
if __name__=="__main__":main()
