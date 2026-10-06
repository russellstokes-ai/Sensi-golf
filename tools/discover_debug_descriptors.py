#!/usr/bin/env python3
"""Discover all embedded Sensible Golf debug variable descriptors."""
from __future__ import annotations
import argparse,json,re,struct
from pathlib import Path
from pe_target_xrefs import parse_pe

TYPE_WIDTH={"B":1,"W":2,"D":4}
DISPLAY_KIND={"D":"decimal","H":"hex","F":"fixed/float-unknown"}

def section_for_va(va,image_base,sections):
    rva=va-image_base
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            return s["name"]
    return None

def printable_strings(data,min_len=4):
    out=[];start=None
    for i,b in enumerate(data+b"\0"):
        if 32<=b<=126:
            if start is None:start=i
        elif start is not None:
            if i-start>=min_len:
                out.append((start,data[start:i].decode("ascii","replace")))
            start=None
    return out

def discover(data,image_base,sections):
    rows=[]
    for off,text in printable_strings(data):
        if len(text)<3 or text[0] not in DISPLAY_KIND or text[1] not in TYPE_WIDTH:
            continue
        if not re.fullmatch(r"[A-Za-z0-9 _/().,+:\-]+",text):
            continue
        ptr_off=off+len(text)+1
        if ptr_off+4>len(data):continue
        ptr=struct.unpack_from("<I",data,ptr_off)[0]
        sec=section_for_va(ptr,image_base,sections)
        if sec not in {"DGROUP",".bss"}:continue
        rows.append({
            "label":text,"file_offset":off,
            "display":DISPLAY_KIND[text[0]],"type_prefix":text[:2],
            "width_bytes":TYPE_WIDTH[text[1]],
            "pointer_file_offset":ptr_off,"variable_va":ptr,
            "variable_section":sec,
        })
    seen=set();result=[]
    for row in rows:
        key=(row["label"],row["variable_va"])
        if key in seen:continue
        seen.add(key);result.append(row)
    return result

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()
    data=args.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)
    report={"image_base":image_base,"descriptors":discover(data,image_base,sections)}
    text=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(text,encoding="utf-8")
    else:print(text,end="")

if __name__=="__main__":main()
