#!/usr/bin/env python3
"""Emulate REAL original v1.014 0x40C71B..0x40C77F CPU aiming sweep.
Capture every candidate heading at 0x40C73D, skip ORIGINAL shot-evaluation
helper (unknown) with a pointer to benign 14-byte data, compare to native C++.
The helper's candidate quality / clubs / actual simulated shots remain OPEN.
"""
import argparse,hashlib,json,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,BALL,STACK,w16,wu32,ru16,ru32
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP,UC_X86_REG_EIP,UC_X86_REG_EBX,UC_X86_REG_ESP
SRC=0x40c71b;END=0x40c77f;HOOK=0x40c93e;CAPTURE=0x40c73d
BASE=0x41fa11;GRID0=0x41fad1;GRID1=0x41fad5;GRID2=0x41fad9
COUNTDIV=0x420533;FIRSTCOUNT=0x420531
def read_table(uc,mode,band):
    return struct.unpack("<4H",bytes(uc.mem_read(BASE+band*48+(mode-4)*8,8)))
def original(uc,mode,band,heading):
    words=read_table(uc,mode,band)
    n,first,step,unknown=words
    if n==0 or n>110:raise ValueError("unverified original CPU sweep candidate count")
    w16(uc,GRID0,n);wu32(uc,GRID1,first);wu32(uc,GRID2,step)
    w16(uc,COUNTDIV,154//n);w16(uc,FIRSTCOUNT,154-154%n)
    wu32(uc,BALL+0x10,heading)
    wu32(uc,PLAYER+0x12,heading)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    uc.mem_write(PLAYER+0x800,b"\0"*14)
    readings=[];calls=[]
    def hook(machine,addr,size,user):
        if addr==CAPTURE:
            readings.append(ru32(machine,BALL+0x10)&0xfff)
        elif addr==HOOK:
            sp=machine.reg_read(UC_X86_REG_ESP)
            ret=ru32(machine,sp)
            machine.reg_write(UC_X86_REG_ESP,sp+4)
            machine.reg_write(UC_X86_REG_EBX,PLAYER+0x800)
            machine.reg_write(UC_X86_REG_EIP,ret)
            calls.append(addr)
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(SRC,END,count=10000)
    finally:uc.hook_del(token)
    if len(readings)!=n or len(calls)!=n:
        raise AssertionError(f"original callbacks incorrect: {mode},{band},{heading},{len(readings)},{len(calls)},{n}")
    return [len(readings),ru32(uc,BALL+0x10)&0xfff,
            ru32(uc,PLAYER+0x12)&0xfff,ru16(uc,FIRSTCOUNT)]+readings
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    cases=[(m,b,h) for b in range(4) for m in range(4,10)
           for h in (0,1,512,2048,4095)]
    native=subprocess.run([str(a.probe)],input="".join(f"{m} {b} {h}\n" for m,b,h in cases),
                          text=True,capture_output=True,check=True)
    lines=native.stdout.strip().splitlines();assert len(lines)==len(cases)
    uc,_=build_uc(a.exe);bad=[];matched=0;count=0
    for case,line in zip(cases,lines):
        source=original(uc,*case);actual=list(map(int,line.split()))
        count+=source[0]
        if source==actual:matched+=1
        else:bad.append({"input":case,"source":source[:12],"native":actual[:12]})
    report={"schema":"pixtee.original-cpu-aiming-sweep-x86.v1",
        "exe_sha256":REFERENCE_SHA256,"cases":len(cases),
        "total_original_candidate_callbacks":count,
        "exact_matches":matched,"mismatches":bad[:40],
        "scope":"original CPU heading search trajectory; original per-heading shot evaluation and candidate choice not included"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original CPU full heading sweep source x86 vs C++: {matched}/{len(cases)}")
    print("Original candidate callback headings compared:",count)
    for row in bad[:5]:print("MISMATCH",row)
    if bad:raise SystemExit("original CPU heading sweep mismatch")
    print("PASS original CPU complete table-driven heading trajectory")
if __name__=="__main__":main()
