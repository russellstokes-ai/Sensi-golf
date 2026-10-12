#!/usr/bin/env python3
"""Authenticate original fifth-SPT-record flag coordinate setup at x86.

Original Windows v1.014 source 0x409F39 loads source [0x41EE2E]
SPT pointer, takes bytes at offset 0x28+4/6, swaps endianness and
writes globals 0x41F3B2, 0x41F3B4, marker 0x41F3B0=3. Native course
object reads same original-format words. No physical tree/flag sprite
presence asserted beyond source setup and other proven flag hit predicates.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,PLAYER,w16,wu32
)
START=0x409F39
STOP=0x409F64
def original(uc,x,y):
    resource=PLAYER+0x300
    spt=bytearray(50)
    struct.pack_into(">HH",spt,44,x,y)
    uc.mem_write(resource,bytes(spt))
    wu32(uc,0x41ee2e,resource)
    hits=[]
    def hook(m,a,size,user):
        if a==STOP:hits.append(a);m.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=35)
    finally:uc.hook_del(h)
    if hits!=[STOP]:raise RuntimeError("original SPT flag setup escaped")
    def read(at):return struct.unpack("<H",bytes(uc.mem_read(at,2)))[0]
    return (read(0x41f3b2),read(0x41f3b4),read(0x41f3b0))
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original Windows v1.014 checksum mismatch")
    rng=random.Random(0x409f39)
    cases=[(x,y) for x in (0,1,255,256,1024,32767,32768,65535)
           for y in (0,1,255,256,65535)]
    cases.extend((rng.randrange(65536),rng.randrange(65536))
                 for _ in range(3072))
    native=subprocess.run([str(a.probe)],input="".join(
        f"{x} {y}\n" for x,y in cases),
        text=True,capture_output=True,check=True).stdout.splitlines()
    if len(native)!=len(cases):raise RuntimeError("incomplete original SPT probe")
    uc,_=build_uc(a.exe)
    mismatches=[]
    for row,line in zip(cases,native):
        exp=original(uc,*row)
        got=tuple(map(int,line.split()))
        if exp!=got:mismatches.append({"spt":row,"original":exp,"native":got})
    out={"schema":"pixtee.original-flag-from-spt-record4.x86.v1",
       "original_sha256":REFERENCE_SHA256,"cases":len(cases),
       "exact":len(cases)-len(mismatches),
       "source_start":"0x409F39","source_stop":"0x409F64",
       "mismatches":mismatches[:25],
       "scope":"Proves automatic flag coordinates and marker setup from fifth SPT record. Does not prove additional tree object geometry or host renderer callback."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Original fifth SPT flag setup: {out['exact']}/{len(cases)} exact")
    for m in mismatches[:5]:print("MISMATCH",m)
    if mismatches:raise SystemExit("source SPT flag/native parity mismatch")
    print("PASS authentic original flag coordinates and global presence marker")
if __name__=="__main__":main()
