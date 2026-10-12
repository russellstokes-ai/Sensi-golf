#!/usr/bin/env python3
"""Source x86 parity for CPU evaluator candidate scoring scratch reset.

Executes original Windows v1.014 0x40C93E..0x40C968 on real
SHA-authenticated executable instructions. Only this bounded scratch
initialization is proven. CPU trajectory quality, club strategy, RNG
and AI gameflow remain unverified.
"""
from __future__ import annotations
import argparse,hashlib,json,random,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,STACK,w16,wu32,ru32)
ENTRY=0x40C93E
STOP=0x40C968
BUFFER=0x41FE89
SPAN=0x41FA0F
STEP=0x41FAD3
ADJUST=0x41F32C
BYTES=14*110

def count_safe(span,step):
    if step==0:return None
    dx=span
    for count in range(1,111):
        dx=(dx-step)&0xffff
        if dx&0x8000:return count
    return None

def vectors():
    rng=random.Random(0x40c93e)
    spans=(0,1,4,7,13,33,65,100,150,350,512,1000,
           32767,32768,65534,65535)
    steps=(1,2,3,5,10,13,50,154,255,1024,4096,
           32767,32768,65535)
    rows=[]
    for span in spans:
        for step in steps:
            if count_safe(span,step) is not None:
                rows.append((span,step,rng.randrange(65536)))
    for _ in range(50000):
        if len(rows)>=1750:break
        span=rng.randrange(65536)
        step=rng.randrange(1,65536)
        if count_safe(span,step) is not None:
            rows.append((span,step,rng.randrange(65536)))
    return rows

def digest(seq):
    h=2166136261
    for b in seq:h=((h^b)*16777619)&0xffffffff
    return h

def original(uc,row):
    span,step,seed=row
    seed_bytes=bytes((seed+i*13)&0xff for i in range(BYTES))
    uc.mem_write(BUFFER,seed_bytes)
    w16(uc,SPAN,span)
    w16(uc,STEP,step)
    wu32(uc,ADJUST,0xc001d00d)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    reached=[]
    def hook(machine,address,size,user):
        if address==STOP:
            reached.append(address)
            machine.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(ENTRY,0,count=4000)
    finally:uc.hook_del(h)
    if reached!=[STOP]:raise RuntimeError("original CPU score scratch reset not finished")
    return (count_safe(span,step),ru32(uc,ADJUST),
            digest(bytes(uc.mem_read(BUFFER,BYTES))))

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original Windows v1.014 checksum mismatch")
    rows=vectors()
    result=subprocess.run([str(a.probe)],
       input="".join(" ".join(map(str,r))+"\n" for r in rows),
       text=True,capture_output=True,check=True)
    native=[tuple(map(int,line.split())) for line in result.stdout.splitlines()]
    if len(native)!=len(rows):raise RuntimeError("missing native CPU scratch states")
    uc,_=build_uc(a.exe)
    mismatches=[]
    for i,(r,got) in enumerate(zip(rows,native)):
        want=original(uc,r)
        if want!=got:
            mismatches.append({"case":i,"inputs":r,"source":want,"native":got})
    out={"schema":"pixtee.original-cpu-quality-record-init-x86.v1",
       "original_sha256":REFERENCE_SHA256,
       "source_entry":"0x40C93E","source_stop":"0x40C968",
       "record_stride_bytes":14,
       "cases":len(rows),"exact":len(rows)-len(mismatches),
       "mismatches":mismatches[:16],
       "scope":"Only original candidate scratch initialization and reset of swing-adjuster; no CPU trajectory evaluation or ranking proved."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Original CPU candidate score scratch: {out['exact']}/{len(rows)} exact")
    for m in mismatches[:3]:print("MISMATCH",m)
    if mismatches:raise SystemExit("original CPU record init mismatch")
    print("PASS original 14-byte candidate record initialization, no fabricated AI scores")
if __name__=="__main__":main()
