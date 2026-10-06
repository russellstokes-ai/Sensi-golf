#!/usr/bin/env python3
"""Disassemble an explicit PE32 VA range using Capstone."""

from __future__ import annotations
import argparse,json
from pathlib import Path
from pe_target_xrefs import parse_pe

def rva_to_off(rva,sections):
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            off=s["raw_offset"]+(rva-s["rva"])
            if off < s["raw_offset"]+s["raw_size"]:
                return off,s
    return None,None

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--start-va",required=True)
    ap.add_argument("--end-va",required=True)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    data=args.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)
    start=int(args.start_va,0);end=int(args.end_va,0)
    start_rva=start-image_base;end_rva=end-image_base
    so,sec=rva_to_off(start_rva,sections)
    eo,sec2=rva_to_off(end_rva-1,sections)
    if so is None or eo is None or sec is not sec2 and sec["name"]!=sec2["name"]:
        raise SystemExit("range must lie in one backed section")
    blob=data[so:eo+1]
    md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=False
    ins=[{"address":i.address,"bytes":bytes(i.bytes).hex(),"mnemonic":i.mnemonic,"op_str":i.op_str}
         for i in md.disasm(blob,start)]
    report={"start_va":start,"end_va":end,"section":sec["name"],"instructions":ins}
    txt=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(txt)
    else:print(txt,end="")
if __name__=="__main__":main()
