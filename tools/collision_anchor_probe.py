#!/usr/bin/env python3
"""Evidence-led collision/object anchor probe for Sensible Golf Windows v1.014."""

from __future__ import annotations
import argparse,json,re,struct
from pathlib import Path

from discover_debug_descriptors import discover
from pe_target_xrefs import parse_pe, file_offset_to_rva, disassemble_windows

KEYWORDS=re.compile(r"(tree|wood|bush|branch|trunk|object|obstacle|collision|collide|wall|fence|post|hit|sprite|tile)",re.I)

def ascii_strings(data:bytes,min_len:int=4):
    out=[]; start=None
    for i,b in enumerate(data+b"\0"):
        if 32<=b<=126:
            if start is None:start=i
        elif start is not None:
            if i-start>=min_len:
                out.append((start,data[start:i].decode("ascii","replace")))
            start=None
    return out

def scan_epf(root:Path):
    rows=[]
    for p in sorted(root.glob("*")):
        if not p.is_file():continue
        name_match=bool(KEYWORDS.search(p.name))
        strings=[]
        data=p.read_bytes()
        for off,s in ascii_strings(data,4):
            if KEYWORDS.search(s):
                strings.append({"offset":off,"text":s})
                if len(strings)>=30:break
        if name_match or strings:
            rows.append({"file":p.name,"size":p.stat().st_size,"name_match":name_match,"strings":strings})
    return rows

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--epf-root",type=Path)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()

    data=a.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)

    candidate_strings=[]
    refs=[]
    for off,s in ascii_strings(data,4):
        if not KEYWORDS.search(s):continue
        rva,sec=file_offset_to_rva(off,sections)
        row={"text":s,"file_offset":off,"section":sec}
        if rva is not None:
            row["rva"]=rva; row["va"]=image_base+rva
            refs.append({"text":s,"file_offset":off,"section":sec,"rva":rva,"va":image_base+rva})
        candidate_strings.append(row)

    descriptors=discover(data,image_base,sections)
    descriptor_hits=[d for d in descriptors if KEYWORDS.search(d["label"])]

    xrefs=disassemble_windows(data,image_base,sections,refs,radius=14) if refs else []

    report={
        "image_base":image_base,
        "entry_rva":entry_rva,
        "keywords":KEYWORDS.pattern,
        "candidate_strings":candidate_strings,
        "debug_descriptors":descriptor_hits,
        "string_xrefs":xrefs,
        "epf_candidates":scan_epf(a.epf_root) if a.epf_root else [],
    }
    text=json.dumps(report,indent=2)+"\n"
    if a.output:a.output.write_text(text,encoding="utf-8")
    else:print(text,end="")

if __name__=="__main__":main()
