#!/usr/bin/env python3
"""Scan PE32 x86 code for memory operands using selected struct displacements."""

from __future__ import annotations
import argparse,json
from pathlib import Path
from pe_target_xrefs import parse_pe,code_sections

REGNAMES=("eax","ebx","ecx","edx","esi","edi","ebp")

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--offset",action="append",required=True)
    ap.add_argument("--radius",type=int,default=8)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from capstone.x86 import X86_OP_MEM
    data=args.exe.read_bytes()
    image_base,_,sections=parse_pe(data)
    offsets={int(x,0) for x in args.offset}
    md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
    hits=[]
    for sec in code_sections(sections):
        blob=data[sec["raw_offset"]:sec["raw_offset"]+sec["raw_size"]]
        insns=list(md.disasm(blob,image_base+sec["rva"]))
        for idx,ins in enumerate(insns):
            matched=[]
            for op in ins.operands:
                if op.type==X86_OP_MEM and op.mem.disp in offsets and op.mem.base:
                    matched.append({
                        "disp":op.mem.disp,
                        "base":ins.reg_name(op.mem.base),
                        "index":ins.reg_name(op.mem.index) if op.mem.index else None,
                        "scale":op.mem.scale,
                    })
            if not matched: continue
            lo=max(0,idx-args.radius);hi=min(len(insns),idx+args.radius+1)
            hits.append({
                "address":ins.address,
                "instruction":{"mnemonic":ins.mnemonic,"op_str":ins.op_str},
                "operands":matched,
                "context":[{"address":x.address,"mnemonic":x.mnemonic,"op_str":x.op_str}
                           for x in insns[lo:hi]]
            })
    report={"offsets":sorted(offsets),"hits":hits}
    txt=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(txt)
    else:print(txt,end="")

if __name__=="__main__":
    main()
