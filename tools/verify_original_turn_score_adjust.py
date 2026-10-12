#!/usr/bin/env python3
"""Run original Sensible Golf v1.014 mode-1 turn/scoring correction x86
0x40C005..0x40C098 against recovered native C++ for live score states.
This is NOT an end-to-end 18-hole multiplayer/season rule simulation.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from itertools import product
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,ru16,ru32,w16,wu32
from unicorn import UC_HOOK_CODE
P=0x425fe2
B=0x4287d6
COUNT=0x429527
MODE=0x42558e
START=0x40c005
NO_CORRECTION=0x40c098
CORRECTION_EXIT=0x40bef4

def source(uc,case):
    n,rows=case
    uc.mem_write(P,b"\0"*(4*0x86))
    uc.mem_write(B,b"\0"*(4*0x2c))
    uc.mem_write(MODE,b"\x01")
    wu32(uc,COUNT,n)
    for i,(stroke,total,marker,dist) in enumerate(rows):
        w16(uc,P+0x86*i+0x52,stroke)
        w16(uc,P+0x86*i+0x56,total)
        w16(uc,P+0x86*i+0x5e,marker)
        wu32(uc,B+0x2c*i+0x18,dist)
    hit=[]
    def hook(machine,addr,size,user):
        if addr in (NO_CORRECTION,CORRECTION_EXIT):
            hit.append(addr)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=2000)
    finally:uc.hook_del(token)
    if not hit:raise RuntimeError("Original scoring correction exited other route")
    post=[(ru16(uc,P+0x86*i+0x52),ru16(uc,P+0x86*i+0x56)) for i in range(4)]
    changes=[i for i in range(n) if post[i]!=(rows[i][0],rows[i][1])]
    if len(changes)>1:raise AssertionError("Original adjusted >one player")
    return [changes[0] if changes else -1]+[x for pair in post for x in pair],hit[-1]

def test_cases():
    out=[]
    rng=random.Random(1014)
    for n in range(1,5):
        patterns=([0]*n,[1]*n,list(range(n)),[n-i for i in range(n)],
                  [2]*n,[0]+[7]*(n-1),[65535]*n,[0,65535]*2)
        for sc in patterns:
            sc=list(sc[:n])
            for marker in (0,1,3):
                for d in (0,1):
                    rows=[(v,(v*3+4)&65535,marker if i%2 else 0,d if i%2 else 0)
                          for i,v in enumerate(sc)]
                    rows.extend([(0,0,0,1)]*(4-n))
                    out.append((n,rows))
        for i in range(750):
            rows=[]
            for player in range(4):
                stroke=rng.choice([0,1,2,3,4,9,23,65535,32768,32767])
                total=rng.randrange(0,65536)
                marker=rng.choice([0,0,0,1,2])
                dist=rng.choice([0,0,0,1,12,65536])
                rows.append((stroke,total,marker,dist))
            out.append((n,rows))
    return out

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    cases=test_cases()
    inputs="".join(str(n)+" "+" ".join("%d %d %d %d"%row for row in rows)+"\n" for n,rows in cases)
    native=subprocess.run([str(a.probe)],input=inputs,text=True,capture_output=True,check=True)
    lines=native.stdout.strip().splitlines()
    assert len(lines)==len(cases)
    uc,_=build_uc(a.exe)
    errors=[];paths={}
    for case,line in zip(cases,lines):
        original,exit_va=source(uc,case)
        actual=list(map(int,line.split()))
        paths[hex(exit_va)]=paths.get(hex(exit_va),0)+1
        if actual!=original:errors.append({"input":case,"original":original,"native":actual})
    report={"schema":"pixtee.original-multiplayer-mode1-turn-score-correction-x86.v1",
        "original_executable_sha256":REFERENCE_SHA256,"vector_count":len(cases),
        "exact_matches":len(cases)-len(errors),"mismatches":errors[:50],
        "source_branch_exit_counts":paths,
        "scope":"original mode-1 turn score correction only, NOT tournament/multiplayer completeness"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original v1.014 mode-1 multiplayer score adjustment: {report['exact_matches']}/{len(cases)}")
    print("Source branch destinations",paths)
    for x in errors[:10]:print("MISMATCH",x)
    if errors:raise SystemExit("Original score adjustment parity failed")
    print("PASS original x86 multi-player turn correction function")
if __name__=="__main__":main()
