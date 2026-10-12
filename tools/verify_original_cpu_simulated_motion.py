#!/usr/bin/env python3
"""Source x86 verify original CPU alternate tick drag + 12-bit Q14 movement.

Run Windows v1.014 0x40CBCD..0x40CC9F in Unicorn and stop at either
0x40CC9F next trial tick or 0x40CC05 mapped-contact recheck. Original
trigonometric source arrays are read from the authentic SHA-matched EXE.
Isolated per-tick test: earlier contact and following course lookups are
outside this parity scope.
"""
from __future__ import annotations
import argparse,hashlib,json,random,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EDI
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,BALL,wi32,wu32,w16,ri32
)
START=0x40cbcd
TERMINALS={0x40cc9f:0,0x40cc05:1}
GREEN=0x41ee7a
def original(uc,row):
    x,y,h,height,direction,green=row
    uc.mem_write(BALL,b"\0"*0x60)
    wi32(uc,BALL,x)
    wi32(uc,BALL+4,y)
    wi32(uc,BALL+0xc,h)
    wi32(uc,BALL+0x14,height)
    wu32(uc,BALL+0x10,direction)
    w16(uc,GREEN,green)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    result=[]
    def hook(machine,address,size,user):
        if address in TERMINALS:
            result.append(address)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=105)
    finally:uc.hook_del(token)
    if len(result)!=1:
        raise RuntimeError(f"original CPU motion state escaped source: {row}: {result}")
    return (TERMINALS[result[0]],ri32(uc,BALL),ri32(uc,BALL+4),
            ri32(uc,BALL+0xc))
def sample():
    rng=random.Random(0x40cbcd)
    cases=[]
    moments=(0,1,-1,0x77f,0x780,0x781,0xeff,0xf00,0xf01,
             0x2000,0x100000,0x7fffffff,-2147483648)
    headings=(0,1,255,511,512,1023,1024,1536,2048,3072,3584,4095)
    coords=(0,1,-1,0x12340000,-0x12340000,2147483647,-2147483648)
    for h in moments:
        for green in (0,1,2,65535):
            for direction in headings:
                cases.append((rng.choice(coords),rng.choice(coords),h,
                              rng.choice((0,1,2,0x32,65536)),direction,green))
    for _ in range(9700):
        cases.append((rng.choice(coords+(rng.randrange(-2**31,2**31),)),
          rng.choice(coords+(rng.randrange(-2**31,2**31),)),
          rng.choice(moments+(rng.randrange(-2**31,2**31),)),
          rng.choice((0,1,2,0x32,65536,rng.randrange(2**31))),
          rng.randrange(4096),rng.choice((0,1,2,65535))))
    return cases
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong source original Windows game SHA")
    rows=sample()
    cp=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,row))+"\n" for row in rows),
        text=True,capture_output=True,check=True)
    native=cp.stdout.splitlines()
    if len(native)!=len(rows):
        raise RuntimeError("native CPU planar tick probe row count mismatch")
    uc,_=build_uc(a.exe)
    misses=[];routes={}
    for row,line in zip(rows,native):
        expected=original(uc,row)
        actual=tuple(map(int,line.split()))
        routes[str(expected[0])]=routes.get(str(expected[0]),0)+1
        if expected!=actual:misses.append({"input":row,
                "source":expected,"native":actual})
    report={"schema":"pixtee.original-cpu-alternate-Q14-drag-motion-x86.v1",
        "sha256":REFERENCE_SHA256,
        "source":"0x40CBCD..0x40CC9F, original trig tables",
        "vectors":len(rows),"exact":len(rows)-len(misses),
        "source_routes":routes,
        "mismatches":misses[:30],
        "excluded":"Full CPU candidate rollout 0x40CB00, original map/object contact, source PRNG and mode controller"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original alternate CPU planar and drag: {report['exact']}/{len(rows)} exact")
    print("Original source route counts:",routes)
    for m in misses[:8]:print("MISMATCH",m)
    if misses:raise SystemExit("original CPU alternate planar source mismatch")
    print("PASS original CPU horizontal force, map recheck and Q14 X/Y projection")
if __name__=="__main__":main()
