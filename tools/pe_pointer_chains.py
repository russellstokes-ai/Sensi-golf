#!/usr/bin/env python3
"""Find raw pointer/offset encodings that lead to selected PE strings.

Useful for Watcom-era PE binaries where near pointers may be stored as DGROUP
relative offsets rather than absolute virtual addresses.
"""

from __future__ import annotations
import argparse, json, struct
from pathlib import Path
from pe_target_xrefs import parse_pe, file_offset_to_rva, find_ascii


def section_for_offset(offset, sections):
    for s in sections:
        if s["raw_offset"] <= offset < s["raw_offset"] + s["raw_size"]:
            return s["name"]
    return None


def hits(data: bytes, value: int):
    if not 0 <= value <= 0xffffffff:
        return []
    needle=struct.pack("<I",value)
    out=[]; pos=0
    while True:
        p=data.find(needle,pos)
        if p<0: break
        out.append(p); pos=p+1
    return out


def words_around(data, offset, before=24, after=40):
    lo=max(0,offset-before); hi=min(len(data),offset+after)
    rows=[]
    p=lo-(lo%4)
    while p+4<=hi:
        rows.append({"offset":p,"u32":struct.unpack_from("<I",data,p)[0],"hex":data[p:p+4].hex()})
        p+=4
    return rows


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--targets-file",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()
    data=args.exe.read_bytes()
    image_base,entry_rva,sections=parse_pe(data)
    dgroup=next((s for s in sections if s["name"]=="DGROUP"),None)
    targets=[x.strip() for x in args.targets_file.read_text().splitlines()
             if x.strip() and not x.lstrip().startswith("#")]
    out=[]
    for text in targets:
        for off in find_ascii(data,text):
            rva,sec=file_offset_to_rva(off,sections)
            if rva is None: continue
            encodings={
                "va":image_base+rva,
                "rva":rva,
                "file_offset":off,
            }
            if dgroup:
                encodings["dgroup_raw_relative"]=off-dgroup["raw_offset"]
                encodings["dgroup_rva_relative"]=rva-dgroup["rva"]
            refs=[]
            seen=set()
            for kind,val in encodings.items():
                for p in hits(data,val):
                    if p==off: continue
                    key=(p,kind)
                    if key in seen: continue
                    seen.add(key)
                    refs.append({
                        "encoding":kind,"encoded_value":val,
                        "file_offset":p,"section":section_for_offset(p,sections),
                        "words":words_around(data,p),
                    })
            out.append({
                "text":text,"string_file_offset":off,"string_section":sec,
                "string_rva":rva,"string_va":image_base+rva,
                "encodings":encodings,"references":refs,
            })
    report={"image_base":image_base,"entry_rva":entry_rva,"sections":sections,"targets":out}
    txt=json.dumps(report,indent=2)+"\n"
    if args.output: args.output.write_text(txt)
    else: print(txt,end="")

if __name__=="__main__":
    main()
