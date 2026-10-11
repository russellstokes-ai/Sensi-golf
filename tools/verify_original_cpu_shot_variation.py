#!/usr/bin/env python3
"""Original v1.014 computer shot deviation fragment 0x40C7DD..0x40C84F.
Inject predetermined original-RNG RETURNS at 0x406FFA, execute all real
x86 branches, and compare with a portable C++ port, bit-for-bit.
This does not prove original RNG distribution or complete shot planning.
"""
import argparse,hashlib,json,struct,subprocess,sys
from pathlib import Path
from itertools import product
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,STACK,w16,ru16
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EIP,UC_X86_REG_EAX,UC_X86_REG_ESI,UC_X86_REG_ESP
POWER=0x41d59e
VARIATION=0x41fa0d
CURVE=0x41f32c
RANDOM=0x406ffa
START=0x40c7dd
END=0x40c84f

def original(uc,flag,power,variation,r100,rpower,rdir):
    w16(uc,PLAYER+0x62,flag)
    w16(uc,POWER,power)
    w16(uc,VARIATION,variation)
    uc.mem_write(CURVE,struct.pack("<i",0))
    esp=STACK+0xf000
    uc.reg_write(UC_X86_REG_ESP,esp)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    choices=[r100,rpower,rdir]
    actual_calls=[]
    def hook(machine,addr,size,user):
        if addr!=RANDOM:return
        from unicorn.x86_const import UC_X86_REG_ESP
        sp=machine.reg_read(UC_X86_REG_ESP)
        ret=struct.unpack("<I",bytes(machine.mem_read(sp,4)))[0]
        call=len(actual_calls)
        if call>=len(choices):raise RuntimeError("original x86 called RNG > 3 times")
        actual_calls.append({"range":machine.reg_read(UC_X86_REG_EAX)&0xffff,
                             "sample":choices[call]})
        old=machine.reg_read(UC_X86_REG_EAX)
        machine.reg_write(UC_X86_REG_EAX,(old&0xffff0000)|choices[call])
        machine.reg_write(UC_X86_REG_ESP,sp+4)
        machine.reg_write(UC_X86_REG_EIP,ret)
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,END,count=1000)
    finally:uc.hook_del(token)
    return [ru16(uc,POWER),ru16(uc,VARIATION),
            struct.unpack("<i",bytes(uc.mem_read(CURVE,4)))[0],
            len(actual_calls)],actual_calls

def make_cases():
    cases=[]
    for flag in (0,1):
      for p in (0,5,30,80,105):
       for v in (0,1,3,7):
        for r1 in (0,15,16,30,80,81,99):
         boundary=max(1,r1>>3)
         for r2 in sorted(set((0,boundary//2,boundary-1))):
          vv=v+(r1>>5 if r1>80 else 0)
          for r3 in sorted(set((0,vv,(vv*2-1 if vv else 0)))):
           cases.append((flag,p,v,r1,r2,r3))
    return cases

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--exe",type=Path,required=True)
    parser.add_argument("--probe",type=Path,required=True)
    parser.add_argument("-o","--output",type=Path,required=True)
    a=parser.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    cases=make_cases()
    native=subprocess.run([str(a.probe)],input="".join("%d %d %d %d %d %d\n"%row for row in cases),
                          text=True,capture_output=True,check=True)
    outs=native.stdout.strip().splitlines()
    assert len(outs)==len(cases)
    uc,_=build_uc(a.exe)
    bad=[];count=0;call_bounds={}
    for i,(row,line) in enumerate(zip(cases,outs)):
        ref,calls=original(uc,*row)
        got=list(map(int,line.split()))
        if got!=ref: bad.append({"input":row,"original":ref,"native":got,"rng_calls":calls})
        else:count+=1
        call_bounds[len(calls)]=call_bounds.get(len(calls),0)+1
    output={"schema":"pixtee.original-cpu-shot-rng-variation-x86.v1",
            "original_exe_sha256":REFERENCE_SHA256,"vectors":len(cases),
            "exact_matches":count,"mismatches":bad[:60],
            "original_rng_call_counts":call_bounds,
            "scope":"source x86 deterministic random return effect only, RNG statistical behaviour and complete AI not claimed"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(output,indent=2)+"\n")
    print(f"Source original computer shot stochastic adjustment: {count}/{len(cases)}")
    for row in bad[:6]:print("MISMATCH",row)
    if bad:raise SystemExit("CPU original swing variation parity failed")
    print("PASS original Windows CPU shot error branch compares exactly")
if __name__=="__main__":main()
