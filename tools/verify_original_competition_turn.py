#!/usr/bin/env python3
"""Original Windows Sensible Golf v1.014 mode4/mode6 turn-owner state
0x40C098..0x40C16D, compared against C++ at exact branch boundary.
Does not prove full game-mode scoring or player ball/camera copying.
"""
import argparse,hashlib,json,struct,subprocess,sys
from itertools import product
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,ru16,w16,wu32
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EBX,UC_X86_REG_EBP,UC_X86_REG_ESI
MODE=0x42558e;ACTIVE=0x4255de;BALL0=0x4287d6;BALL1=0x428802
FLAG0=0x4255b2;FLAG1=0x4255b4;SLOTS=0x4255cc;PREV0=0x4255da;PREV1=0x4255dc
END=0x40c16d;SKIP=0x40c1b7

def original(uc,args):
    mode,side,d0,d1,f0,f1,p0,p1,a,b,c,d=args
    uc.mem_write(MODE,bytes([mode]))
    w16(uc,ACTIVE,side)
    wu32(uc,BALL0+0x18,d0)
    wu32(uc,BALL1+0x18,d1)
    w16(uc,FLAG0,f0)
    w16(uc,FLAG1,f1)
    uc.mem_write(SLOTS,bytes([a,b,c,d]))
    w16(uc,PREV0,p0)
    w16(uc,PREV1,p1)
    stop=[]
    def hook(machine,addr,size,user):
        if addr in (END,SKIP):
            stop.append(addr)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(0x40c098,0,count=400)
    finally:uc.hook_del(token)
    if not stop:raise RuntimeError("unexpected original match turn mode path")
    if stop[0]==SKIP:
        return [-1,-1,-1,ru16(uc,ACTIVE),ru16(uc,PREV0),ru16(uc,PREV1)]
    ebx=uc.reg_read(UC_X86_REG_EBX)
    ebp=uc.reg_read(UC_X86_REG_EBP)
    esi=uc.reg_read(UC_X86_REG_ESI)
    ball=0 if esi==BALL0 else (1 if esi==BALL1 else -2)
    return [ebx&0xffff,ebp&0xffff,ball,ru16(uc,ACTIVE),
            ru16(uc,PREV0),ru16(uc,PREV1)]

def cases():
    out=[]
    for mode,side,d0,d1,f0,f1,p0,p1,slots in product(
        (1,4,6,7),(0,1),(0,1),(0,1),(0,1),(0,1),
        (0,1,2),(0,1,2),((0,1,2,3),(2,3,0,1))):
        out.append((mode,side,d0,d1,f0,f1,p0,p1,*slots))
    return out

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("--probe",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    args=ap.parse_args()
    assert hashlib.sha256(args.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    rows=cases()
    p=subprocess.run([str(args.probe)],input="".join("%d %d %d %d %d %d %d %d %d %d %d %d\n"%x for x in rows),
                     text=True,capture_output=True,check=True)
    lines=p.stdout.strip().splitlines()
    assert len(rows)==len(lines)
    uc,_=build_uc(args.exe)
    misses=[]
    for row,line in zip(rows,lines):
        src=original(uc,row);native=list(map(int,line.split()))
        if src!=native:misses.append({"input":row,"original":src,"native":native})
    summary={"schema":"pixtee.original-comp-turn-mode4-6-v1",
       "original_sha256":REFERENCE_SHA256,"cases":len(rows),
       "exact_matches":len(rows)-len(misses),"mismatches":misses[:100],
       "scope":"mode4/6 side and slot decision only, NOT full match or season"}
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(summary,indent=2)+"\n")
    print(f"Original game mode-4/6 side/turn selection: {summary['exact_matches']}/{len(rows)}")
    for issue in misses[:10]:print("MISMATCH",issue)
    if misses:raise SystemExit("Competition mode original selection mismatch")
    print("PASS original x86 competition-side and player-slot branch")
if __name__=="__main__":main()
