#!/usr/bin/env python3
"""Original v1.014 CPU simulated shot initial forces, source x86 oracle.

Exact CPU trial velocity setup 0x40CCA4..0x40CCC1 is source-authenticated
via Unicorn on the archived SHA-matched executable. No copyrighted code or
course is committed. This is NOT proof of complete CPU AI player decisions.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
  build_uc,REFERENCE_SHA256,PLAYER,BALL,wi32,w16,ri32)
START=0x40cca4
STOP=0x40ccc1
def original(uc,row):
    scale,v,h,power=row
    uc.mem_write(PLAYER,b"\0"*0x90)
    uc.mem_write(BALL,b"\0"*0x70)
    wi32(uc,PLAYER+0x32,scale)
    wi32(uc,PLAYER+0x2a,v)
    wi32(uc,PLAYER+0x2e,h)
    w16(uc,0x41d59e,power)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    reached=[]
    def hook(machine,address,size,user):
        if address==STOP:
            reached.append(address)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=30)
    finally:uc.hook_del(token)
    if reached!=[STOP]:
        raise RuntimeError("original CPU trajectory force source left region")
    return ri32(uc,BALL+8),ri32(uc,BALL+12)
def vectors():
    rng=random.Random(0x40cca4)
    signed=(-2147483648,-2147483647,-65536,-32768,-1,0,1,2,
            32767,65536,0x7ffffffe,0x7fffffff)
    ps=(0,1,2,3,12,127,255,256,511,65534,65535)
    rows=[]
    for scale in signed:
        for p in ps:
            for offset in (-2147483648,-1,0,2147483647):
                rows.append((scale,offset,offset,p))
    for _ in range(9000):
        rows.append((rng.choice(signed+(rng.randrange(-2**31,2**31),)),
                     rng.randrange(-2**31,2**31),
                     rng.randrange(-2**31,2**31),
                     rng.choice(ps+(rng.randrange(65536),))))
    return rows
def main():
    a=argparse.ArgumentParser()
    a.add_argument("--exe",type=Path,required=True)
    a.add_argument("--probe",type=Path,required=True)
    a.add_argument("-o","--output",type=Path,required=True)
    p=a.parse_args()
    if hashlib.sha256(p.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original Windows 1.014 executable SHA mismatch")
    rows=vectors()
    result=subprocess.run([str(p.probe)],
        input="".join(" ".join(map(str,r))+"\n" for r in rows),
        text=True,capture_output=True,check=True)
    native=result.stdout.splitlines()
    if len(native)!=len(rows):
        raise RuntimeError("incomplete native CPU candidate velocity output")
    uc,_=build_uc(p.exe)
    mismatches=[]
    for row,line in zip(rows,native):
        expected=original(uc,row)
        actual=tuple(map(int,line.split()))
        if actual!=expected:
            mismatches.append({"input":row,"original":expected,"native":actual})
    out={"schema":"pixtee.original-cpu-trial-force-setup.x86.v1",
      "source_sha256":REFERENCE_SHA256,
      "source_x86":"0x40CCA4..0x40CCC1",
      "inputs":"player +0x32 shot scale, +0x2A/+0x2E base forces, 0x41D59E captured power",
      "outputs":"ball +0x08/+0x0C signed32 wrapping",
      "cases":len(rows),"exact":len(rows)-len(mismatches),
      "mismatches":mismatches[:30],
      "not_proven":"Original complete candidate roll-out simulation 0x40CB00, metric computation, trial ranking, RNG ownership or club/power policy."}
    p.output.parent.mkdir(parents=True,exist_ok=True)
    p.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Authentic original CPU trial force: {out['exact']}/{len(rows)} exact")
    for m in mismatches[:5]: print("MISMATCH",m)
    if mismatches:raise SystemExit("source-machine CPU trial force mismatch")
    print("PASS original simulated CPU shot initial force source parity")
if __name__=="__main__":main()
