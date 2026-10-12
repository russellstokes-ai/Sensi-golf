#!/usr/bin/env python3
"""Source x86 0x40C50B ten animation timers vs native C++.

All active animations are in their positive-timer stage, so the real
original decoder 0x40C539 is not invoked. Covers original round-robin
slot update and event slot preservation (not unseen animation scripts).
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_ESP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,STACK,SENTINEL)
SLOTS=0x41f8f0
START=0x40c50b
def rows():
    rng=random.Random(0x40c50b)
    cases=[]
    for count in range(11):
        for _ in range(270):
            active_indices=set(rng.sample(range(10),count))
            slots=[]
            for i in range(10):
                st=rng.choice((0,1,2,0xfffe)) if i in active_indices else 65535
                ticks=rng.choice((1,2,3,4,5,100,65535))
                slots.append((st,rng.randrange(65536),
                    rng.randrange(65536),rng.randrange(65536),
                    rng.randrange(65536),ticks,
                    rng.getrandbits(32),rng.getrandbits(32)))
            cases.append(slots)
    return cases
def machine(uc,slots):
    uc.mem_write(SLOTS,b"".join(
        struct.pack("<6H2I",*slot) for slot in slots))
    sp=STACK+0xf000-4
    uc.mem_write(sp,struct.pack("<I",SENTINEL))
    uc.reg_write(UC_X86_REG_ESP,sp)
    uc.emu_start(START,SENTINEL,count=650)
    actual=[]
    for i in range(10):
        actual.extend(struct.unpack(
            "<6H2I",bytes(uc.mem_read(SLOTS+i*20,20))))
    active=sum(s[0]!=65535 for s in slots)
    newly_ready=sum(s[0]!=65535 and s[5]==1 for s in slots)
    return [active,newly_ready]+actual
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong original GOLFWIN.EXE")
    cases=rows()
    feed="".join(" ".join(map(str,
             (v for slot in case for v in slot)))+"\n"
             for case in cases)
    native=subprocess.run([str(a.probe)],input=feed,
        text=True,capture_output=True,check=True).stdout.splitlines()
    if len(native)!=len(cases):
        raise RuntimeError("original source native animation tick output count mismatch")
    uc,_=build_uc(a.exe)
    mismatch=[]
    for case,line in zip(cases,native):
        source=machine(uc,case)
        actual=[int(v) for v in line.split()]
        if actual!=source:
            mismatch.append({"original":source[:18],"native":actual[:18]})
    out={"schema":"pixtee.original-animation-timer-dispatch-x86.v1",
      "source_sha256":REFERENCE_SHA256,
      "start":"0x40C50B","return":"original RET to test-only sentinel",
      "cases":len(cases),"exact":len(cases)-len(mismatch),
      "mismatches":mismatch[:24],
      "scope":"All ten original animation-slot tick dispatch, active timer>0 only; script decode and host renderer still unverified."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Original animation ten-slot timer tick: {out['exact']}/{len(cases)} exact")
    for x in mismatch[:6]:print("MISMATCH",x)
    if mismatch:raise SystemExit("animation source timer parity failed")
    print("PASS original ten-slot animation timer scheduling without fabricated scripts")
if __name__=="__main__":main()
