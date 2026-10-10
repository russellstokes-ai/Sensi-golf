#!/usr/bin/env python3
"""Find x86 PE32 instruction references to explicit virtual addresses."""

from __future__ import annotations
import argparse,json,struct
from pathlib import Path
from pe_target_xrefs import parse_pe, code_sections

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--va",action="append",required=True,
                    help="virtual address, decimal or 0x-prefixed")
    ap.add_argument("--radius",type=int,default=32)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    try:
        from capstone import Cs,CS_ARCH_X86,CS_MODE_32
        from capstone.x86 import X86_OP_IMM,X86_OP_MEM
    except ImportError as exc:
        raise SystemExit("capstone required") from exc

    targets=[int(x,0) for x in args.va]
    data=args.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)
    md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
    xrefs=[]
    for sec in code_sections(sections):
        blob=data[sec["raw_offset"]:sec["raw_offset"]+sec["raw_size"]]
        base=image_base+sec["rva"]
        insns=list(md.disasm(blob,base))
        for i,ins in enumerate(insns):
            matched=[]
            for target in targets:
                ok=False
                for op in ins.operands:
                    if op.type==X86_OP_IMM and (op.imm & 0xffffffff)==target:
                        ok=True
                    elif op.type==X86_OP_MEM and (op.mem.disp & 0xffffffff)==target:
                        ok=True
                if not ok and struct.pack("<I",target) in bytes(ins.bytes):
                    ok=True
                if ok: matched.append(target)
            if not matched: continue
            lo=max(0,i-args.radius);hi=min(len(insns),i+args.radius+1)
            xrefs.append({
                "section":sec["name"],"xref_va":ins.address,
                "targets":matched,
                "instructions":[{
                    "address":x.address,"bytes":bytes(x.bytes).hex(),
                    "mnemonic":x.mnemonic,"op_str":x.op_str
                } for x in insns[lo:hi]]
            })
    report={"image_base":image_base,"targets":targets,"xrefs":xrefs}
    txt=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(txt)
    else:print(txt,end="")

if __name__=="__main__":main()
