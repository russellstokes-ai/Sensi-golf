#!/usr/bin/env python3
"""Apply minimal compatibility patches to Sensible Golf v1.014 for Wine oracle CI.

Only two obsolete hardware assumptions are changed in a disposable copy:
1) BIOS INT 1Ah RNG seeding -> fixed deterministic seed.
2) CLI/STI around a one-byte timer handoff -> NOP/NOP.

No shot, ball, club, terrain, collision or RNG-update logic is altered.
"""

from __future__ import annotations
import argparse,hashlib
from pathlib import Path
from pe_target_xrefs import parse_pe

EXPECTED_SHA256="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
SEED_VA=0x402B04
SEED_EXPECTED=bytes.fromhex("b402cd1a6633d0c1e210668bd16633d3891510a34100c3")
CLI_VA=0x402A6A
STI_VA=0x402A78

def va_to_offset(va,size,image_base,sections):
    rva=va-image_base
    for s in sections:
        span=max(s["virtual_size"],s["raw_size"])
        if s["rva"] <= rva < s["rva"]+span:
            off=s["raw_offset"]+(rva-s["rva"])
            if off+size <= s["raw_offset"]+s["raw_size"]:
                return off
    raise ValueError(f"VA {va:#x} is not file-backed")

def patch(data:bytes,seed:int)->bytes:
    digest=hashlib.sha256(data).hexdigest()
    if digest != EXPECTED_SHA256:
        raise ValueError(f"unexpected GOLFWIN.EXE sha256 {digest}")
    image_base,_,sections=parse_pe(data)
    out=bytearray(data)

    seed_off=va_to_offset(SEED_VA,len(SEED_EXPECTED),image_base,sections)
    actual=data[seed_off:seed_off+len(SEED_EXPECTED)]
    if actual != SEED_EXPECTED:
        raise ValueError(f"unexpected seed bytes at {SEED_VA:#x}: {actual.hex()}")
    replacement=bytes.fromhex("c70510a34100")+int(seed & 0xffffffff).to_bytes(4,"little")+b"\xc3"
    replacement += b"\x90"*(len(SEED_EXPECTED)-len(replacement))
    out[seed_off:seed_off+len(SEED_EXPECTED)]=replacement

    for va,expected in ((CLI_VA,0xFA),(STI_VA,0xFB)):
        off=va_to_offset(va,1,image_base,sections)
        if data[off] != expected:
            raise ValueError(f"unexpected privileged opcode at {va:#x}: {data[off]:#x}")
        out[off]=0x90

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
    changed=[i for i,(a,b) in enumerate(zip(data,result)) if a!=b]
    print(f"patched {args.input} -> {args.output}")
    print(f"seed=0x{args.seed & 0xffffffff:08x}")
    print(f"original_sha256={hashlib.sha256(data).hexdigest()}")
    print(f"patched_sha256={hashlib.sha256(result).hexdigest()}")
    print(f"changed_bytes={len(changed)}")
if __name__=="__main__": main()
