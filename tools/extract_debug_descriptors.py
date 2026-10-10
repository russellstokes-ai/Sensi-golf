#!/usr/bin/env python3
"""Extract variable pointers embedded after Sensible Golf debug labels."""

from __future__ import annotations
import argparse,json,struct
from pathlib import Path
from pe_target_xrefs import parse_pe, file_offset_to_rva, find_ascii

TYPE_WIDTH={"B":1,"W":2,"D":4}
DISPLAY_KIND={"D":"decimal","H":"hex","F":"fixed/float-unknown"}

def section_for_va(va,image_base,sections):
    rva=va-image_base
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            return s["name"]
    return None

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--targets-file",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    data=args.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)
    targets=[x.strip() for x in args.targets_file.read_text().splitlines()
             if x.strip() and not x.lstrip().startswith("#")]
    rows=[]
    for text in targets:
        found=find_ascii(data,text)
        for off in found:
            end=off+len(text)
            nul_ok=end < len(data) and data[end]==0
            ptr_off=end+1
            ptr=struct.unpack_from("<I",data,ptr_off)[0] if nul_ok and ptr_off+4<=len(data) else None
            rows.append({
                "label":text,
                "file_offset":off,
                "type_prefix":text[:2],
                "display":DISPLAY_KIND.get(text[0],"unknown") if len(text)>=2 else "unknown",
                "width_bytes":TYPE_WIDTH.get(text[1]) if len(text)>=2 else None,
                "nul_terminated":nul_ok,
                "pointer_file_offset":ptr_off if ptr is not None else None,
                "variable_va":ptr,
                "variable_section":section_for_va(ptr,image_base,sections) if ptr is not None else None,
            })
    report={"image_base":image_base,"descriptors":rows}
    txt=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(txt)
    else:print(txt,end="")

if __name__=="__main__":main()
