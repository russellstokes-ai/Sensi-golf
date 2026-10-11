#!/usr/bin/env python3
"""Source-executable original 0x40AACF -> 0x40A871 safe-anchor restore.

The original world OOB acknowledgement bypasses bounds testing, and
ALWAYS chooses the safe anchor, even for the exact world-boundary
coordinate which the original separate 0x40A823 comparator treats as
inside. Original non-state UI/refresh callbacks skipped in emulator.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EIP,UC_X86_REG_ESP,UC_X86_REG_ESI,UC_X86_REG_EDI
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,BALL,PLAYER,STACK,ri32,ru16,w16,wu32,wi32
)
from original_v1014_hazard_recovery_oracle import HELPERS
from original_v1014_course_lookup_oracle import (
    GREEN_MODE_VA,MAP_X_EXTENT_VA,MAP_Y_EXTENT_VA
)
START=0x40AACF
END=0x40A8D6

def rows():
    rng=random.Random(0x40AACF)
    wx=34*16
    wy=114*8
    cases=[]
    for x in (-32768,-1,wx,wx+1,wx+500,32767):
      for y in (-32768,-1,100,wy,wy+1,32767):
       for mode5a in (0,4,5,0x7fff,0x8000,0xffff):
        sx=rng.randrange(0,wx-15)
        sy=rng.randrange(0,wy-15)
        cases.append((x<<16,y<<16,sx<<16,sy<<16,
                      wx-0x100,wy-0xd0,
                      mode5a,rng.randrange(65536),rng.randrange(65536)))
    for _ in range(4200):
      x=rng.choice((-32768,-1,wx,wx+1,wx+500,32767))
      y=rng.choice((-32768,-1,100,wy,wy+1,32767))
      sx=rng.randrange(0,wx-15)
      sy=rng.randrange(0,wy-15)
      cases.append(((x<<16)|rng.randrange(65536),
                    (y<<16)|rng.randrange(65536),
                    (sx<<16)|rng.randrange(65536),
                    (sy<<16)|rng.randrange(65536),
                    wx-0x100,wy-0xd0,
                    rng.choice((0,3,4,5,7,0x7fff,0x8000,0xffff)),
                    rng.randrange(65536),rng.randrange(65536)))
    return cases

def original(uc,row):
    x,y,sx,sy,xext,yext,m5a,m5c,m62=row
    uc.mem_write(PLAYER,b"\x00"*0x100)
    uc.mem_write(BALL,b"\x00"*0x100)
    wi32(uc,BALL,x);wi32(uc,BALL+4,y)
    wi32(uc,BALL+0x14,0x123456)
    w16(uc,BALL+0x1c,100)
    w16(uc,PLAYER,4)
    w16(uc,PLAYER+0x5a,m5a)
    w16(uc,PLAYER+0x5c,m5c)
    w16(uc,PLAYER+0x62,m62)
    wu32(uc,PLAYER+0x64,sx)
    wu32(uc,PLAYER+0x68,sy)
    w16(uc,GREEN_MODE_VA,0)
    wu32(uc,MAP_X_EXTENT_VA,xext)
    wu32(uc,MAP_Y_EXTENT_VA,yext)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    hits=[]
    def hook(machine,address,size,user):
        if address==END:
            hits.append(address);machine.emu_stop()
        elif address in HELPERS:
            sp=machine.reg_read(UC_X86_REG_ESP)
            ret=struct.unpack("<I",bytes(machine.mem_read(sp,4)))[0]
            machine.reg_write(UC_X86_REG_ESP,sp+4)
            machine.reg_write(UC_X86_REG_EIP,ret)
    tok=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=2000)
    finally:uc.hook_del(tok)
    if hits!=[END]:
        raise RuntimeError(f"original bit2 OOB source did not return to 0x40A8D6: {row}")
    return (
      ri32(uc,BALL),ri32(uc,BALL+4),
      ri32(uc,PLAYER+6),ri32(uc,PLAYER+0x0a),
      ri32(uc,PLAYER+0x4a),ri32(uc,PLAYER+0x4e),
      ri32(uc,BALL+0x14),ru16(uc,BALL+0x1c),
      ru16(uc,PLAYER),ru16(uc,PLAYER+0x5c),ru16(uc,PLAYER+0x62)
    )

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original executable hash mismatch")
    cases=rows()
    native=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,r))+"\n" for r in cases),
        text=True,capture_output=True,check=True)
    lines=native.stdout.splitlines()
    if len(lines)!=len(cases):raise RuntimeError("native case count mismatch")
    uc,_=build_uc(a.exe)
    mismatches=[]
    for case,line in zip(cases,lines):
        expected=original(uc,case)
        actual=tuple(map(int,line.split()))
        if expected!=actual:
            mismatches.append({"input":case,"original":expected,"native":actual})
    report={"schema":"pixtee.original-out-of-bounds-world-bit2-restore.x86.v1",
      "original_sha256":REFERENCE_SHA256,"vectors":len(cases),
      "exact":len(cases)-len(mismatches),
      "boundary_equality_cases":"included",
      "source":"0x40AACF -> 0x40A871 -> 0x40A8D6",
      "stubs":"Only unrelated source refresh/UI helper CALL bodies",
      "mismatches":mismatches[:20],
      "scope":"OOB post-event source branch and restored fields; original event UI timing, penalty display and AI/multiplayer scheduling not asserted"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original world OOB safe-anchor restore: {report['exact']}/{len(cases)} exact")
    for mis in mismatches[:7]:print("MISMATCH",mis)
    if mismatches:raise SystemExit("Original world OOB restore parity mismatch")
    print("PASS authentic source unconditional out-of-bounds safe-anchor and 60 pause")
if __name__=="__main__":main()
