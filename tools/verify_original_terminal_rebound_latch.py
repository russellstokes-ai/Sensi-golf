#!/usr/bin/env python3
"""Compare genuine Sensible Golf Windows v1.014 terminal contact latch+rebound.

Original region 0x40A79D..0x40A7E0. The unrelated sound-event
CALL at 0x40A7B7 is replaced with same-length NOPs *inside the Unicorn
emulator only*; the original source file is fingerprint verified and not
changed. Sound callback invocation is still counted. We compare bit 0
of the original player state plus both signed forces and branch choice.
This does NOT prove a tree object, artwork, or live course activation.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import random
import struct
import subprocess
import sys
from itertools import product
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc, REFERENCE_SHA256, PLAYER, BALL, STACK
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_ESP

START=0x40A79D
EVENT_CALL=0x40A7B7
DESTINATIONS={0x40A626:1,0x40A7E0:0}

def original(uc, row):
    flags,vertical,horizontal=row
    uc.mem_write(PLAYER+0x60,struct.pack("<H",flags))
    uc.mem_write(BALL+0x08,struct.pack("<i",vertical))
    uc.mem_write(BALL+0x0C,struct.pack("<i",horizontal))
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    # Original route contains PUSHAD/POPAD around the source sound call.
    uc.reg_write(UC_X86_REG_ESP,STACK+0xF000)
    visited=[];event_calls=[]
    def on_code(machine,address,size,user):
        if address==EVENT_CALL: event_calls.append(address)
        if address in DESTINATIONS:
            visited.append(address)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,on_code)
    try:
        uc.emu_start(START,0,count=65)
    finally:
        uc.hook_del(token)
    if len(visited)!=1 or len(event_calls)>1:
        raise RuntimeError(f"source path escaped expected region: {row}; {visited}, {event_calls}")
    out_v=struct.unpack("<i",bytes(uc.mem_read(BALL+0x08,4)))[0]
    out_h=struct.unpack("<i",bytes(uc.mem_read(BALL+0x0C,4)))[0]
    out_flags=struct.unpack("<H",bytes(uc.mem_read(PLAYER+0x60,2)))[0]
    return (out_v,out_h,DESTINATIONS[visited[0]],out_flags,len(event_calls)),hex(visited[0])

def vectors():
    flags=(0,1,2,3,0x8000,0x8001,0xFFFE,0xFFFF)
    forces=(-2147483648,-2147483647,-8448,-5,-1,0,1,2,5,8448,2147483646,2147483647)
    rows=list(product(flags,forces[:8],forces[:8]))
    rnd=random.Random(20261012)
    for _ in range(3400):
        rows.append((rnd.choice(flags),
                     rnd.randint(-2147483648,2147483647),
                     rnd.randint(-2147483648,2147483647)))
    return rows

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong original Windows executable SHA-256")
    rows=vectors()
    cmd=subprocess.run([str(a.probe)],
        input="".join(f"{f} {v} {h}\n" for f,v,h in rows),
        text=True,capture_output=True,check=True)
    lines=cmd.stdout.splitlines()
    if len(lines)!=len(rows):
        raise RuntimeError("native latch probe returned wrong case count")
    uc,_=build_uc(a.exe)
    # Original v1.014 E8 call is exactly 5 bytes; bypass sound only.
    assert bytes(uc.mem_read(EVENT_CALL,1))==b"\xe8"
    uc.mem_write(EVENT_CALL,b"\x90"*5)
    bad=[];paths={};events={}
    for row,line in zip(rows,lines):
        expected,target=original(uc,row)
        actual=tuple(map(int,line.split()))
        paths[target]=paths.get(target,0)+1
        events[str(expected[-1])]=events.get(str(expected[-1]),0)+1
        if expected!=actual:
            bad.append({"input":row,"original":expected,"native":actual,"target":target})
    evidence={
      "schema":"pixtee.original-terminal-contact-latch-and-rebound-x86.v1",
      "original_sha256":REFERENCE_SHA256,
      "vectors":len(rows),
      "exact":len(rows)-len(bad),
      "routes":paths,
      "event_call_paths":events,
      "source_patch":"Unicorn-memory only: original audio CALL at 0x40A7B7 -> five NOPs; original bytes not redistributed",
      "mismatches":bad[:50],
      "scope":"Authentic original player flag, event-call request, vertical/horizontal force and branch; object activation not proven"
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(evidence,indent=2)+"\n")
    print(f"Original terminal latch and rebound: {evidence['exact']}/{len(rows)} exact")
    print("Original x86 destinations:",paths,"event call paths:",events)
    for mismatch in bad[:10]:print("MISMATCH",mismatch)
    if bad:raise SystemExit("original contact latch/rebound mismatch")
    print("PASS authentic 0x40A79D event latch and rebound, no tree-object claim")

if __name__=="__main__":
    main()
