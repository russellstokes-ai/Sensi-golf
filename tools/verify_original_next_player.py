#!/usr/bin/env python3
"""Original v1.014 player selection x86 vs recovered native function.
Verifies both original metric and forced-holder branches; does not claim
complete multiplayer gameplay, scoring, or AI.
"""
import argparse,hashlib,json,struct,subprocess,sys
from pathlib import Path
from itertools import product
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256
from unicorn.x86_const import UC_X86_REG_EBX

P=0x425fe2
S=0x4287d6
COUNT=0x429527
FORCED=0x41f885

def original(uc,count,forced,rows):
    uc.mem_write(COUNT,struct.pack("<H",count))
    uc.mem_write(FORCED,bytes([forced]))
    for i,(identity,excluded,score) in enumerate(rows):
        uc.mem_write(P+i*0x86+0x6e,bytes([identity]))
        uc.mem_write(P+i*0x86+0x5e,struct.pack("<H",excluded))
        uc.mem_write(S+i*0x2c+0x18,struct.pack("<I",score))
    start=0x40c1b7 if forced else 0x40c1f1
    uc.emu_start(start,0x40c22f,count=300)
    ebx=uc.reg_read(UC_X86_REG_EBX)
    assert P<=ebx<P+4*0x86 and (ebx-P)%0x86==0,hex(ebx)
    return (ebx-P)//0x86

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    uc,_=build_uc(a.exe)
    cases=[]
    for count in range(1,5):
        for metric in product((0,3,17),repeat=count):
            for exclude in product((0,1),repeat=count):
                rows=[(i+1,exclude[i],metric[i]) for i in range(count)]
                rows.extend((i+1,0,0) for i in range(count,4))
                cases.append((count,0,rows))
        # Include special-holder branch for every player identity.
        for holder in range(1,count+1):
            rows=[(i+1,i%2,17+i) for i in range(4)]
            cases.append((count,holder,rows))
    assert len(cases)>1500
    probe_input="".join(f"{n} {forced} "+" ".join(f"{identity} {ex} {metric}" for identity,ex,metric in rows)+"\n"
                        for n,forced,rows in cases)
    native=subprocess.run([str(a.probe)],input=probe_input,text=True,
                          capture_output=True,check=True)
    answers=native.stdout.strip().splitlines()
    assert len(answers)==len(cases),(len(answers),len(cases))
    mismatches=[]
    for i,((n,forced,rows),line) in enumerate(zip(cases,answers)):
        source=original(uc,n,forced,rows)
        observed=int(line)
        if source!=observed:
            mismatches.append({"index":i,"count":n,"forced":forced,
                               "rows":rows,"original":source,"native":observed})
    report={"schema":"pixtee.original-win1014-turn-selector-oracle.v1",
            "source_exe_sha256":REFERENCE_SHA256,"test_vectors":len(cases),
            "exact_matches":len(cases)-len(mismatches),
            "mismatches":mismatches[:100],
            "scope":"original decision 0x40c1b7..0x40c22f only; not full multiplayer or mode rules"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original x86 next player selection: {report['exact_matches']}/{len(cases)} matches")
    for m in mismatches[:8]:print("MISMATCH",m)
    if mismatches:raise SystemExit("Original next-player parity failed")
    print("PASS actual original Windows game selection branch zero-tolerance")
if __name__=="__main__": main()
