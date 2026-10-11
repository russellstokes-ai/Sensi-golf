#!/usr/bin/env python3
"""SHA-verified original MAPM object-byte and x86 map-reference inventory.

Tests whether the unused upper six MAPM word bits carry distinguishable
information and locates original x86 functions referencing map resources.
No proof of tree identity unless a source renderer/collision predicate is
independently established. No original course or executable bytes output.
"""
from __future__ import annotations
import argparse,hashlib,json,re,struct
from collections import Counter
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from pe_target_xrefs import parse_pe
from pe_disasm_range import rva_to_off

SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
RESOURCE_ADDRESSES=("0x41ee22","0x41ee26","0x41ee06",
                    "0x41ee72","0x41ee76")
HIGH_IMMEDIATE={"0x3ff","0xfc00","0x7ff","0xf000","0xffc0","0x3f"}
def course_summary(mapm):
    w,h=struct.unpack_from(">HH",mapm,0x54)
    if w<=0 or h<=0 or len(mapm)<0x60+w*h*2:
        raise ValueError("MAPM01 truncated")
    raw=struct.unpack_from(">"+str(w*h)+"H",mapm,0x60)
    high=Counter(value>>10 for value in raw)
    tiles=Counter(value&0x3ff for value in raw)
    combos=Counter((value>>10,value&0x3ff) for value in raw)
    nonzero=sum(n for k,n in high.items() if k!=0)
    nonzero_positions=[
        {"x":i%w,"y":i//w,"high_six_bits":v>>10,
         "low_ten_bits":v&0x3ff}
        for i,v in enumerate(raw) if v>>10
    ]
    return {"upper_nonzero_tile_positions":nonzero_positions,
        "width":w,"height":h,"tiles_total":len(raw),
        "high_six_bits_distribution":dict(sorted(high.items())),
        "nonzero_high_six_bit_tile_count":nonzero,
        "low_ten_bit_distinct_tiles":len(tiles),
        "upper_bit_combination_count":len(combos),
        "high_six_bits_scoped":"Only data distribution; no tree/obstacle identity claimed"}

def original_machine_refs(exe):
    base,_,secs=parse_pe(exe)
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    hits=[];n=0
    for sec in secs:
        # Some original PE section names are nonstandard. Locate the
        # executable region by the verified 0x409535 machine-code VA,
        # NOT a guessed '.text' section label (zero matches previously).
        lookup_rva=0x409535-base
        start_rva=sec["rva"]
        section_span=max(sec["virtual_size"],sec["raw_size"])
        if not (start_rva<=lookup_rva<start_rva+section_span):
            continue
        offset=sec["raw_offset"]
        buf=exe[offset:offset+sec["raw_size"]]
        va=base+sec["rva"]
        ins=list(md.disasm(buf,va))
        n+=len(ins)
        for i,z in enumerate(ins):
            tokens=z.op_str.lower()
            mapref=any(s in tokens for s in RESOURCE_ADDRESSES)
            indirect_call= z.mnemonic in ("call","jmp") and "0x409535" in tokens
            highmask=(
                z.mnemonic in ("and","shr","sar","shl","test") and
                any(re.search(r"(?<![a-z0-9])"+re.escape(k)+r"(?![a-z0-9])",tokens)
                    for k in HIGH_IMMEDIATE)
            )
            if not(mapref or indirect_call or highmask):continue
            radius=46 if indirect_call else 7
            nearby=ins[max(0,i-radius):min(len(ins),i+radius+1)]
            hits.append({"at":hex(z.address),"kind":"map_reference" if mapref
                else "source_course_lookup_caller" if indirect_call
                else "immediate_mask_shift",
                "instruction":f"{z.mnemonic} {z.op_str}",
                "context":[f"{q.address:#x} {q.mnemonic} {q.op_str}" for q in nearby]})
    return {"disassembled_count":n,
            "hits":hits,
            "map_reference_count":sum(x["kind"]=="map_reference" for x in hits),
            "terrain_lookup_call_count":sum(x["kind"]=="source_course_lookup_caller"
                                             for x in hits),
            "mask_or_shift_count":sum(x["kind"]=="immediate_mask_shift" for x in hits)}

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("--mapm",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    a=ap.parse_args()
    exe=a.exe.read_bytes()
    if hashlib.sha256(exe).hexdigest()!=SHA:
        raise SystemExit("wrong original Windows 1.014 source")
    summary=course_summary(a.mapm.read_bytes())
    x86=original_machine_refs(exe)
    report={"schema":"pixtee.original-mapm-upper-bits-and-tree-xrefs-investigation.v1",
      "exe_sha256":SHA,"original_course":"MAPM01 (private CI only)",
      "course":summary,"x86":x86,
      "limits":["MAPM high word patterns do not establish tree IDs.",
                "Static x86 xrefs are not proof of runtime collision activation.",
                "Original object classification and height geometry remain open."]}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print("Original MAPM high-six distribution:",summary["high_six_bits_distribution"])
    print("Tiles with high bits:",summary["nonzero_high_six_bit_tile_count"],
          "out of",summary["tiles_total"])
    print("Original x86 instruction count:",x86["disassembled_count"])
    print("Original upper-bit positions:",summary["upper_nonzero_tile_positions"])
    print("Original x86 resource references:",x86["map_reference_count"],
          "course lookup callers:",x86["terrain_lookup_call_count"],
          "mask/shift candidates:",x86["mask_or_shift_count"])
    for row in x86["hits"]:
        if row["kind"]!="immediate_mask_shift":
            print("SOURCE",row["kind"],row["at"])
            print("\n".join(row["context"]))
    print("Diagnostic only: no tree identity or collision parity signoff")
if __name__=="__main__":main()
