#!/usr/bin/env python3
"""Extract metadata-only draw/fade swing-profile tables from Sensible Golf PE."""
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
            if off < s["raw_offset"]+s["raw_size"]:
                return off
    raise ValueError(f"unbacked VA {va:#x}")

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--base-va",default="0x41f204")
    ap.add_argument("--profiles",type=int,default=11)
    ap.add_argument("--bytes-per-profile",type=int,default=28)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    data=args.exe.read_bytes()
    image_base,_,sections=parse_pe(data)
    base=int(args.base_va,0)
    # 0x41F204 is the centre word used when accuracy_error == 0.
    # Each profile has 28 bytes = 14 signed 16-bit samples. The launch code
    # adds an even signed accuracy offset directly to this centre pointer.
    rows=[]
    for p in range(args.profiles):
        center_va=base+p*args.bytes_per_profile
        center_off=va_to_offset(center_va,image_base,sections)
        samples={}
        for error in range(-12,14,2):
            off=center_off+error
            if off < 0 or off+2 > len(data): continue
            samples[str(error)]=struct.unpack_from("<h",data,off)[0]
        rows.append({
            "profile_index":p,
            "center_va":center_va,
            "center_value":struct.unpack_from("<h",data,center_off)[0],
            "samples_by_even_accuracy_error":samples,
        })
    report={"base_va":base,"profile_stride":args.bytes_per_profile,"profiles":rows}
    txt=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(txt,encoding="utf-8")
    else:print(txt,end="")

if __name__=="__main__":main()
