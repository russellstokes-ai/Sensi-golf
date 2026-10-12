#!/usr/bin/env python3
"""Authentic original Windows v1.014 CPU candidate score formula oracle.

Execute 0x40C9FD..0x40CA1E (the 14-byte record's contact-code-indexed
course penalty and source green bypass). Source executable SHA checked;
only ephemeral synthetic 32-WORD penalty tables are placed inside
Unicorn memory. No original game binary/asset is committed.

This does not verify trajectory simulation 0x40CB00, original table
initialization or original multi-mode computer strategy.
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
STOP=0x40ca1e
GREEN=0x41ee7a
PENALTIES=0x41fadf
def original(machine,row):
    distance,code,green,*table=row
    machine.mem_write(PENALTIES,struct.pack("<32H",*table))
    w16(machine,GREEN,green)
    w16(machine,PLAYER+0xA,code)
    w16(machine,PLAYER+0xC,0xcafe)
    machine.reg_write(UC_X86_REG_EAX,(0xa5a50000|distance))
    machine.reg_write(UC_X86_REG_EBP,PLAYER)
    machine.emu_start(START,STOP,count=40)
    return ru16(machine,PLAYER+0xC)
def vectors():
    rng=random.Random(0x40c9fd)
    cases=[]
    special=(0,1,2,7,8,9,10,15,30,31)
    greens=(0,1,2,65535)
    distances=(0,1,2,100,32767,32768,65534,65535)
    for code in range(32):
        for green in greens:
            for dist in distances:
                table=[rng.randrange(65536) for _ in range(32)]
                table[code]=rng.choice((0,1,100,32767,32768,65535))
                cases.append((dist,code,green,*table))
    for _ in range(7300):
        code=rng.choice(special+(rng.randrange(32),))
        green=rng.choice(greens)
        dist=rng.choice(distances+(rng.randrange(65536),))
        table=[rng.randrange(65536) for _ in range(32)]
        cases.append((dist,code,green,*table))
    return cases
def main():
    a=argparse.ArgumentParser()
    a.add_argument("--exe",type=Path,required=True)
    a.add_argument("--probe",type=Path,required=True)
    a.add_argument("-o","--output",type=Path,required=True)
    opt=a.parse_args()
    if hashlib.sha256(opt.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original game SHA256 mismatch")
    rows=vectors()
    process=subprocess.run([str(opt.probe)],
        input="".join(" ".join(map(str,row))+"\n" for row in rows),
        text=True,capture_output=True,check=True)
    native=[int(s) for s in process.stdout.splitlines()]
    if len(native)!=len(rows):
        raise RuntimeError("native CPU score count mismatch")
    uc,_=build_uc(opt.exe)
    mismatch=[]
    for row,result in zip(rows,native):
        expected=original(uc,row)
        if expected!=result:
            mismatch.append({"distance":row[0],"code":row[1],
                "green":row[2],"original":expected,
                "native":result,"penalty":row[3+row[1]]})
    rep={"schema":"pixtee.original-cpu-score-distance-penalty-x86.v1",
         "reference_sha256":REFERENCE_SHA256,
         "source_x86":"0x40C9FD..0x40CA1E",
         "records":len(rows),"exact":len(rows)-len(mismatch),
         "mismatches":mismatch[:25],
         "source_table":"Synthetic original-layout 32 WORD penalty table, injected only into emulator",
         "source_output":"signed collision status indexes source penalty WORD; green mode exactly 1 skips ADD AX,BX; final score is wrapping WORD",
         "not_proven":"Real course penalty-table initialization, complete original CPU simulation, 14-byte record generation and original AI shot-policy lifecycle."}
    opt.output.parent.mkdir(parents=True,exist_ok=True)
    opt.output.write_text(json.dumps(rep,indent=2)+"\n")
    print(f"Authentic original CPU score metric: {rep['exact']}/{len(rows)} exact")
    for m in mismatch[:8]:print("MISMATCH",m)
    if mismatch:raise SystemExit("source original CPU score formula mismatch")
    print("PASS original CPU candidate distance + terrain penalty scoring (source x86)")
if __name__=="__main__":main()
