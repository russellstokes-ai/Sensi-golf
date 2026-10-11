#!/usr/bin/env python3
"""Compare original 0x40C665..0x40C6B2 search parameters with native C++.
This is authentic CPU-difficulty parameter extraction, not an entire AI.
"""
import argparse,hashlib,json,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,ru16,w16
from unicorn.x86_const import UC_X86_REG_ESI

def original(uc,mode,special):
    w16(uc,PLAYER+0x5a,mode)
    w16(uc,PLAYER+0x62,1 if special else 0)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.emu_start(0x40c665,0x40c6b2,count=85)
    return [ru16(uc,0x41fa0b),ru16(uc,0x41fa0d),ru16(uc,0x41fa0f)]

def main():
    a=argparse.ArgumentParser()
    a.add_argument("--exe",required=True,type=Path)
    a.add_argument("--probe",required=True,type=Path)
    a.add_argument("-o","--output",required=True,type=Path)
    p=a.parse_args()
    assert hashlib.sha256(p.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    uc,_=build_uc(p.exe)
    cases=[(m,f) for m in range(4,16) for f in (0,1)]
    native=subprocess.run([str(p.probe)],input="".join(f"{m} {f}\n" for m,f in cases),
                          text=True,capture_output=True,check=True)
    lines=native.stdout.strip().splitlines()
    assert len(lines)==len(cases)
    mismatches=[]
    for (m,f),line in zip(cases,lines):
        raw=original(uc,m,f)
        actual=list(map(int,line.split()))
        if actual!=raw:mismatches.append({"mode":m,"flag":f,"original":raw,"native":actual})
    p.output.parent.mkdir(parents=True,exist_ok=True)
    p.output.write_text(json.dumps({"schema":"pixtee.original-computer-skill-params.v1",
        "original_sha256":REFERENCE_SHA256,"cases":len(cases),
        "exact_matches":len(cases)-len(mismatches),"mismatches":mismatches,
        "scope":"only original CPU budget/difficulty arithmetic; NOT complete CPU shot planner"},indent=2)+"\n")
    print(f"Original CPU difficulty search parameters: {len(cases)-len(mismatches)}/{len(cases)}")
    if mismatches:raise SystemExit(str(mismatches[:3]))
    print("PASS original CPU difficulty source values; shot-planning and AI remain open")
if __name__=="__main__":main()
