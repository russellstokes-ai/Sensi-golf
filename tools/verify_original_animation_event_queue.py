#!/usr/bin/env python3
"""Original Windows 1.014 animated-event slot allocator 0x40C4C0 oracle.

Verifies the FIRST free slot of 10 original 0x14-byte records,
exact x/y/width/height/resource-pointer/timer writes and unchanged
frame offset, using original GOLFWIN.EXE x86 under Unicorn. Does not
claim animation frames/visual timing after original 0x40C50B.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,
    UC_X86_REG_EDX,UC_X86_REG_EBP)
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256
SLOTS=0x41f8f0
ASSETS=0x41f890
START=0x40c4c0
EXIT={0x40c4d6:-1,0x40c50a:0}
def rows():
    rng=random.Random(0x40c4c0)
    out=[]
    for i in range(10):
        for _ in range(45):
            statuses=[rng.choice((0,1,2,3,0xfffe)) for j in range(10)]
            statuses[i]=0xffff
            for j in range(i):statuses[j]=rng.choice((0,1,2,0xfffe))
            slots=[]
            for st in statuses:
                slots.append((st,
                   rng.randrange(65536),rng.randrange(65536),
                   rng.randrange(65536),rng.randrange(65536),
                   rng.randrange(65536),rng.getrandbits(32),
                   rng.getrandbits(32)))
            out.append((rng.randrange(65536),rng.randrange(65536),
                rng.randrange(65536),rng.randrange(65536),
                rng.randrange(16),
                [rng.getrandbits(32) for _ in range(16)],slots))
    for _ in range(2900):
        slots=[]
        for i in range(10):
            st=rng.choice((0xffff,0xffff,0,1,2,0xfffe))
            slots.append((st,
                rng.randrange(65536),rng.randrange(65536),
                rng.randrange(65536),rng.randrange(65536),
                rng.randrange(65536),rng.getrandbits(32),
                rng.getrandbits(32)))
        if rng.randrange(5)==0:
            slots=[(0,)+v[1:] for v in slots]
        out.append((rng.randrange(65536),rng.randrange(65536),
                    rng.randrange(65536),rng.randrange(65536),
                    rng.randrange(16),
                    [rng.getrandbits(32) for _ in range(16)],slots))
    return out
def machine(uc,data):
    x,y,w,h,id,assets,slots=data
    uc.mem_write(ASSETS,struct.pack("<16I",*assets))
    uc.mem_write(SLOTS,b"".join(struct.pack("<6H2I",*r) for r in slots))
    for reg,val in ((UC_X86_REG_EAX,x),(UC_X86_REG_EBX,y),
                    (UC_X86_REG_ECX,w),(UC_X86_REG_EDX,h),
                    (UC_X86_REG_EBP,id)):
        uc.reg_write(reg,val)
    seen=[]
    def hook(m,address,size,user):
        if address in EXIT:
            seen.append(address)
            m.emu_stop()
    hook_id=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=250)
    finally:uc.hook_del(hook_id)
    if len(seen)!=1:raise RuntimeError("original animation event left known source branch")
    expected_index=next((i for i,slot in enumerate(slots) if slot[0]==0xffff),-1)
    slot_from_source=expected_index if seen[0]==0x40c50a else -1
    if slot_from_source!=expected_index:
        raise RuntimeError(f"source event handler did not choose expected first free slot: {seen}")
    final=[]
    for i in range(10):
        final.extend(struct.unpack(
            "<6H2I",bytes(uc.mem_read(SLOTS+i*20,20))))
    return [slot_from_source]+final,hex(seen[0])
def main():
    a=argparse.ArgumentParser()
    a.add_argument("--exe",type=Path,required=True)
    a.add_argument("--probe",type=Path,required=True)
    a.add_argument("-o","--output",type=Path,required=True)
    o=a.parse_args()
    if hashlib.sha256(o.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original Windows executable SHA-256 mismatch")
    cases=rows()
    feed="".join(
        " ".join(str(x) for x in (
            [v[0],v[1],v[2],v[3],v[4]]+v[5]+
            [member for slot in v[6] for member in slot]))+"\n"
        for v in cases)
    process=subprocess.run([str(o.probe)],input=feed,text=True,
                           capture_output=True,check=True)
    outs=process.stdout.splitlines()
    if len(outs)!=len(cases):
        raise RuntimeError(f"original animation probe output {len(outs)}/{len(cases)}")
    uc,_=build_uc(o.exe)
    mismatch=[];branches={}
    for row,text_out in zip(cases,outs):
        expected,route=machine(uc,row)
        branches[route]=branches.get(route,0)+1
        actual=[int(x) for x in text_out.split()]
        if actual!=expected:
            mismatch.append({"source_branch":route,
               "first_input":row[:5],
               "expected":expected[:19],"native":actual[:19]})
    report={"schema":"pixtee.original-animation-event-allocator-x86.v1",
      "source_sha256":REFERENCE_SHA256,
      "source_entry":"0x40C4C0","slots":10,"slot_bytes":20,
      "cases":len(cases),"exact":len(cases)-len(mismatch),
      "original_branches":branches,"mismatches":mismatch[:20],
      "scope":"Source event enqueue allocation only. No source animation cursor/timing, graphics, sound, or user UI acknowledgement."}
    o.output.parent.mkdir(parents=True,exist_ok=True)
    o.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original 10-slot animation event enqueue: {report['exact']}/{len(cases)} exact")
    print("Original x86 exit destinations:",branches)
    for item in mismatch[:5]:print("MISMATCH",item)
    if mismatch:raise SystemExit("original event slot/native divergence")
    print("PASS source x86 first free slot, asset pointer, event parameters and frame offset preservation")
if __name__=="__main__":main()
