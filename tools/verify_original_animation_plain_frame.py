#!/usr/bin/env python3
"""Source-authenticate original non-command animation script frames.

Original v1.014 0x40C539..0x40C559 executes the real original
scene-event script decoder on synthetic, bounded asset blobs in
ephemeral original-machine memory. Negative signed WORD opcode branch
(0x40C55A) deliberately excluded and stays an open fidelity gate.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,BALL,w16,wu32,ru16,ru32
START=0x40c539
STOP=0x40c559
def original(uc,row):
    delay,frame,offset,height,width=row
    script=bytearray(offset+8)
    struct.pack_into("<H",script,0,delay)
    struct.pack_into("<H",script,6+offset,frame)
    uc.mem_write(BALL,bytes(script))
    uc.mem_write(PLAYER,b"\0"*0x40)
    w16(uc,PLAYER,0)
    w16(uc,PLAYER+6,height)
    w16(uc,PLAYER+8,width)
    w16(uc,PLAYER+0xa,0)
    wu32(uc,PLAYER+0xc,BALL)
    wu32(uc,PLAYER+0x10,offset)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    hit=[]
    def hook(machine,address,size,user):
        if address==STOP:
            hit.append(address);machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=35)
    finally:uc.hook_del(token)
    if hit!=[STOP]:
        raise RuntimeError(f"source script did not return plain frame {row}")
    return (ru16(uc,PLAYER+6),ru16(uc,PLAYER+0xa),
            ru16(uc,PLAYER+6),ru16(uc,PLAYER+0xa),
            ru32(uc,PLAYER+0x10),ru16(uc,PLAYER+8))
def cases():
    rng=random.Random(0x40c539)
    rows=[]
    for delay in (0,1,4,100,255,256,65535):
      for frame in (0,1,7,256,0x7ffe,0x7fff):
        for offset in (0,2,4,6,30,126):
            rows.append((delay,frame,offset,
                rng.randrange(65536),rng.randrange(65536)))
    for _ in range(5600):
        rows.append((rng.randrange(65536),rng.randrange(32768),
                     rng.randrange(0,65)*2,
                     rng.randrange(65536),rng.randrange(65536)))
    return rows
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original Windows v1.014 SHA mismatch")
    rows=cases()
    proc=subprocess.run([str(a.probe)],input="".join(
        " ".join(map(str,x))+"\n" for x in rows),
        text=True,capture_output=True,check=True)
    native=[tuple(map(int,line.split())) for line in proc.stdout.splitlines()]
    if len(native)!=len(rows):
        raise RuntimeError("native animation source frame case count mismatch")
    uc,_=build_uc(a.exe)
    mismatch=[]
    for row,out in zip(rows,native):
        expected=original(uc,row)
        if out!=expected:
            mismatch.append({"case":row,"x86":expected,"native":out})
    result={
      "schema":"pixtee.original-noncommand-animation-script-frame-x86.v1",
      "source_sha256":REFERENCE_SHA256,
      "source":"0x40C539..0x40C559",
      "cases":len(rows),"exact":len(rows)-len(mismatch),
      "mismatches":mismatch[:30],
      "scope":"Original nonnegative script frame/delay/offset only. Negative opcode dispatcher at 0x40C55A, frame rendering and time-to-display not recovered."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(result,indent=2)+"\n")
    print(f"Original animation script plain frame: {result['exact']}/{len(rows)} exact")
    for item in mismatch[:6]:print("MISMATCH",item)
    if mismatch:raise SystemExit("authentic source plain animation frame mismatch")
    print("PASS authentic original animation script frame progression")
if __name__=="__main__":main()
