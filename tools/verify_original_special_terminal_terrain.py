#!/usr/bin/env python3
"""Original Windows 1.014 x86 special numeric terrain 35 / 50 branch vs C++.

Runs authentic machine code 0x40A686..0x40A7E0. It replaces only the
unrelated sprite and audio CALLs in emulator memory with same-width NOPs,
records their original call sites and event EDX code, and checks original
player flags, height, both forces, and continuation branch against C++.
NOT complete host event/audio semantics, tree geometry, or actual course hit.
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
from unicorn.x86_const import UC_X86_REG_EDI,UC_X86_REG_ESI,UC_X86_REG_EBX,UC_X86_REG_ESP

START=0x40A686
EXIT={0x40A626:1,0x40A7E0:0}
SPRITE_CALLS=(0x40A6C0,0x40A6F8)
SOUND_CALLS={0x40A6D1:1,0x40A720:11}
PATCH_CALLS=SPRITE_CALLS+tuple(SOUND_CALLS)

def signed_high_word(i):
    x=(i>>16)&0xffff
    return x if x<0x8000 else x-0x10000

def original(uc,row):
    code,club,flags,vertical,horizontal,height=row
    uc.mem_write(PLAYER,struct.pack("<H",flags))
    uc.mem_write(PLAYER+0x28,struct.pack("<H",club))
    uc.mem_write(BALL+0x08,struct.pack("<i",vertical))
    uc.mem_write(BALL+0x0C,struct.pack("<i",horizontal))
    uc.mem_write(BALL+0x14,struct.pack("<i",height))
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_EBX,code)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    ends=[];sprites=[];sound=[]
    def hook(machine,address,size,user):
        if address in SPRITE_CALLS: sprites.append(address)
        if address in SOUND_CALLS: sound.append(SOUND_CALLS[address])
        if address in EXIT:
            ends.append(address)
            machine.emu_stop()
    handle=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(START,0,count=120)
    finally:
        uc.hook_del(handle)
    if len(ends)!=1 or len(sprites)!=1 or len(sound)!=1:
        raise RuntimeError(f"original source path escaped scoped call/exit: {row}, {ends}, {sprites}, {sound}")
    out_v=struct.unpack("<i",bytes(uc.mem_read(BALL+8,4)))[0]
    out_h=struct.unpack("<i",bytes(uc.mem_read(BALL+12,4)))[0]
    out_height=struct.unpack("<i",bytes(uc.mem_read(BALL+20,4)))[0]
    out_flags=struct.unpack("<H",bytes(uc.mem_read(PLAYER,2)))[0]
    return (1,out_v,out_h,out_height,out_flags,EXIT[ends[0]],sound[0]),hex(ends[0])

def vectors():
    heights=[
        -2147483648,-2147418113,-262144,-65536,-1,0,1,0xffff,
        0x10000,199<<16,200<<16,(200<<16)|0xffff,
        201<<16,202<<16,2147483647
    ]
    forces=[-2147483648,-65536,-8448,-5,-1,0,1,5,8448,65536,2147483647]
    flags=[0,1,2,3,0x8000,0x8001,0xffff]
    clubs=[0,3,11,12]
    # 2,000+ boundary cases across course flags, signed heights and both clubs.
    rows=[]
    for code,club,flags,height,vertical,horizontal in product(
         (35,50),clubs,flags,heights,(-8448,0,8448),(0,65536)):
        if code==35 and signed_high_word(height)>200: continue
        rows.append((code,club,flags,vertical,horizontal,height))
    rng=random.Random(10143550)
    for _ in range(3800):
        code=rng.choice((35,50))
        club=rng.choice((0,1,11,12,12,13))
        flags=rng.choice(flags)
        height=rng.choice(heights+[rng.randint(-2147483648,2147483647)])
        if code==35 and signed_high_word(height)>200:
            height=(200<<16)|(height&0xffff)
        vertical=rng.choice(forces+[rng.randint(-2147483648,2147483647)])
        horizontal=rng.choice(forces+[rng.randint(-2147483648,2147483647)])
        rows.append((code,club,flags,vertical,horizontal,height))
    return rows

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("unverified original Windows v1.014 exe")
    rows=vectors()
    cmd=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,row))+"\n" for row in rows),
        text=True,capture_output=True,check=True)
    lines=cmd.stdout.splitlines()
    if len(lines)!=len(rows):raise RuntimeError("wrong native oracle probe count")
    uc,_=build_uc(a.exe)
    for address in PATCH_CALLS:
        b=bytes(uc.mem_read(address,5))
        if b[0]!=0xe8:
            raise RuntimeError(f"unexpected original CALL instruction {address:#x}: {b.hex()}")
        uc.mem_write(address,b"\x90"*5)
    mismatch=[];routes={};events={};codes={}
    for row,line in zip(rows,lines):
        expected,target=original(uc,row)
        actual=tuple(map(int,line.split()))
        routes[target]=routes.get(target,0)+1
        events[str(expected[-1])]=events.get(str(expected[-1]),0)+1
        codes[str(row[0])]=codes.get(str(row[0]),0)+1
        if actual!=expected:
            mismatch.append({"input":row,"expected":expected,
                             "native":actual,"destination":target})
    evidence={
      "schema":"pixtee.original-special-terrain-35-50-terminal-x86.v1",
      "source_sha256":REFERENCE_SHA256,
      "total":len(rows),"exact":len(rows)-len(mismatch),
      "original_routes":routes,"original_event_call_arguments":events,
      "terrain_code_counts":codes,
      "patched_in_emulator_only":[hex(x) for x in PATCH_CALLS],
      "mismatches":mismatch[:40],
      "scope":"X86 0x40A686..0x40A7E0 after numeric terrain code lookup; no source course-object mapping, sprite/audio side effects or gameplay hook."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(evidence,indent=2)+"\n")
    print(f"Original terrain 35/50 terminal behaviour: {evidence['exact']}/{len(rows)} exact")
    print("Original exits:",routes,"event arguments:",events,"terrain vectors:",codes)
    for bad in mismatch[:6]:print("MISMATCH",bad)
    if mismatch:raise SystemExit("source terrain 35/50 terminal branch mismatch")
    print("PASS authentic original code-35/50 state and route; not whole collision parity")

if __name__=="__main__":
    main()
