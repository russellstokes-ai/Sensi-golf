#!/usr/bin/env python3
"""Run full original Welly OUTER state dispatcher from initial first click.

Original SHA-verified Windows v1.014 0x4075FF -> 0x407620 setup ->
0x40768B rising -> 0x407787 falling -> completion. Compares native
startup, samples and source state exactly. Original byte keyboard bit
0x10 is injected at each logical invocation; evidence DOES NOT cover
actual OS input poll or real-time scheduler cadence.
"""
from __future__ import annotations
import argparse,hashlib,json,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER
from trace_original_welly_machine_code import (
    execute,w8,w16,w32,r16
)
from unicorn.x86_const import UC_X86_REG_ESP
INPUT=0x41F000
ENTRY=0x4075FF
ADDR=(0x41d58c,0x41d592,0x41d594,0x41d596,0x41d59a,
      0x41d59c,0x41d59e,0x41d5a0,0x41d5a2)
def state(uc):
    return (r16(uc,ADDR[0]),r16(uc,ADDR[1]),r16(uc,ADDR[2]),
        struct.unpack("<I",bytes(uc.mem_read(ADDR[3],4)))[0],
        *[r16(uc,addr) for addr in ADDR[4:]])
def run(exe,club,rise,fall):
    uc,_=build_uc(exe)
    uc.mem_write(PLAYER,b"\0"*0x100)
    w32(uc,0x41f3a8,PLAYER)
    w16(uc,PLAYER+0x28,club)
    w16(uc,0x41d58c,1)
    w8(uc,0x42db56,0)
    w8(uc,INPUT,0x10)
    execute(uc,ENTRY)
    if state(uc)[0]!=2:
        raise RuntimeError("original UI meter startup did not enter rising stage")
    observed=[state(uc)]
    fell_on=-1
    for t in range(190):
        before=state(uc)[0]
        if before==0:break
        pressed=(t==rise) or (fell_on>=0 and t-fell_on==fall)
        w8(uc,INPUT,0x10 if pressed else 0)
        execute(uc,ENTRY)
        after=state(uc)
        observed.append(after)
        if before==2 and after[0]==3:fell_on=t
    return observed
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong original GOLFWIN.EXE")
    scenarios=[(club,rise,fall)
        for club in (0,5,12)
        for rise,fall in ((12,10),(0,0),(80,69),(-1,-1),
                           (-1,10),(12,-1),(50,3))]
    rows=subprocess.run([str(a.probe)],input="".join(
        f"{c} {r} {f}\n" for c,r,f in scenarios),
        text=True,capture_output=True,check=True).stdout.splitlines()
    original_rows=[]
    for club,rise,fall in scenarios:
        original_rows.extend(run(a.exe,club,rise,fall))
    native=[tuple(map(int,line.split())) for line in rows]
    if len(native)!=len(original_rows):
        raise RuntimeError(f"source native outer callback count mismatch {len(native)} != {len(original_rows)}")
    mismatch=[]
    for i,(got,want) in enumerate(zip(native,original_rows)):
        if got!=want:
            mismatch.append({"sample":i,"original":want,"native":got})
    result={
        "schema":"pixtee.original-welly-outer-state-dispatcher-source-x86.v1",
        "sha256":REFERENCE_SHA256,
        "source_entry":"0x4075FF (indirect stage jump table)",
        "source_init":"0x407620 (first button) to rising 0x40768B",
        "scenarios":len(scenarios),"sample_comparisons":len(native),
        "exact":len(native)-len(mismatch),
        "mismatches":mismatch[:50],
        "remaining":"Operating-system input-to-source bit0x10 debouncing, real wallclock frame scheduling and original UI colours not yet proven."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(result,indent=2)+"\n")
    print(f"Original outer Welly UI startup + rising/falling: {result['exact']}/{len(native)} exact")
    for item in mismatch[:8]:print("MISMATCH",item)
    if mismatch:raise SystemExit("original outer Welly machine parity mismatch")
    print("PASS original stage-dispatch startup value, accumulator and tick state")
if __name__=="__main__":main()
