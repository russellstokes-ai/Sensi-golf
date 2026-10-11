#!/usr/bin/env python3
"""Original v1.014 x86 0x40A7BD..0x40A7E0 vs portable C++ integer rebound.

We stop at the first x86 continue/terminal destination and read ball force
words. This does not identify a tree object or prove any live course hit.
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
from original_v1014_oracle import build_uc, REFERENCE_SHA256, BALL
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EDI

START=0x40A7BD
DESTINATIONS={0x40A626:True,0x40A7E0:False}

def emulate(uc, pair):
    vertical,horizontal=pair
    uc.mem_write(BALL+0x08,struct.pack("<i",vertical))
    uc.mem_write(BALL+0x0C,struct.pack("<i",horizontal))
    uc.reg_write(UC_X86_REG_EDI,BALL)
    reached=[]
    def on_code(machine,address,size,userdata):
        if address in DESTINATIONS:
            reached.append(address)
            machine.emu_stop()
    hook=uc.hook_add(UC_HOOK_CODE,on_code)
    try:
        uc.emu_start(START,0,count=30)
    finally:
        uc.hook_del(hook)
    if len(reached)!=1:
        raise RuntimeError(f"unclassified original rebound route: {pair}, {reached}")
    out_v=struct.unpack("<i",bytes(uc.mem_read(BALL+0x08,4)))[0]
    out_h=struct.unpack("<i",bytes(uc.mem_read(BALL+0x0C,4)))[0]
    return (out_v,out_h,int(DESTINATIONS[reached[0]])),hex(reached[0])

def vectors():
    edge=(-2147483648,-2147483647,-1073741824,-65536,-8448,-5,-2,
          -1,0,1,2,5,8448,65536,1073741823,1073741824,
          2147483646,2147483647)
    rows=list(product(edge,edge))
    rnd=random.Random(20261011)
    for _ in range(4200):
        rows.append((rnd.randint(-2147483648,2147483647),
                     rnd.randint(-2147483648,2147483647)))
    return rows

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong original Windows v1.014 SHA-256")
    rows=vectors()
    probe=subprocess.run([str(a.probe)],
        input="".join(f"{v} {h}\n" for v,h in rows),
        text=True,capture_output=True,check=True)
    lines=probe.stdout.splitlines()
    if len(lines)!=len(rows):
        raise RuntimeError(f"incorrect native probe output {len(lines)}/{len(rows)}")
    uc,_=build_uc(a.exe)
    mismatches=[]
    targets={}
    for pair,line in zip(rows,lines):
        expected,target=emulate(uc,pair)
        actual=tuple(map(int,line.split()))
        targets[target]=targets.get(target,0)+1
        if expected!=actual:
            mismatches.append({"input":pair,"original":expected,
                               "recovered":actual,"x86_target":target})
    result={
      "schema":"pixtee.original-terminal-rebound-x86.v1",
      "sha256":REFERENCE_SHA256,
      "vectors":len(rows),
      "exact":len(rows)-len(mismatches),
      "routes":targets,
      "mismatches":mismatches[:50],
      "scope":"0x40A7BD..0x40A7E0 rebound and continuation decision only; live tree/object collision not proven"
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(result,indent=2)+"\n")
    print(f"Original terminal rebound: {result['exact']}/{result['vectors']} exact")
    print("Original x86 destinations:",targets)
    for problem in mismatches[:8]:
        print("MISMATCH",problem)
    if mismatches:
        raise SystemExit("terminal original/native rebound mismatch")
    print("PASS exact signed NEG/SAR/SAR/ADD/JG force result")

if __name__=="__main__":
    main()
