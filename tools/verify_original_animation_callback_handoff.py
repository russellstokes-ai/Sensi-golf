#!/usr/bin/env python3
"""Authenticate original animation script 0x8000 host-callback handoff.

Original Windows 1.014 x86 0x40C539 -> 0x40C56A..0x40C572.
The callback is a TEST-ONLY RET STUB in private Unicorn memory.
Compare the source's final script slot state against native C++ that
holds an opaque, NON-EXECUTABLE token and applies its state only after
the host acknowledges the callback. This does NOT recover any original
real callback's gameplay/UI/audio effects or wall clock scheduling.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_ESP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,PLAYER,BALL,STACK,
    w16,wu32,ru16,ru32)
ENTRY=0x40C539
STOP=0x40C572
STUB=PLAYER+0x200

def source(uc,row):
    delay,offset,status,x,y,height,width=row
    uc.mem_write(PLAYER,bytes(0x80))
    uc.mem_write(STUB,b"\xb8\x78\x56\x34\x12\xc3")
    script=bytearray(offset+12)
    struct.pack_into("<H",script,0,delay)
    struct.pack_into("<H",script,offset+6,0x8000)
    struct.pack_into("<I",script,offset+8,STUB)
    uc.mem_write(BALL,bytes(script))
    for pos,val in ((0,status),(2,x),(4,y),(6,height),(8,width),(10,0)):
        w16(uc,PLAYER+pos,val)
    wu32(uc,PLAYER+12,BALL)
    wu32(uc,PLAYER+16,offset)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    seen={"stub":0,"stop":0}
    def hook(machine,address,size,user):
        if address==STUB:
            seen["stub"]+=1
        if address==STOP:
            seen["stop"]+=1
            machine.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(ENTRY,0,count=200)
    finally:
        uc.hook_del(h)
    if seen!={"stub":1,"stop":1}:
        raise RuntimeError(f"original 0x8000 callback route not verified: {row} {seen}")
    return (ru16(uc,PLAYER),ru16(uc,PLAYER+10),
            ru16(uc,PLAYER+6),ru32(uc,PLAYER+16),
            ru32(uc,PLAYER+12),ru16(uc,PLAYER+2),
            ru16(uc,PLAYER+4),ru16(uc,PLAYER+8),STUB)

def cases():
    rng=random.Random(0x40c56a)
    rows=[]
    for delay in (0,1,4,100,255,65535):
      for offset in (0,2,4,14,64,128):
       for status in (0,1,3,0xfffe):
        for height in (0,1,0xfffe,0xffff):
            rows.append((delay,offset,status,
                         rng.randrange(65536),rng.randrange(65536),
                         height,rng.randrange(65536)))
    for _ in range(3000):
        rows.append((rng.randrange(65536),
           rng.randrange(65)*2,rng.choice((0,1,4,0xfffe)),
           rng.randrange(65536),rng.randrange(65536),
           rng.randrange(65536),rng.randrange(65536)))
    return rows

def main():
    a=argparse.ArgumentParser()
    a.add_argument("--exe",type=Path,required=True)
    a.add_argument("--probe",type=Path,required=True)
    a.add_argument("-o","--output",type=Path,required=True)
    p=a.parse_args()
    if hashlib.sha256(p.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original source binary SHA mismatch")
    rows=cases()
    result=subprocess.run([str(p.probe)],
       input="".join(" ".join(map(str,r))+"\n" for r in rows),
       text=True,capture_output=True,check=True)
    native=[tuple(map(int,line.split())) for line in result.stdout.splitlines()]
    if len(native)!=len(rows):raise RuntimeError("missing native callback outputs")
    uc,_=build_uc(p.exe)
    mismatches=[]
    for idx,(r,got) in enumerate(zip(rows,native)):
        original=source(uc,r)
        if original!=got:
            mismatches.append({"case":idx,"input":r,"source":original,"native":got})
    report={
       "schema":"pixtee.original-animation-0x8000-opaque-host-callback-x86.v1",
       "original_sha256":REFERENCE_SHA256,
       "source_entry":"0x40C539",
       "source_handler":"0x40C56A..0x40C572",
       "scoped_cases":len(rows),
       "exact":len(rows)-len(mismatches),
       "callback_stub_executions":len(rows),
       "mismatches":mismatches[:16],
       "scope":"Original x86 command-payload dispatch, callback invocation and post-RET slot state compared with host-ACK-native handoff; original external function side effects never executed or claimed."
    }
    p.output.parent.mkdir(parents=True,exist_ok=True)
    p.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original 0x8000 callback handoff: {report['exact']}/{len(rows)} x86 state matches")
    if mismatches:
        for m in mismatches[:4]:print("MISMATCH",m)
        raise SystemExit("original opcode0 callback/native parity failure")
    print("PASS real source CALL through script DWORD; native host token never executed")
if __name__=="__main__":main()
