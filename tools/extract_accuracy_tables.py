#!/usr/bin/env python3
"""Extract Sensible Golf v1.014 accuracy bounds, profile selectors and swing tables."""

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
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    data=args.exe.read_bytes()
    image_base,_,sections=parse_pe(data)

    lower_va=0x41d4c0
    upper_va=0x41d4c2
    selector_va=0x41d4c4
    profile_center_va=0x41f204

    lower=struct.unpack_from("<H",data,va_to_offset(lower_va,image_base,sections))[0]
    upper=struct.unpack_from("<H",data,va_to_offset(upper_va,image_base,sections))[0]

    selector_off=va_to_offset(selector_va,image_base,sections)
    selectors=[]
    for club in range(13):
        row=list(data[selector_off+club*10:selector_off+(club+1)*10])
        selectors.append({"club_index":club,"profile_ids_one_based":row})

    # Launch code:
    #   error = meter - 63
    #   even_error = (error >> 1) << 1   (x86 SAR then SHL)
    #   ptr = profile_center + (profile_id-1)*28 + even_error
    possible_even_offsets=sorted({
        ((error >> 1) << 1)
        for error in range(int(lower)-63,int(upper)-63+1)
    })

    profiles=[]
    for p in range(11):
        center=profile_center_va+p*28
        samples={}
        for even_error in possible_even_offsets:
            off=va_to_offset(center+even_error,image_base,sections)
            samples[str(even_error)]=struct.unpack_from("<h",data,off)[0]
        profiles.append({
            "profile_id_one_based":p+1,
            "center_va":center,
            "samples_by_even_accuracy_error":samples,
        })

    report={
        "accuracy_center":63,
        "accuracy_lower":lower,
        "accuracy_upper":upper,
        "raw_error_range":[int(lower)-63,int(upper)-63],
        "evenized_offsets":possible_even_offsets,
        "selector_table_va":selector_va,
        "selector_row_width":10,
        "selectors":selectors,
        "profile_center_va":profile_center_va,
        "profile_stride_bytes":28,
        "profiles":profiles,
    }
    txt=json.dumps(report,indent=2)+"\n"
    if args.output:args.output.write_text(txt,encoding="utf-8")
    else:print(txt,end="")

if __name__=="__main__":main()
