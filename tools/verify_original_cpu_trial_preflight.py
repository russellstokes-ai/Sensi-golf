#!/usr/bin/env python3
"""Original Windows 1.014 simulated CPU shot preflight (0x40CB40).

Exercises the authentic short x86 route from a completed original
map descriptor lookup through code9 descending contact and putter
zero/positive-force gating, stopping before any gravity/roll update.
No original copyrighted binaries are stored in source control.
"""
from __future__ import annotations
import argparse,hashlib,json,random,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBX
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,PLAYER,BALL,w16,wi32
)
ENTRY=0x40cb40
EXITS={0x40cb6c:0,0x40cbcd:1,0x40cc71:2,0x40cc49:3}
def source(uc,row):
    terrain,club,v,h,z=row
    uc.mem_write(PLAYER,b"\0"*0x80)
    uc.mem_write(BALL,b"\0"*0x80)
    w16(uc,PLAYER+0x28,club)
    wi32(uc,BALL+8,v);wi32(uc,BALL+12,h)
    wi32(uc,BALL+0x14,z)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_EBX,terrain)
    path=[]
    def stop(m,address,size,user):
        if address in EXITS:
            path.append(address);m.emu_stop()
    hook=uc.hook_add(UC_HOOK_CODE,stop)
    try:uc.emu_start(ENTRY,0,count=40)
    finally:uc.hook_del(hook)
    if len(path)!=1:
        raise RuntimeError(f"unexpected CPU source route {row}: {path}")
    return EXITS[path[0]]
def vectors():
    rng=random.Random(0x40cb40)
    forces=(-2147483648,-2097152,-1,0,1,10,0x2100,0x140000,
            0x140001,0x7fffffff)
    heights=(-2147483648,-1,0,0x13ffff,0x140000,0x140001,
             0x7fffffff)
    codes=(0,1,8,9,10,11,35,60,0xffff)
    clubs=(0,5,11,12,13,0xffff)
    rows=[]
    for terrain in codes:
        for club in clubs:
            for v in (-2147483648,-1,0,1,2147483647):
                for height in heights:
                    rows.append((terrain,club,v,rng.choice(forces),height))
    for _ in range(7500):
        rows.append((rng.choice(codes+(rng.randrange(65536),)),
                     rng.choice(clubs+(rng.randrange(65536),)),
                     rng.choice(forces+(rng.randrange(-2**31,2**31),)),
                     rng.choice(forces+(rng.randrange(-2**31,2**31),)),
                     rng.choice(heights+(rng.randrange(-2**31,2**31),))))
    return rows
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original game executable SHA mismatch")
    rows=vectors()
    proc=subprocess.run([str(a.probe)],
       input="".join(" ".join(map(str,x))+"\n" for x in rows),
       capture_output=True,text=True,check=True)
    native=[int(x) for x in proc.stdout.splitlines()]
    if len(native)!=len(rows):raise RuntimeError("CPU preflight input count mismatch")
    uc,_=build_uc(a.exe)
    mismatch=[];routes={k:0 for k in EXITS.values()}
    for row,out in zip(rows,native):
        expected=source(uc,row)
        routes[expected]+=1
        if out!=expected:mismatch.append({"input":row,"source":expected,"native":out})
    result={"schema":"pixtee.cpu-trial-preflight-original-windows-x86.v1",
       "source_sha256":REFERENCE_SHA256,"source_entry":hex(ENTRY),
       "source_exit_routes":{hex(k):v for k,v in EXITS.items()},
       "vectors":len(rows),"exact":len(rows)-len(mismatch),
       "source_route_histogram":routes,
       "mismatches":mismatch[:30],
       "limits":"This original 0x40CB40 branch is after a separately verified map lookup and before the simulator gravity step. Full original CPU candidate scoring is not verified."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(result,indent=2)+"\n")
    print(f"Original CPU candidate preflight routes: {result['exact']}/{len(rows)} exact")
    print("Source routes",routes)
    for m in mismatch[:7]:print("MISMATCH",m)
    if mismatch:raise SystemExit("original x86 CPU preflight/native mismatch")
    print("PASS original game code9 and putter candidate simulator route parity")
if __name__=="__main__":main()
