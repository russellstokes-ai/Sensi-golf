#!/usr/bin/env python3
"""Extract the live-player accuracy selector, bounds and profile tables from Sensible Golf v1.014."""

from __future__ import annotations
import argparse, json, struct
from pathlib import Path
from pe_target_xrefs import parse_pe

CENTER = 63
SELECTOR_VA = 0x41D4C4
BOUNDS_VA = 0x41D546
PROFILE_CENTER_VA = 0x41F204
CLUBS = 13
LIE_SLOTS = 10
PROFILES = 11
PROFILE_STRIDE = 28

def va_to_offset(va, image_base, sections):
    rva = va - image_base
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            off=s["raw_offset"]+(rva-s["rva"])
            if off < s["raw_offset"]+s["raw_size"]:
                return off
    raise ValueError(f"unbacked VA {va:#x}")

def read_u16(data, off):
    return struct.unpack_from("<H", data, off)[0]

def read_i16(data, off):
    return struct.unpack_from("<h", data, off)[0]

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    data=args.exe.read_bytes()
    image_base,_,sections=parse_pe(data)

    selector_off=va_to_offset(SELECTOR_VA,image_base,sections)
    selectors=[]
    for club in range(CLUBS):
        row=list(data[selector_off+club*LIE_SLOTS:selector_off+(club+1)*LIE_SLOTS])
        selectors.append(row)

    bounds=[]
    bounds_off=va_to_offset(BOUNDS_VA,image_base,sections)
    for p in range(PROFILES):
        lower=read_u16(data,bounds_off+p*4)
        upper=read_u16(data,bounds_off+p*4+2)
        bounds.append({"profile_id":p+1,"lower":lower,"upper":upper})

    profiles=[]
    for p in range(PROFILES):
        center_va=PROFILE_CENTER_VA+p*PROFILE_STRIDE
        lower=bounds[p]["lower"]
        upper=bounds[p]["upper"]
        errors=list(range(lower-CENTER, upper-CENTER+1))
        even_offsets=sorted(set((e >> 1) << 1 for e in errors))
        samples={}
        for e in even_offsets:
            off=va_to_offset(center_va+e,image_base,sections)
            samples[str(e)]=read_i16(data,off)
        profiles.append({
            "profile_id":p+1,
            "center_va":center_va,
            "lower":lower,
            "upper":upper,
            "errors":[min(errors),max(errors)] if errors else [],
            "samples_by_even_error":samples,
        })

    report={
        "accuracy_center":CENTER,
        "selector_va":SELECTOR_VA,
        "selector_row_width":LIE_SLOTS,
        "selectors":selectors,
        "bounds_va":BOUNDS_VA,
        "bounds":bounds,
        "profile_center_va":PROFILE_CENTER_VA,
        "profile_stride":PROFILE_STRIDE,
        "profiles":profiles,
    }
    text=json.dumps(report,indent=2)+"\n"
    if args.output: args.output.write_text(text,encoding="utf-8")
    else: print(text,end="")

if __name__=="__main__":
    main()
