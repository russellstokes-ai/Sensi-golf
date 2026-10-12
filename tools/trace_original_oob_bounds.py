#!/usr/bin/env python3
"""Discover original v1.014 course-boundary behaviour with genuine MAPM01.

EXE and original MAPM/MAPI remain ephemeral CI-only inputs. Intentionally
diagnostic; source machine code and outcome labels must be inspected before
turning these observations into a portable source-parity contract.
"""
from __future__ import annotations
import argparse,hashlib,json,struct,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE, UC_PROT_ALL
from unicorn.x86_const import (
    UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,
    UC_X86_REG_EDX,UC_X86_REG_EBP,UC_X86_REG_EIP,
    UC_X86_REG_ESP)
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,wu32,w16,STACK
from original_v1014_course_lookup_oracle import (
    LOOKUP_VA,LOOKUP_RETURN_VA,MAPBUF,DESC,MASK,
    MAP_PTR_VA,MAP_ROW_STRIDE_VA,MAP_X_EXTENT_VA,
    MAP_Y_EXTENT_VA,GREEN_MODE_VA,DESC_BANK_PTR_VA,MASK_BANK_PTR_VA)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("--mapm",type=Path,required=True)
    ap.add_argument("--descriptor-bank",type=Path,required=True)
    ap.add_argument("--selector-bank",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    a=ap.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    course=a.mapm.read_bytes()
    desc=a.descriptor_bank.read_bytes()
    mask=a.selector_bank.read_bytes()
    width,height=struct.unpack_from(">HH",course,0x54)
    uc,_=build_uc(a.exe)
    uc.mem_map(MAPBUF,0x30000,UC_PROT_ALL)
    uc.mem_write(MAPBUF,course)
    uc.mem_write(DESC,desc)
    uc.mem_write(MASK,mask)
    wu32(uc,MAP_PTR_VA,MAPBUF)
    wu32(uc,DESC_BANK_PTR_VA,DESC)
    wu32(uc,MASK_BANK_PTR_VA,MASK)
    wu32(uc,MAP_ROW_STRIDE_VA,width*2)
    wu32(uc,MAP_X_EXTENT_VA,width*16-0x100)
    wu32(uc,MAP_Y_EXTENT_VA,height*8-0xD0)
    w16(uc,GREEN_MODE_VA,0)
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    engine=Cs(CS_ARCH_X86,CS_MODE_32)
    instructions=[f"{i.address:#x} {i.mnemonic} {i.op_str}"
                  for i in engine.disasm(bytes(uc.mem_read(LOOKUP_VA,0x40967c-LOOKUP_VA)),
                                         LOOKUP_VA)]
    tests=[
      (-32768,0),(-257,1),(-1,0),(-1,128),(0,-32768),(0,-1),
      (0,0),(1,1),(2,2),(15,7),(16,8),(127,127),
      (width*16-0x101,100),(width*16-0x100,100),
      (width*16-0xff,100),(width*16-1,100),(width*16,100),
      (width*16+1,100),(width*16+32,100),
      (100,height*8-0xd1),(100,height*8-0xd0),
      (100,height*8-1),(100,height*8),(100,height*8+32),
      (32767,32767),
    ]
    records=[]
    for x,y in tests:
      uc.reg_write(UC_X86_REG_EAX,x&0xffff)
      uc.reg_write(UC_X86_REG_EBX,y&0xffff)
      uc.reg_write(UC_X86_REG_ECX,0)
      uc.reg_write(UC_X86_REG_EDX,0)
      uc.reg_write(UC_X86_REG_EBP,0)
      uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
      trace=[];hits=[]
      def hook(machine,address,size,user):
        if address==LOOKUP_RETURN_VA:
          hits.append(address)
          machine.emu_stop()
        if address>=0x409535 and address<0x40967c:
          trace.append(address)
      token=uc.hook_add(UC_HOOK_CODE,hook)
      failure=None
      try:uc.emu_start(LOOKUP_VA,0,count=900)
      except Exception as e:failure=str(e)
      finally:uc.hook_del(token)
      records.append({
        "x":x,"y":y,
        "ended_at":hex(uc.reg_read(UC_X86_REG_EIP)),
        "hit_return":bool(hits),
        "eax":uc.reg_read(UC_X86_REG_EAX)&0xffffffff,
        "ebx":uc.reg_read(UC_X86_REG_EBX)&0xffffffff,
        "ecx":uc.reg_read(UC_X86_REG_ECX)&0xffffffff,
        "edx":uc.reg_read(UC_X86_REG_EDX)&0xffffffff,
        "instructions":len(trace),
        "first_trace":[hex(t) for t in trace[:16]],
        "failure":failure,
      })
    report={"original_sha256":REFERENCE_SHA256,
      "course":"MAPM01","map_dimensions":[width,height],
      "source_instructions":instructions,"cases":records}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print("Original course dimensions",width,height)
    print("Original source 0x409535 disassembly:")
    print("\n".join(instructions))
    # Surrounding source regions: the world OOB path explicitly jumps to
    # 0x40CD00, while the next game-tick status bit2 branches to 0x40AACF.
    # These may contain presentation/turn scheduling beyond the scoped
    # negative-lookup source proof and require separate recovery.
    for begin,end,label in ((0x40CD00,0x40CD90,"out-of-world-event"),
                            (0x40AACF,0x40AB3C,"next-tick-bit2"),
                            (0x40A220,0x40A250,"pre-shot-counter")):
        print("Original supporting x86 region:",label)
        for ins in engine.disasm(bytes(uc.mem_read(begin,end-begin)),begin):
            print(f"{ins.address:#x} {ins.mnemonic} {ins.op_str}")
    print("Original boundary probe summary:")
    for row in records:
      print("OOB_XY",row["x"],row["y"],"exit",row["ended_at"],
            "eax",hex(row["eax"]),"bx",row["ebx"]&0xffff,
            "cx",row["ecx"]&0xffff,"dx",row["edx"]&0xffff,
            "steps",row["instructions"],"error",row["failure"])
if __name__=="__main__":main()
