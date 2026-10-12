#!/usr/bin/env python3
"""CPU scoring against the UNMODIFIED original v1.014 static penalty data.

Original signed source 0x40C9FD..0x40CA1E reads numeric gameplay WORDs
at 0x41FADF plus code*2. This test does not inject/modify that table.
The previous synthetic-table 8324-vector proof verifies source algebra.
Source gameplay table after full course/mode initialization remains a
separate unverified state-transition topic.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_EBP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,PLAYER,w16,ru16
)
START=0x40c9fd
END=0x40ca1e
PENALTY_TABLE=0x41fadf
GREEN_MODE=0x41ee7a
def source(machine,distance,code,green):
    w16(machine,PLAYER+0xa,code)
    w16(machine,PLAYER+0xc,0)
    w16(machine,GREEN_MODE,green)
    machine.reg_write(UC_X86_REG_EBP,PLAYER)
    machine.reg_write(UC_X86_REG_EAX,0xd00d0000|distance)
    machine.emu_start(START,END,count=40)
    return ru16(machine,PLAYER+0xc)
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original Windows 1.014 sha256 mismatch")
    uc,_=build_uc(a.exe)
    data=struct.unpack("<32H",bytes(uc.mem_read(PENALTY_TABLE,64)))
    rng=random.Random(0x41fadf)
    cases=[(0,code,0) for code in range(32)]
    cases += [(distance,code,mode)
        for code in range(32) for mode in (0,1,2,65535)
        for distance in (0,1,2,100,32767,32768,65535)]
    cases += [(rng.randrange(65536),rng.randrange(32),
               rng.choice((0,1,2,65535))) for _ in range(4300)]
    native=subprocess.run([str(a.probe)],
        input="".join(f"{d} {c} {g}\n" for d,c,g in cases),
        text=True,capture_output=True,check=True).stdout.splitlines()
    if len(native)!=len(cases):raise RuntimeError("original CPU static table native vectors truncated")
    mismatches=[]
    for case,line in zip(cases,native):
        expected=source(uc,*case)
        actual=int(line)
        if expected!=actual:
            mismatches.append({"input":case,"original":expected,"native":actual})
    rep={"schema":"pixtee.original-cpu-actual-static-penalties-x86.v1",
        "source_sha256":REFERENCE_SHA256,
        "source_data_address":"0x41FADF (unmodified, original executable image)",
        "original_table_words":list(data),
        "source_x86":"0x40C9FD..0x40CA1E",
        "vectors":len(cases),"exact":len(cases)-len(mismatches),
        "mismatches":mismatches[:20],
        "scope":"Authenticates original .data static numeric penalty table and its score computation. Does not establish runtime course/mode table rewrites, source candidate trajectories or full AI."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(rep,indent=2)+"\n")
    print(f"Authentic original static CPU penalty table + score: {rep['exact']}/{len(cases)} exact")
    print("Original static table:",data)
    for m in mismatches[:5]:print("MISMATCH",m)
    if mismatches:raise SystemExit("original CPU static table/native score mismatch")
    print("PASS original static penalty table and candidate score source x86")
if __name__=="__main__":main()
