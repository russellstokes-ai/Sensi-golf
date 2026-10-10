#!/usr/bin/env python3
"""Audit likely PRNG/random and timing anchors in Sensible Golf Windows v1.014."""

from __future__ import annotations
import argparse,json,re
from pathlib import Path
from pe_target_xrefs import parse_pe,file_offset_to_rva,disassemble_windows
from discover_debug_descriptors import discover

RANDOM=re.compile(r"(rand|random|seed|noise|jitter|chance)",re.I)
TIMING=re.compile(r"(tick|timer|time|clock|frame|delay|pause|speed|vbl|vertical)",re.I)

def strings(data,min_len=4):
    out=[];start=None
    for i,b in enumerate(data+b"\0"):
        if 32<=b<=126:
            if start is None:start=i
        elif start is not None:
            if i-start>=min_len:out.append((start,data[start:i].decode("ascii","replace")))
            start=None
    return out

def collect(data,image_base,sections,pattern):
    rows=[];refs=[]
    for off,s in strings(data):
        if not pattern.search(s):continue
        rva,sec=file_offset_to_rva(off,sections)
        row={"text":s,"file_offset":off,"section":sec}
        if rva is not None:
            row.update({"rva":rva,"va":image_base+rva})
            refs.append(dict(row))
        rows.append(row)
    return rows,refs

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()
    data=a.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)
    descriptors=discover(data,image_base,sections)

    rand_strings,rand_refs=collect(data,image_base,sections,RANDOM)
    time_strings,time_refs=collect(data,image_base,sections,TIMING)
    rand_desc=[d for d in descriptors if RANDOM.search(d["label"])]
    time_desc=[d for d in descriptors if TIMING.search(d["label"])]

    report={
      "image_base":image_base,
      "entry_rva":entry_rva,
      "random":{"strings":rand_strings,"descriptors":rand_desc,
                "xrefs":disassemble_windows(data,image_base,sections,rand_refs,radius=16) if rand_refs else []},
      "timing":{"strings":time_strings,"descriptors":time_desc,
                "xrefs":disassemble_windows(data,image_base,sections,time_refs,radius=16) if time_refs else []},
    }
    text=json.dumps(report,indent=2)+"\n"
    if a.output:a.output.write_text(text,encoding="utf-8")
    else:print(text,end="")
if __name__=="__main__":main()
