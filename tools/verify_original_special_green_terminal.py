#!/usr/bin/env python3
"""Test real x86 v1.014 club12 terrain 8/9/10 branch state vs C++.
Runs complete original terminal branch until original transition, skipping
visual/audio event handlers but observing the ORIGINAL emitted event ID.
Does not prove the subsequent 100-tick UI/game-mode continuation.
"""
import argparse,hashlib,json,struct,subprocess,sys
from itertools import product
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,BALL,PLAYER,STACK
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EIP,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBX,UC_X86_REG_EDX,UC_X86_REG_ESP

START=0x40a5a2
VISUAL=0x40c4c0
EVENT=0x403ef2
SPECIAL=0x40cd00
HOLED=0x40cd25
GATE=0x41d64b
TERMINAL=0x41f883
MODE=0x42558e
def w16(uc,a,v):uc.mem_write(a,struct.pack("<H",v&65535))
def r16(uc,a):return struct.unpack("<H",bytes(uc.mem_read(a,2)))[0]
def skip(machine):
    sp=machine.reg_read(UC_X86_REG_ESP)
    back=struct.unpack("<I",bytes(machine.mem_read(sp,4)))[0]
    machine.reg_write(UC_X86_REG_ESP,sp+4)
    machine.reg_write(UC_X86_REG_EIP,back)
def original(uc,case):
    code,mode,flags,ct52,ct56,gate=case
    uc.mem_write(PLAYER,b"\0"*0x100)
    uc.mem_write(BALL,b"\0"*0x100)
    w16(uc,PLAYER,flags)
    w16(uc,PLAYER+0x28,12)
    w16(uc,PLAYER+0x52,ct52)
    w16(uc,PLAYER+0x56,ct56)
    w16(uc,PLAYER+0x5a,mode)
    w16(uc,BALL+0x1c,0)
    w16(uc,TERMINAL,0)
    uc.mem_write(GATE,bytes([gate]))
    uc.mem_write(MODE,b"\0")
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_EBX,code)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    events=[];stop=[]
    def hook(machine,a,size,user):
        if a==VISUAL:skip(machine)
        elif a==EVENT:
            events.append(machine.reg_read(UC_X86_REG_EDX)&0xffffffff)
            skip(machine)
        elif a in (SPECIAL,HOLED):
            stop.append(a);machine.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(START,0,count=6000)
    except Exception as exc:
        raise RuntimeError(f"Original terminal x86 failure input={case} EIP={uc.reg_read(UC_X86_REG_EIP):#x}, events={events}: {exc}") from exc
    finally:uc.hook_del(h)
    if len(stop)!=1:raise RuntimeError("unrecognized original terminal "+str(case)+" "+str(stop))
    return [1,r16(uc,PLAYER),r16(uc,PLAYER+0x52),r16(uc,PLAYER+0x56),
            r16(uc,BALL+0x1c),r16(uc,TERMINAL),
            bytes(uc.mem_read(GATE,1))[0],
            events[-1] if events else 65535,2 if stop[0]==SPECIAL else 1]
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    uc,_=build_uc(a.exe)
    descriptor_for_code={}
    for tile in range(77):
        try:
            pointer=struct.unpack("<I",bytes(uc.mem_read(0x41dff7+tile*4,4)))[0]
            descriptor=struct.unpack("<H",bytes(uc.mem_read(pointer,2)))[0]
        except Exception:continue
        if descriptor in (8,9,10) and descriptor not in descriptor_for_code:
            descriptor_for_code[descriptor]=tile
    print("Original descriptor indexes for green terminal codes:",descriptor_for_code)
    if set(descriptor_for_code)!={8,9,10}:
        raise RuntimeError("Cannot prove all code8,9,10 original descriptor indexes")
    inputs=list(product((8,9,10), (0,1,3,4,12,65535),
                        (0,1,4,7,65535),(0,1,7,65535),
                        (0,11,65535),(0,1)))
    # The actual x86 entry takes a descriptor TABLE INDEX in BX, then
    # dereferences it to the original terrain code. Native takes the code.
    actual_inputs=[(c,m,f,s,t,g) for c,m,f,s,t,g in inputs]
    native=subprocess.run([str(a.probe)],input="".join("%d %d %d %d %d %d\n"%v for v in actual_inputs),
                         text=True,capture_output=True,check=True)
    lines=native.stdout.strip().splitlines()
    assert len(lines)==len(inputs)
    uc,_=build_uc(a.exe)
    bad=[];equal=0
    for row,line in zip(inputs,lines):
        src_code,src_mode,src_flags,src_52,src_56,src_gate=row
        source=original(uc,(descriptor_for_code[src_code],src_mode,src_flags,src_52,src_56,src_gate))
        actual=list(map(int,line.split()))
        if source==actual:equal+=1
        else:bad.append({"original_input":row,"original":source,"native":actual})
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps({"schema":"pixtee.original-code8-9-10-clip.v1",
        "sha256":REFERENCE_SHA256,"cases":len(inputs),"exact_matches":equal,
        "mismatches":bad[:70],"scope":"terminal branch and counters only, not full original post-pause continuation"},indent=2)+"\n")
    print(f"Original source terrain 8/9/10 putter terminal: {equal}/{len(inputs)}")
    for x in bad[:10]:print("MISMATCH",x)
    if bad:raise SystemExit("original terminal state not identical")
    print("PASS original source code8/9/10 terminal state")
if __name__=="__main__":main()
