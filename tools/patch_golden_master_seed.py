#!/usr/bin/env python3
"""Patch only Sensible Golf v1.014's BIOS-clock RNG seed initializer.

Wine cannot execute the game's real-mode INT 1Ah call. For golden-master CI,
replace that one initializer with a deterministic seed store. Downstream RNG
and gameplay code remain original and untouched.
"""

from __future__ import annotations
import argparse,hashlib
from pathlib import Path
from pe_target_xrefs import parse_pe

EXPECTED_SHA256="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
PATCH_VA=0x402B04
EXPECTED=bytes.fromhex("b402cd1a6633d0c1e210668bd16633d3891510a34100c3")

def va_to_offset(va,image_base,sections):
    rva=va-image_base
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            off=s["raw_offset"]+(rva-s["rva"])
            if off+len(EXPECTED) <= s["raw_offset"]+s["raw_size"]:
                return off
    raise ValueError(f"VA {va:#x} is not file-backed")

def patch(data:bytes,seed:int)->bytes:
    digest=hashlib.sha256(data).hexdigest()
    if digest != EXPECTED_SHA256:
        raise ValueError(f"unexpected GOLFWIN.EXE sha256 {digest}")
    image_base,_,sections=parse_pe(data)
    off=va_to_offset(PATCH_VA,image_base,sections)
    actual=data[off:off+len(EXPECTED)]
    if actual != EXPECTED:
        raise ValueError(f"unexpected bytes at {PATCH_VA:#x}: {actual.hex()}")
    # C7 05 <abs32> <imm32>; RET; NOP padding to exactly replace the function.
    replacement=bytes.fromhex("c70510a34100")+int(seed & 0xffffffff).to_bytes(4,"little")+b"\xc3"
    replacement += b"\x90"*(len(EXPECTED)-len(replacement))
    out=bytearray(data)
    out[off:off+len(EXPECTED)]=replacement
    return bytes(out)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("input",type=Path)
    ap.add_argument("output",type=Path)
    ap.add_argument("--seed",type=lambda s:int(s,0),default=0x12345678)
    args=ap.parse_args()
    data=args.input.read_bytes()
    result=patch(data,args.seed)
    args.output.write_bytes(result)
    print(f"patched {args.input} -> {args.output}")
    print(f"seed=0x{args.seed & 0xffffffff:08x}")
    print(f"original_sha256={hashlib.sha256(data).hexdigest()}")
    print(f"patched_sha256={hashlib.sha256(result).hexdigest()}")
    print(f"changed_bytes={sum(a!=b for a,b in zip(data,result))}")
if __name__=="__main__": main()
