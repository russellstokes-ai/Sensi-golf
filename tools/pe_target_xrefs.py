#!/usr/bin/env python3
"""Locate PE32 code references to selected ASCII strings and disassemble around them.

Designed for the Sensible Golf Windows executable. Requires capstone when
--disassemble is requested. The output is metadata/instructions only.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


def parse_pe(data: bytes):
    if data[:2] != b"MZ":
        raise ValueError("not an MZ executable")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe+4] != b"PE\0\0":
        raise ValueError("not a PE executable")
    machine, nsec, _, _, _, opt_size, _ = struct.unpack_from("<HHIIIHH", data, pe+4)
    opt = pe + 24
    magic = struct.unpack_from("<H", data, opt)[0]
    if magic != 0x10B:
        raise ValueError("only PE32 is supported")
    image_base = struct.unpack_from("<I", data, opt+28)[0]
    entry_rva = struct.unpack_from("<I", data, opt+16)[0]
    sections = []
    spos = opt + opt_size
    for i in range(nsec):
        p = spos + i*40
        name = data[p:p+8].split(b"\0",1)[0].decode("ascii","replace")
        virtual_size, rva, raw_size, raw_offset = struct.unpack_from("<IIII", data, p+8)
        sections.append({
            "name":name, "rva":rva, "virtual_size":virtual_size,
            "raw_size":raw_size, "raw_offset":raw_offset,
        })
    return image_base, entry_rva, sections


def file_offset_to_rva(offset: int, sections):
    for s in sections:
        start=s["raw_offset"]; end=start+s["raw_size"]
        if start <= offset < end:
            return s["rva"] + (offset-start), s["name"]
    return None, None


def rva_to_file_offset(rva: int, sections):
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            off=s["raw_offset"] + (rva-s["rva"])
            if off < s["raw_offset"]+s["raw_size"]:
                return off
    return None


def find_ascii(data: bytes, text: str):
    needle=text.encode("ascii")
    pos=0; out=[]
    while True:
        p=data.find(needle,pos)
        if p<0: break
        out.append(p); pos=p+1
    return out


def code_sections(sections):
    # This build uses BEGTEXT rather than conventional .text.
    return [s for s in sections if s["name"] in {"BEGTEXT",".text","CODE","_TEXT"}]


def disassemble_windows(data, image_base, sections, refs, radius=18):
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
        from capstone.x86 import X86_OP_IMM, X86_OP_MEM
    except ImportError as exc:
        raise RuntimeError("capstone is required for --disassemble") from exc

    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=True
    out=[]
    for sec in code_sections(sections):
        blob=data[sec["raw_offset"]:sec["raw_offset"]+sec["raw_size"]]
        base=image_base+sec["rva"]
        insns=list(md.disasm(blob,base))
        address_to_index={ins.address:i for i,ins in enumerate(insns)}
        for ref in refs:
            target=ref["va"]
            for idx,ins in enumerate(insns):
                matched=False
                for op in ins.operands:
                    if op.type == X86_OP_IMM and op.imm == target:
                        matched=True
                    elif op.type == X86_OP_MEM and op.mem.disp == target:
                        matched=True
                if not matched and struct.pack("<I",target) in bytes(ins.bytes):
                    matched=True
                if not matched:
                    continue
                lo=max(0,idx-radius); hi=min(len(insns),idx+radius+1)
                out.append({
                    "target":ref,
                    "xref_va":ins.address,
                    "section":sec["name"],
                    "instructions":[
                        {
                            "address":x.address,
                            "bytes":bytes(x.bytes).hex(),
                            "mnemonic":x.mnemonic,
                            "op_str":x.op_str,
                        }
                        for x in insns[lo:hi]
                    ],
                })
    return out


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--target",action="append",default=[])
    ap.add_argument("--targets-file",type=Path)
    ap.add_argument("--disassemble",action="store_true")
    ap.add_argument("--radius",type=int,default=18)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    targets=list(args.target)
    if args.targets_file:
        targets += [x.strip() for x in args.targets_file.read_text().splitlines()
                    if x.strip() and not x.lstrip().startswith("#")]

    data=args.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)
    refs=[]
    for text in targets:
        for off in find_ascii(data,text):
            rva,sec=file_offset_to_rva(off,sections)
            if rva is None: continue
            refs.append({
                "text":text,"file_offset":off,"section":sec,
                "rva":rva,"va":image_base+rva,
            })

    result={
        "image_base":image_base,
        "entry_rva":entry_rva,
        "sections":sections,
        "targets":refs,
    }
    if args.disassemble:
        result["xrefs"]=disassemble_windows(data,image_base,sections,refs,args.radius)

    text=json.dumps(result,indent=2)+"\n"
    if args.output: args.output.write_text(text)
    else: print(text,end="")


if __name__ == "__main__":
    main()
