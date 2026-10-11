#!/usr/bin/env python3
"""Execute actual original x86 code9 terrain event dispatch at 0x40A374.
Return original NONE/DEFLECT/SPECIAL-STOP branch without invoking its
visual/audio/resource helpers. C++ predicate must match all paths.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from itertools import product
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,BALL
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBX
START=0x40a374
TARGETS={0x40a43b:0,0x40a3ac:1,0x40a3e0:2}

def original(uc,case):
    tile,club,vertical,height=case
    uc.mem_write(PLAYER+0x28,struct.pack("<H",club))
    uc.mem_write(BALL+0x08,struct.pack("<i",vertical))
    uc.mem_write(BALL+0x14,struct.pack("<i",height))
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_EBX,tile)
    reached=[]
    def hook(machine,address,size,user):
        if address in TARGETS:
            reached.append(address)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=60)
    finally:uc.hook_del(token)
    if len(reached)!=1:
        raise RuntimeError(f"unclassified code9 original x86 branch: {case} -> {reached}")
    return TARGETS[reached[0]],hex(reached[0])
def cases():
    out=list(product((0,6,8,9,10,35,77),
                     (0,1,11,12,13,65535),
                     (-50000,-1,0,1,50000),
                     (-100000,-1,0,0x2ffff,0x30000,0x30001,
                      0x13ffff,0x140000,0x140001)))
    r=random.Random(1014)
    for i in range(2500):
        out.append((r.choice((9,9,9,10,0)),
                    r.choice((0,4,12,13)),
                    r.randint(-200000,200000),
                    r.randint(-100000,1500000)))
    return out
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    vectors=cases()
    response=subprocess.run([str(a.probe)],input="".join("%d %d %d %d\n"%row for row in vectors),
        text=True,capture_output=True,check=True)
    lines=response.stdout.strip().splitlines()
    assert len(lines)==len(vectors)
    uc,_=build_uc(a.exe)
    errors=[];visited={};matched=0
    for row,line in zip(vectors,lines):
        expected,target=original(uc,row)
        visited[target]=visited.get(target,0)+1
        actual=int(line)
        if expected!=actual:errors.append({"input":row,"source":expected,"native":actual,"branch":target})
        else:matched+=1
    report={"schema":"pixtee.original-code9-event-branch-x86.v1",
        "exe_sha256":REFERENCE_SHA256,"vectors":len(vectors),
        "exact_matches":matched,"source_branches":visited,
        "mismatches":errors[:60],
        "scope":"original code9 ternary event activation; NOT full event-11 timeout/restart/recovery, nor visual/audio"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original code9 event branch original x86 vs C++: {matched}/{len(vectors)}")
    print("source branches",visited)
    for issue in errors[:10]:print("MISMATCH",issue)
    if errors:raise SystemExit("original code9 event branch mismatched")
    print("PASS exact code9 None / Deflect / Stop classification")
if __name__=="__main__":main()
