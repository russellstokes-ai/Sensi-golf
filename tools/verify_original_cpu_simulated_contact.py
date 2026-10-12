#!/usr/bin/env python3
"""Original CPU trajectory simulation gravity, stop and near-cup lip fork.

Executes hash-authenticated original v1.014 x86 at 0x40CB6C until
0x40CBC7 (continue), 0x40CBC3 (stop), or 0x40CBA7 (original PRNG
would be called next). The model NEVER fabricates random outcomes.
Scope is a source-scoped CPU simulation fragment, not the complete AI.
"""
from __future__ import annotations
import argparse,hashlib,json,random,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EDI
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,BALL,wi32,ri32)
START=0x40cb6c
EXIT={0x40cbc7:0,0x40cbc3:1,0x40cba7:2}
def original(uc,row):
    v,h,z,d=row
    uc.mem_write(BALL,b"\x00"*0x100)
    for addr,value in ((8,v),(12,h),(0x14,z),(0x18,d)):
        wi32(uc,BALL+addr,value)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    reached=[]
    def hook(m,addr,size,user):
        if addr in EXIT:
            reached.append(addr)
            m.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=80)
    finally:uc.hook_del(token)
    if len(reached)!=1:
        raise RuntimeError(f"unproven original candidate return {row}:{reached}")
    return (EXIT[reached[0]],ri32(uc,BALL+8),ri32(uc,BALL+12),
            ri32(uc,BALL+0x14),ri32(uc,BALL+0x18)),hex(reached[0])
def cases():
    rng=random.Random(0x40cb6c)
    edge=(-2147483648,-2147483647,-0x2100,-1,0,1,
          0x2000,0x2100,0x32,0x33,0xcfff,0xd000,0xd001,
          0x7fffffff)
    rows=[]
    for v in edge:
        for z in edge:
            for d in (0,1,-1,50,65536):
                for h in (0,0xcfff,0xd000,0xd001):
                    rows.append((v,h,z,d))
    for _ in range(9000):
        rows.append((rng.choice(edge+(rng.randrange(-2**31,2**31),)),
                     rng.choice(edge+(rng.randrange(-2**31,2**31),)),
                     rng.choice(edge+(rng.randrange(-2**31,2**31),)),
                     rng.choice((0,1,-1,rng.randrange(-2**31,2**31)))))
    return rows
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original source executable checksum mismatch")
    rows=cases()
    child=subprocess.run([str(a.probe)],input="".join(
        " ".join(map(str,row))+"\n" for row in rows),
        text=True,capture_output=True,check=True)
    native=child.stdout.splitlines()
    if len(native)!=len(rows):
        raise RuntimeError(f"missing source CPU probe outputs: {len(native)}/{len(rows)}")
    uc,_=build_uc(a.exe)
    mismatch=[];branches={}
    for row,line in zip(rows,native):
        original_value,route=original(uc,row)
        branches[route]=branches.get(route,0)+1
        actual=tuple(int(s) for s in line.split())
        if original_value!=actual:
            mismatch.append({"input":row,"original":original_value,
                             "native":actual,"x86_route":route})
    out={"schema":"pixtee.original-cpu-trial-gravity-and-lip-source.x86.v1",
      "source_sha256":REFERENCE_SHA256,"source_start":"0x40CB6C",
      "end_reasons":{"continue":"0x40CBC7","stop":"0x40CBC3",
                     "needs_prng":"0x40CBA7"},
      "original_routes":branches,"cases":len(rows),
      "exact":len(rows)-len(mismatch),"mismatches":mismatch[:32],
      "scope":"Original computer trial gravity and close-cup precontact; ball-map lookup, PRNG, complete CPU trajectory, candidate score and policy are unverified."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Original simulated CPU contact: {out['exact']}/{len(rows)} exact")
    print("Source branches:",branches)
    for x in mismatch[:6]:print("MISMATCH",x)
    if mismatch:raise SystemExit("original CPU simulation contact mismatch")
    print("PASS original simulated shot gravity, zero-distance stop and lip request")
if __name__=="__main__":main()
