#!/usr/bin/env python3
"""Identify real v1.014 flagpole/object source values, not presumed sprites.

Direct original Windows x86 reads/writes for 0x41F3B2 and 0x41F3B4
(flag coordinates), 0x41EE22/26 (course maps), the actual SPT player
record pointer and original WOOD sprites. No graphics/masks are committed.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from pe_target_xrefs import parse_pe
SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
FLAG=("0x41f3b2","0x41f3b4")
OTHER=("0x41f3a8","0x41ee22","0x41ee26","0x41f3ac")
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("-o","--output",type=Path,required=True)
    args=p.parse_args()
    source=args.exe.read_bytes()
    if hashlib.sha256(source).hexdigest()!=SHA:
        raise SystemExit("not original SHA-verified Windows 1.014")
    base,_,sections=parse_pe(source)
    dis=Cs(CS_ARCH_X86,CS_MODE_32)
    refs=[]
    for sec in sections:
        if not(sec["rva"]<=0x40a50b-base<
               sec["rva"]+max(sec["virtual_size"],sec["raw_size"])):
            continue
        code=source[sec["raw_offset"]:sec["raw_offset"]+sec["raw_size"]]
        ins=list(dis.disasm(code,base+sec["rva"]))
        for idx,inst in enumerate(ins):
            ops=inst.op_str.lower()
            vals=[a for a in FLAG+OTHER if a in ops]
            if not vals:continue
            context=[f"{n.address:#x} {n.mnemonic} {n.op_str}"
                     for n in ins[max(0,idx-20):min(len(ins),idx+22)]]
            refs.append({"address":hex(inst.address),
                "source":inst.mnemonic+" "+inst.op_str,
                "global_addresses":vals,"context":context})
    wood_strings={name:source.count(name.encode("ascii"))
        for name in ("WOOD1","WOOD2","WOOD3","WOOD4","TREE","FLAG")}
    result={"schema":"pixtee.original-flagpole-and-tree-identity-source-xrefs.v1",
        "original_sha256":SHA,"global_reference_count":len(refs),
        "flag_coordinate_reference_count":sum(
            any(a in r["global_addresses"] for a in FLAG) for r in refs),
        "references":refs,"literal_string_counts":wood_strings,
        "no_assumed_tree_geometry":True,
        "scope":"Source xrefs and binary-string evidence; no positive object existence or 3D tree collision height claimed without original runtime proof"}
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2)+"\n")
    print("Original flag read/write references:",
          result["flag_coordinate_reference_count"],
          "other global xrefs:",len(refs),
          "WOOD/TREE/FLAG literal counts:",wood_strings)
    for x in refs:
        if any(a in x["global_addresses"] for a in FLAG):
            print("\nORIGINAL FLAG ADDRESS XREF",x["address"],x["source"])
            print("\n".join(x["context"]))
    print("Source analysis only; tree identity and height remain blocked")
if __name__=="__main__":main()
