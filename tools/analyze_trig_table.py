#!/usr/bin/env python3
"""Analyze Sensible Golf's fixed-point trigonometric lookup table."""

from __future__ import annotations
import argparse,json,math,struct
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
    raise ValueError(f"VA {va:#x} is not file-backed")

def candidate_values(count, func, quant, clamp_negative_peak=False):
    out=[]
    for i in range(count):
        x=func(2*math.pi*i/4096.0)*16384.0
        if quant=="round": v=int(round(x))
        elif quant=="trunc": v=int(x)
        elif quant=="floor": v=math.floor(x)
        elif quant=="ceil": v=math.ceil(x)
        else: raise ValueError(quant)
        if clamp_negative_peak and v < -16383:
            v=-16383
        v=((int(v)+32768)&0xffff)-32768
        out.append(v)
    return out

def compare(actual,vals):
    mismatch_indices=[i for i,(a,b) in enumerate(zip(actual,vals)) if a!=b]
    return {
        "mismatches":len(mismatch_indices),
        "max_abs_error":max(abs(a-b) for a,b in zip(actual,vals)),
        "first_mismatch_indices":mismatch_indices[:16],
    }

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("--base-va",default="0x41a670")
    ap.add_argument("--count",type=int,default=5120)
    ap.add_argument("-o","--output",type=Path)
    args=ap.parse_args()

    data=args.exe.read_bytes()
    image_base,_,sections=parse_pe(data)
    base=int(args.base_va,0)
    off=va_to_offset(base,image_base,sections)
    actual=list(struct.unpack_from("<"+("h"*args.count),data,off))

    candidates={}
    for fname,func in (("sin",math.sin),("cos",math.cos)):
        for quant in ("round","trunc","floor","ceil"):
            vals=candidate_values(args.count,func,quant)
            candidates[f"{fname}-{quant}"]=compare(actual,vals)

    exact_candidate=candidate_values(args.count,math.sin,"trunc",clamp_negative_peak=True)
    candidates["sin-trunc-clamp-negative-peak"]=compare(actual,exact_candidate)

    samples={str(i):actual[i] for i in [0,1,2,256,512,768,1024,1536,2048,3072,4095,4096,5119] if i < len(actual)}
    periodic_4096=all(actual[i]==actual[i+4096] for i in range(min(1024,args.count-4096))) if args.count>4096 else None

    report={
        "base_va":base,
        "count":args.count,
        "samples":samples,
        "candidate_matches":candidates,
        "periodic_4096_for_extra_quarter":periodic_4096,
        "exact_regeneration_candidate":"int(sin(2*pi*i/4096)*16384), clamped to minimum -16383",
        "sha256_note":"Raw table bytes are not emitted; only reproducibility metrics and selected samples are reported."
    }
    txt=json.dumps(report,indent=2)+"\n"
    if args.output: args.output.write_text(txt)
    else: print(txt,end="")

if __name__=="__main__":
    main()
