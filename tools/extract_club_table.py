#!/usr/bin/env python3
"""Extract metadata-only club physics tables from Sensible Golf v1.014 PE."""

from __future__ import annotations
import argparse,json,struct
from pathlib import Path
from pe_target_xrefs import parse_pe

def va_to_offset(va,image_base,sections):
    rva=va-image_base
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            off=s["raw_offset"]+(rva-s["rva"])
            if off < s["raw_offset"]+s["raw_size"]: return off
    raise ValueError(f"unbacked VA {va:#x}")

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--club-base",default="0x41f108")
    ap.add_argument("--clubs",type=int,default=13)
    ap.add_argument("--lie-distance-base",default="0x41d4c4")
    ap.add_argument("--lie-width",type=int,default=10)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()
    data=args.exe.read_bytes()
    image_base,_,sections=parse_pe(data)

    club_base=int(args.club_base,0)
    off=va_to_offset(club_base,image_base,sections)
    clubs=[]
    for i in range(args.clubs):
        a,b,c=struct.unpack_from("<iii",data,off+i*12)
        clubs.append({
            "index":i,
            "raw_vertical_base":a,
            "raw_horizontal_base":b,
            "power_scale":c,
            "loaded_vertical_base":a >> 1,
            "loaded_horizontal_base":b >> 1,
            "special_putter_path":i==12,
        })

    lie_base=int(args.lie_distance_base,0)
    loff=va_to_offset(lie_base,image_base,sections)
    lie_rows=[]
    for i in range(args.clubs):
        row=list(data[loff+i*args.lie_width:loff+(i+1)*args.lie_width])
        lie_rows.append({"index":i,"values":row})

    report={
        "club_table_va":club_base,
        "entry_size":12,
        "club_count":args.clubs,
        "clubs":clubs,
        "lie_distance_table_va":lie_base,
        "lie_row_width":args.lie_width,
        "lie_distance_rows":lie_rows,
        "notes":[
            "At 0x40A1C1 the selected club index is multiplied by 12 and added to club_table_va.",
            "First and second dwords are arithmetic-shifted/right-shifted by one before loading shot base V/H force.",
            "Third dword is copied as the shot power scaling field.",
            "Club index 12 takes the special putter update path in 0x40CB59."
        ]
    }
    txt=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(txt)
    else:print(txt,end="")

if __name__=="__main__":main()
