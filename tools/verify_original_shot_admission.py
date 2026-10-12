#!/usr/bin/env python3
"""Original Windows v1.014 0x40A2B0 player shot-tick admission oracle.

Executes the original x86 source up to entry into normal terrain, the
terminal/finished branch, idle branch or CPU-turn delegate. The CALL to
the original CPU planner (0x40A2E8) is stubbed in Unicorn memory only;
its invocation and the source player flag/mode writes are compared.
This is NOT an x86 simulation of the upstream 0x40A250 helpers or CPU AI.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from itertools import product
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    BALL,PLAYER,STACK,build_uc,wu32,w16,REFERENCE_SHA256
)
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_ESP

START=0x40A2B0
CPU_CALL=0x40A2E8
TERMINAL=0x40A7E0
BIT2_BRANCH=0x40AACF
IDLE=0x40AA78
ACTIVE=0x40A2F7

def vectors():
    flags=(0,1,2,3,4,5,6,7,0x8000,0x8001,0x8002,0x8004,
           0x7fff,0xfffe,0xffff)
    modes=(0,1,2,3,4,5,6,0x7fff,0x8000,0xfffe,0xffff)
    rows=list(product(flags,modes[:8],modes[:8]))
    rng=random.Random(0x40a2b0)
    for _ in range(6200):
        rows.append((rng.choice(flags+(rng.randrange(65536),)),
                    rng.choice(modes+(rng.randrange(65536),)),
                    rng.choice(modes+(rng.randrange(65536),))))
    return rows

def original(uc,row):
    flags,mode5a,mode5c=row
    uc.mem_write(PLAYER,b"\x00"*0x100)
    wu32(uc,PLAYER+2,BALL)
    w16(uc,PLAYER,flags)
    w16(uc,PLAYER+0x5a,mode5a)
    w16(uc,PLAYER+0x5c,mode5c)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    stops=[];cpu=[]
    def hook(machine,address,size,user):
        if address==CPU_CALL:cpu.append(address)
        if address in (TERMINAL,BIT2_BRANCH,IDLE,ACTIVE):
            stops.append(address);machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(START,0,count=50)
    finally:
        uc.hook_del(token)
    if len(stops)!=1 or len(cpu)>1:
        raise RuntimeError(f"source 0x40A2B0 escaped expected branches: {row}, {stops}, {cpu}")
    dest=stops[0]
    if dest==TERMINAL:route=0
    elif dest==BIT2_BRANCH:route=1
    elif dest==ACTIVE:route=3
    elif dest==IDLE and cpu:route=4
    elif dest==IDLE:route=2
    else:raise RuntimeError(f"unexpected source branch {hex(dest)}")
    def word(off):
        return struct.unpack("<H",bytes(uc.mem_read(PLAYER+off,2)))[0]
    return (route,word(0),word(0x5a),word(0x5c),len(cpu)),hex(dest)

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("not source original Windows 1.014 executable")
    rows=vectors()
    native=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,row))+"\n" for row in rows),
        text=True,capture_output=True,check=True)
    lines=native.stdout.splitlines()
    if len(lines)!=len(rows):
        raise RuntimeError("native pre-shot admission row count mismatch")
    uc,_=build_uc(a.exe)
    original_call=bytes(uc.mem_read(CPU_CALL,5))
    if original_call[0]!=0xe8:
        raise RuntimeError("unexpected original CPU delegate CALL encoding")
    uc.mem_write(CPU_CALL,b"\x90"*5)
    mismatch=[];destinations={};routes={};cpu_requests=0
    for row,line in zip(rows,lines):
        expected,dest=original(uc,row)
        got=tuple(map(int,line.split()))
        routes[str(expected[0])]=routes.get(str(expected[0]),0)+1
        destinations[dest]=destinations.get(dest,0)+1
        cpu_requests+=expected[-1]
        if expected!=got:
            mismatch.append({"input":row,"original":expected,
                             "native":got,"x86_target":dest})
    evidence={
      "schema":"pixtee.original-shot-tick-entry-player-status-x86.v1",
      "original_sha256":REFERENCE_SHA256,
      "vectors":len(rows),"exact":len(rows)-len(mismatch),
      "original_gate_routes":routes,
      "x86_exit_targets":destinations,
      "original_cpu_planner_call_attempts":cpu_requests,
      "original_cpu_call_stubbed_in_emulator_only":hex(CPU_CALL),
      "mismatches":mismatch[:50],
      "scope":"Authentic 0x40A2B0 to first player-state branch and recorded CPU callback. NOT upstream tick, completed multiplayer cycle, full CPU strategy or actual course-object activation."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(evidence,indent=2)+"\n")
    print(f"Original player-state shot entry: {evidence['exact']}/{len(rows)} exact")
    print("Original source outcomes:",routes,"CPU delegate requests:",cpu_requests)
    for miss in mismatch[:8]:print("MISMATCH",miss)
    if mismatch:raise SystemExit("source shot-admission state mismatch")
    print("PASS authentic original upstream player-state gate; source CPU callback not reproduced")

if __name__=="__main__":main()
