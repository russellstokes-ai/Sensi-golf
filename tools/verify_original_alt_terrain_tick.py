#!/usr/bin/env python3
"""Authentic second original Windows 1.014 course tick terrain branches.

Execute original 0x40CC2D..0x40CC74 and compare native C++ numeric
landing 35/60, 8-10 and all other-code rebounds. This is a source
subroutine, not evidence that the full world/object renderer or AI tick
has been integrated.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from itertools import product
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EDI,UC_X86_REG_ESP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,BALL,STACK,w16,wi32,ri32
)
START=0x40CC2D
EXIT={0x40CC74:0,0x40CC4D:1,0x40CBCD:2}
CODES=(0,1,2,3,4,5,6,7,8,9,10,11,34,35,36,50,59,60,61,76,65535)
V=(-2147483648,-1073741824,-65536,-8448,-1,0,1,8448,65536,
   1073741824,2147483647)
H=(-2147483648,-1000000,-65536,-1,0,1,100,50000,65536,1000000,2147483647)
def vectors():
    rng=random.Random(0x40CC2D)
    rows=[]
    for code in CODES:
        for v in V:
            for h in H:
                rows.append((code,v,h))
    for _ in range(5400):
        rows.append((rng.choice(CODES+(rng.randrange(65536),)),
                     rng.choice(V+(rng.randrange(-2147483648,2147483648),)),
                     rng.choice(H+(rng.randrange(-2147483648,2147483648),))))
    return rows
def original(uc,row):
    code,v,h=row
    uc.mem_write(BALL,b"\x00"*0x60)
    w16(uc,BALL+0x1e,code)
    wi32(uc,BALL+8,v)
    wi32(uc,BALL+0xc,h)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    branch=[]
    def hook(machine,address,size,user):
        if address in EXIT:
            branch.append(address);machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=100)
    finally:uc.hook_del(token)
    if len(branch)!=1:
        raise RuntimeError(f"original second terrain branch escaped: {row}, {branch}")
    return (EXIT[branch[0]],ri32(uc,BALL+8),ri32(uc,BALL+0xc)),hex(branch[0])
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original Windows source executable checksum failure")
    rows=vectors()
    proc=subprocess.run([str(a.probe)],input="".join(
        " ".join(map(str,row))+"\n" for row in rows),
        text=True,capture_output=True,check=True)
    lines=proc.stdout.splitlines()
    if len(lines)!=len(rows):
        raise RuntimeError("native original alternate terrain count mismatch")
    uc,_=build_uc(a.exe)
    counts={};errs=[];terrain_groups={"35":0,"60":0,"8to10":0,"other":0}
    for row,line in zip(rows,lines):
        expected,exit_addr=original(uc,row)
        native=tuple(int(x) for x in line.split())
        counts[exit_addr]=counts.get(exit_addr,0)+1
        terrain_groups["35" if row[0]==35 else "60" if row[0]==60
            else "8to10" if 8<=row[0]<=10 else "other"]+=1
        if native!=expected:
            errs.append({"input":row,"original":expected,"native":native,
                         "x86_exit":exit_addr})
    report={"schema":"pixtee.original-second-terrain-tick-numeric-code35-60-x86.v1",
       "original_sha256":REFERENCE_SHA256,
       "source_range":"0x40CC2D..0x40CC74",
       "vectors":len(rows),"exact":len(rows)-len(errs),
       "original_routes":counts,"terrain_groups":terrain_groups,
       "mismatches":errs[:32],
       "limits":"Isolated second-course-tick source branch only; not original tree collision object geometry or live world scheduling."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original second course-tick terrain: {report['exact']}/{len(rows)} exact")
    print("Source branches:",counts,"source code classes:",terrain_groups)
    for error in errs[:8]:print("MISMATCH",error)
    if errs:raise SystemExit("original second terrain tick/native diverged")
    print("PASS original source code35/code60 versus cup8-10 and signed rebound")
if __name__=="__main__":main()
