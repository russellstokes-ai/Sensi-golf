#!/usr/bin/env python3
"""Original source 0x409535 -> 0x40A318 world OOB penalty parity.

Authentic Windows v1.014 executable against genuine first-hole MAPM/MAPI
course assets ONLY in ephemeral CI. Verifies the SOURCE coordinate-lookup
failure -> player flags/2 counters/100-pause and 0x40CD00 route. It does
NOT implement or assert downstream original 0x40CD00/UI/turn scheduling,
nor treats descriptor22 in-map OOB tiles as the world boundary path.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (
    UC_X86_REG_EAX,UC_X86_REG_EDI,UC_X86_REG_ESI,UC_X86_REG_ESP
)
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,BALL,PLAYER,STACK,w16,wu32,wi32,ru16
)
import verify_original_mapped_terminal_pipeline as mapped
from original_v1014_course_lookup_oracle import (
    MAPBUF,DESC,MASK,MAP_ROW_STRIDE_VA,MAP_X_EXTENT_VA,MAP_Y_EXTENT_VA
)

START=0x40A2F7
OOB_EXIT=0x40CD00
VALID_EXIT=0x40A360
SLOPE_CALL=0x40A311
DISPLAY_CALL=0x40A339
GAME_GATE=0x41F883
HEIGHTS=[0,10,20,100,32767,0xffff]
COUNTERS=[0,1,2,10,99,100,254,255,65534,65535]

def vectors(width,height):
    rng=random.Random(0x40A318)
    xs=[-32768,-1024,-2,-1,0,1,2,15,16,
        width*16-2,width*16-1,width*16,width*16+1,
        width*16+100,32767]
    ys=[-32768,-1024,-2,-1,0,1,2,7,8,
        height*8-2,height*8-1,height*8,height*8+1,32767]
    rows=[]
    def draw(x,y):
        return (x<<16,y<<16,
                width*16-0x100,height*8-0xd0,
                rng.choice((0,1,2,3,4,0x8000,65535)),
                rng.choice(COUNTERS),rng.choice(COUNTERS),
                rng.choice(HEIGHTS),rng.choice((0,1,7,65535)))
    for x in xs:
        for y in ys:rows.append(draw(x,y))
    for _ in range(4800):
        xsamp=rng.choice((-32768,rng.randint(-400,600),
                         rng.randint(0,width*16-1),width*16,
                         width*16+rng.randint(0,1000),32767))
        ysamp=rng.choice((-32768,rng.randint(-400,600),
                         rng.randint(0,height*8-1),height*8,
                         height*8+rng.randint(0,1000),32767))
        rows.append(draw(xsamp,ysamp))
    return rows

def original(uc,row):
    x,y,xextent,yextent,flags,hole,total,pause,gate=row
    uc.mem_write(PLAYER,b"\x00"*0x100)
    uc.mem_write(BALL,b"\x00"*0x100)
    w16(uc,PLAYER,flags)
    w16(uc,PLAYER+0x52,hole)
    w16(uc,PLAYER+0x56,total)
    wi32(uc,BALL,x)
    wi32(uc,BALL+4,y)
    w16(uc,BALL+0x1c,pause)
    w16(uc,GAME_GATE,gate)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    seen=[]
    def hook(machine,address,size,user):
        if address in (OOB_EXIT,VALID_EXIT):
            seen.append(address);machine.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=1100)
    finally:uc.hook_del(h)
    if len(seen)!=1:
        raise RuntimeError(f"original world OOB unverified exit at {uc.reg_read(UC_X86_REG_EAX):#x}: {row}")
    outside=seen[0]==OOB_EXIT
    out=(int(outside),-1 if outside else 0,
         ru16(uc,PLAYER),ru16(uc,PLAYER+0x52),
         ru16(uc,PLAYER+0x56),ru16(uc,BALL+0x1c),
         ru16(uc,GAME_GATE),int(outside))
    return out,hex(seen[0])

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--mapm",required=True,type=Path)
    p.add_argument("--descriptor-bank",required=True,type=Path)
    p.add_argument("--selector-bank",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong original Windows v1.014 executable")
    course=a.mapm.read_bytes()
    width,height=struct.unpack_from(">HH",course,0x54)
    cases=vectors(width,height)
    native=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,row))+"\n" for row in cases),
        text=True,capture_output=True,check=True)
    lines=native.stdout.splitlines()
    if len(lines)!=len(cases):
        raise RuntimeError(f"native OOB source result count {len(lines)}/{len(cases)}")
    uc,_=build_uc(a.exe)
    mapped.setup_original(uc)
    uc.mem_write(MAPBUF,course)
    uc.mem_write(DESC,a.descriptor_bank.read_bytes())
    uc.mem_write(MASK,a.selector_bank.read_bytes())
    wu32(uc,MAP_ROW_STRIDE_VA,width*2)
    wu32(uc,MAP_X_EXTENT_VA,width*16-0x100)
    wu32(uc,MAP_Y_EXTENT_VA,height*8-0xd0)
    for addr in (SLOPE_CALL,DISPLAY_CALL):
        raw=bytes(uc.mem_read(addr,5))
        if raw[0]!=0xe8:
            raise RuntimeError(f"unexpected original helper call at {addr:#x}: {raw.hex()}")
        uc.mem_write(addr,b"\x90"*5)
    mismatch=[];routes={};penalties=0
    for row,line in zip(cases,lines):
        expected,dest=original(uc,row)
        got=tuple(int(s) for s in line.split())
        routes[dest]=routes.get(dest,0)+1
        penalties+=expected[0]
        if got!=expected:
            mismatch.append({"input":row,"original":expected,
                             "native":got,"route":dest})
    report={
      "schema":"pixtee.original-world-boundary-source-machine.v1",
      "original_sha256":REFERENCE_SHA256,
      "real_course":"MAPM01 plus MAPI01/02; only private CI, not committed",
      "world_dimensions":[width*16,height*8],
      "cases":len(cases),"exact_matches":len(cases)-len(mismatch),
      "source_x86_routes":routes,"penalty_branches":penalties,
      "x86_source":"0x409535 (negative AX) -> 0x40A318..0x40A34C (two player counters + bit2 + 100 pause)",
      "original_helper_calls_stubbed_in_emulator_only":[hex(SLOPE_CALL),hex(DISPLAY_CALL)],
      "mismatches":mismatch[:25],
      "scope":"Source world bounds and immediate 0x40CD00 handoff ONLY; no downstream OOB animation, position reset or multiplayer score correction"
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original world OOB boundary -> penalty route: {report['exact_matches']}/{len(cases)} exact")
    print("Original world OOB penalty:",penalties,"of",len(cases),"source routes",routes)
    for m in mismatch[:6]:print("MISMATCH",m)
    if mismatch:raise SystemExit("Original source world out-of-bounds parity mismatch")
    print("PASS exact original source map OOB branch and counter penalties")
if __name__=="__main__":main()
