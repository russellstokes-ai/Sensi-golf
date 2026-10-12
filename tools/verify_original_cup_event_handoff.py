#!/usr/bin/env python3
"""Authentic Windows 1.014 cup code8 0x40CD25 event handoff oracle.

Reuse the already verified original OOB display/source event memory probe,
but ENTER from the distinct original 0x40CD25 branch, and compare the
cup-specific message ID and resource pointer. No original GUI copied.
"""
from __future__ import annotations
import argparse,hashlib,json,random,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import verify_original_world_oob_event_handoff as event
from original_v1014_oracle import build_uc,REFERENCE_SHA256

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("not authentic original v1.014 GOLFWIN.EXE")
    rng=random.Random(0x40cd25)
    rows=[tuple(rng.getrandbits(n) for n in (8,32,32,32))
          for _ in range(1280)]
    native=subprocess.run([str(a.probe)],input="".join(
        str(i)+"\n" for i in range(len(rows))),
        text=True,capture_output=True,check=True).stdout.splitlines()
    if len(native)!=len(rows):raise RuntimeError("incomplete native cup event")
    uc,_=build_uc(a.exe)
    event.START=0x40cd25
    mismatch=[]
    for row,line in zip(rows,native):
        expected=event.run(uc,row)
        actual=tuple(map(int,line.split()))
        if expected!=actual:
            mismatch.append({"input":row,"original":expected,"native":actual})
    out={"schema":"pixtee.original-code8-cup-event-x86.v1",
         "original_sha256":REFERENCE_SHA256,
         "source_range":"0x40CD25..0x40CFE9",
         "cases":len(rows),"exact":len(rows)-len(mismatch),
         "mismatches":mismatch[:30],
         "scope":"Original terminal code8 cup presentation request; no original host UI, prompt delay or turn advancement"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Original code8 cup event handoff: {out['exact']}/{len(rows)} exact")
    for m in mismatch[:5]:print("MISMATCH",m)
    if mismatch:raise SystemExit("cup event branch parity mismatch")
    print("PASS authentic source cup event ID 0x1000F and resource pointer")
if __name__=="__main__":main()
