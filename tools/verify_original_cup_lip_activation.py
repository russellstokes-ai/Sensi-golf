#!/usr/bin/env python3
"""Actual x86 v1.014 0x40A456..0x40A4A8 cup-lip activation vs C++.
Emulate the exact original conditional transfers; stop BEFORE original
RNG or scorer, which are separately tested. No imaginary objects.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from itertools import product
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,BALL
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI

START=0x40a456
BOUNCE=0x40a47a
TERMINAL=0x40a4a8
OTHER=0x40a501
def original(uc,case):
    dist,mode,height,horiz=case
    uc.mem_write(BALL+0x18,struct.pack("<I",0 if dist else 100))
    uc.mem_write(BALL+0x14,struct.pack("<i",height))
    uc.mem_write(BALL+0x0c,struct.pack("<i",horiz))
    uc.mem_write(PLAYER+0x5a,struct.pack("<H",mode))
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    reached=[]
    def hook(machine,addr,size,user):
        if addr in (BOUNCE,TERMINAL,OTHER):
            reached.append(addr)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=60)
    finally:uc.hook_del(token)
    if len(reached)!=1:
        raise RuntimeError(f"original cup lip stopped unexpectedly {case}: {reached}")
    return int(reached[0]==BOUNCE),hex(reached[0])
def make_cases():
    rows=list(product((0,1),(0,1,3,4,6,12,65535),
        (-65536,-1,0,1,0x31,0x32,0x33,0x140000),
        (0,0xcfff,0xd000,0xd001,0x7fffffff,-1)))
    rng=random.Random(1014)
    for _ in range(2500):
        rows.append((rng.randrange(2),
            rng.choice((0,1,2,3,4,12,65535)),
            rng.choice((-100000,-1,0,50,51,52,rng.randint(-100000,100000))),
            rng.choice((0,0xcfff,0xd000,0xd001,-1,rng.randint(-1000,1000000)))))
    return rows
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    cases=make_cases()
    response=subprocess.run([str(a.probe)],input="".join("%d %d %d %d\n"%x for x in cases),
        text=True,capture_output=True,check=True)
    answers=response.stdout.strip().splitlines()
    assert len(answers)==len(cases)
    uc,_=build_uc(a.exe)
    bad=[];matched=0;paths={}
    for case,line in zip(cases,answers):
        expected,route=original(uc,case)
        paths[route]=paths.get(route,0)+1
        got=int(line)
        if expected==got:matched+=1
        else:bad.append({"input":case,"original":expected,"native":got,"x86_target":route})
    report={"schema":"pixtee.original-cup-lip-activation-branch.v1",
        "sha256":REFERENCE_SHA256,"case_count":len(cases),"exact_matches":matched,
        "original_paths":paths,"mismatches":bad[:100],
        "scope":"Actual x86 cup-lip ACTIVATION only; not dynamic near-hole distance calculation or entire bounce/recovery"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original x86 cup lip activation vs native: {matched}/{len(cases)} exact")
    print("Original branch targets",paths)
    for issue in bad[:10]:print("MISMATCH",issue)
    if bad:raise SystemExit("native cup-lip activation not exact")
    print("PASS exact original conditional paths including signed-negative height")
if __name__=="__main__":main()
