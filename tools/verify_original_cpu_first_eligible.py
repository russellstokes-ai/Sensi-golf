#!/usr/bin/env python3
"""Original 1.014 CPU evaluator first eligible candidate status scan parity.

Executes authentic source x86 0x40CA5A..0x40CA8E/0x40CAC4 on
synthetic signed score/status record fixtures. No claim about
candidate metric computation, shot quality, club or CPU strategies.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EBP,UC_X86_REG_EBX,UC_X86_REG_EDX,UC_X86_REG_ECX
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,w16
ENTRY=0x40CA5A
VALID=0x40CA8E
NONE=0x40CAC4
SCORE=0x41FE89
SPAN=0x41FA0F
STEP=0x41FAD3
BYTES=14*110

def signed16(x):
    return x if x<0x8000 else x-0x10000

def max_scanned(span,step):
    if step==0:return None
    cx=0
    for n in range(1,111):
        cx=(cx+step)&0xffff
        if signed16(cx)>=signed16(span):
            return n
    return None

def cases():
    rng=random.Random(0x40ca5a)
    rows=[]
    for span in (0,1,2,7,10,30,50,101,250,500,512,32767,32768,65535):
        for step in (1,2,5,7,10,25,50,77,154,255,500,32767,65535):
            if max_scanned(span,step) is None:continue
            for first in (0,1,2,5,20,80,65535):
                rows.append((span,step,first,rng.randrange(65536)))
    for _ in range(20000):
        if len(rows)>=2700:break
        span=rng.randrange(65536)
        step=rng.randrange(1,65536)
        if max_scanned(span,step) is None:continue
        rows.append((span,step,rng.choice((0,1,2,10,30,75,100,65535)),
                     rng.randrange(65536)))
    return rows

def source(uc,row):
    span,step,first,seed=row
    scratch=bytearray(BYTES)
    for i in range(110):
        if i==first:
            a=seed&0x7fff;b=(seed*3)&0x7fff
        elif (i+seed)%2:
            a=0xffff;b=0
        else:
            a=0;b=0x8000
        struct.pack_into("<HH",scratch,14*i,a,b)
    uc.mem_write(SCORE,bytes(scratch))
    w16(uc,SPAN,span);w16(uc,STEP,step)
    hit=[]
    def cb(machine,address,size,user):
        if address in (VALID,NONE):
            hit.append(address)
            machine.emu_stop()
    hook=uc.hook_add(UC_HOOK_CODE,cb)
    try:uc.emu_start(ENTRY,0,count=2000)
    finally:uc.hook_del(hook)
    if len(hit)!=1:raise RuntimeError("source CPU first-valid scan did not reach exit")
    ebp=uc.reg_read(UC_X86_REG_EBP)
    index=(ebp-SCORE)//14
    if ebp<SCORE or (ebp-SCORE)%14!=0:
        raise RuntimeError(f"source EBP not aligned to original record: {hex(ebp)}")
    return (int(hit[0]==VALID),index,
        uc.reg_read(UC_X86_REG_ECX)&0xffff,
        uc.reg_read(UC_X86_REG_EBX)&0xffff,
        uc.reg_read(UC_X86_REG_EDX)&0xffff)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("--probe",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    a=ap.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original source Windows build mismatch")
    rows=cases()
    proc=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,r))+"\n" for r in rows),
        text=True,capture_output=True,check=True)
    native=[tuple(map(int,line.split())) for line in proc.stdout.splitlines()]
    if len(native)!=len(rows):raise RuntimeError("Native first-eligible probe missing cases")
    uc,_=build_uc(a.exe)
    mismatches=[]
    for idx,(row,out) in enumerate(zip(rows,native)):
        want=source(uc,row)
        if out!=want:
            mismatches.append({"case":idx,"input":row,
                "original":want,"native":out})
    report={"schema":"pixtee.original-cpu-first-eligible-signed-record-scan.x86.v1",
       "original_sha256":REFERENCE_SHA256,
       "source_start":hex(ENTRY),
       "valid_exit":hex(VALID),"exhausted_exit":hex(NONE),
       "cases":len(rows),"exact":len(rows)-len(mismatches),
       "mismatches":mismatches[:16],
       "scope":"Original eligibility/status scan only. Missing original metric generation, full AI strategy and evaluated shot ranking."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original CPU eligible scoring records: {report['exact']}/{len(rows)} exact")
    if mismatches:
        for m in mismatches[:4]:print("MISMATCH",m)
        raise SystemExit("source CPU scoring status eligibility mismatched")
    print("PASS signed original CPU candidate eligibility scan and terminal branches")
if __name__=="__main__":main()
