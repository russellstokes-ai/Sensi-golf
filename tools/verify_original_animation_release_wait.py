#!/usr/bin/env python3
"""Verify original animation signed commands 0x8004 and 0x8008.

Original jump table 0x41F89C was inspected from SHA-verified Windows
1.014 and opcode 0x8004 targets native source 0x40C573 (slot close),
0x8008 targets 0x40C58D (wait WORD). Test source-emulated command
execution itself, not a guessed callback. 0x8000 remains unimplemented.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_ESP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,BALL,STACK,w16,wu32,ru16,ru32
START=0x40C539
STOPS={4:0x40C58C,8:0x40C598}
def original(uc,row):
    opcode,delay,wait,offset,status,height,width,x,y=row
    script=bytearray(offset+10)
    struct.pack_into("<H",script,0,delay)
    struct.pack_into("<H",script,6+offset,0x8000+opcode)
    struct.pack_into("<H",script,8+offset,wait)
    uc.mem_write(BALL,bytes(script))
    uc.mem_write(PLAYER,b"\0"*0x80)
    w16(uc,PLAYER,status)
    w16(uc,PLAYER+2,x)
    w16(uc,PLAYER+4,y)
    w16(uc,PLAYER+6,height)
    w16(uc,PLAYER+8,width)
    w16(uc,PLAYER+0xA,0)
    wu32(uc,PLAYER+0xC,BALL)
    wu32(uc,PLAYER+0x10,offset)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    seen=[]
    def hook(m,address,size,user):
        if address==STOPS[opcode]:
            seen.append(address);m.emu_stop()
    t=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=90)
    finally:uc.hook_del(t)
    if seen!=[STOPS[opcode]]:
        raise RuntimeError(f"original opcode failed authentic JMP table {row}")
    return (1 if opcode==4 else 2,
       ru16(uc,PLAYER),ru16(uc,PLAYER+0xA),ru16(uc,PLAYER+6),
       ru32(uc,PLAYER+0x10),ru32(uc,PLAYER+0xC),
       ru16(uc,PLAYER+2),ru16(uc,PLAYER+4),ru16(uc,PLAYER+8))
def vectors():
    rng=random.Random(0x41f89c)
    rows=[]
    for opcode in (4,8):
        for delay in (0,1,4,100,65535):
          for wait in (0,1,4,100,65535):
           for offset in (0,2,4,64,128):
            rows.append((opcode,delay,wait,offset,
               rng.choice((0,1,5,0xfffe)),
               rng.randrange(65536),rng.randrange(65536),
               rng.randrange(65536),rng.randrange(65536)))
    for _ in range(5400):
        rows.append((rng.choice((4,8)),
          rng.randrange(65536),rng.randrange(65536),
          2*rng.randrange(0,65),rng.choice((0,1,5,0xfffe)),
          rng.randrange(65536),rng.randrange(65536),
          rng.randrange(65536),rng.randrange(65536)))
    return rows
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original exe SHA mismatch")
    cases=vectors()
    proc=subprocess.run([str(a.probe)],input="".join(
        " ".join(map(str,c))+"\n" for c in cases),
        text=True,capture_output=True,check=True)
    native=[tuple(map(int,line.split())) for line in proc.stdout.splitlines()]
    if len(native)!=len(cases):raise RuntimeError("native command count mismatch")
    uc,_=build_uc(a.exe)
    mismatches=[];by_command={4:0,8:0}
    for case,result in zip(cases,native):
        want=original(uc,case)
        by_command[case[0]]+=1
        if result!=want:mismatches.append({"input":case,"source":want,"native":result})
    out={"schema":"pixtee.original-animation-release-wait-source-x86.v1",
      "source_sha256":REFERENCE_SHA256,"source_start":"0x40C539",
      "commands":{"0x8004":"0x40C573, release slot",
                  "0x8008":"0x40C58D, read next WAIT word"},
      "cases":len(cases),"exact":len(cases)-len(mismatches),
      "by_command":by_command,"mismatches":mismatches[:30],
      "limits":"External callback opcode0x8000, unknown opcodes and original sprite/audio UI side effects remain unverified."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Original event 0x8004/0x8008 commands: {out['exact']}/{len(cases)} exact")
    print("Source opcode counts",by_command)
    for v in mismatches[:8]:print("MISMATCH",v)
    if mismatches:raise SystemExit("original 0x8004/0x8008 command parity failure")
    print("PASS original animation source release and WAIT command effects")
if __name__=="__main__":main()
