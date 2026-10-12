#!/usr/bin/env python3
"""Focused original x86 continuation study for CPU evaluator and object gates.

No claim that the original graphics-object updater is equivalent to
tree collision. Prints bounded original executable instructions from
SHA-verified Windows v1.014, with no game bytes/graphics archived.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from pe_target_xrefs import parse_pe
SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
RANGES=[
    ("original_cpu_score_candidate_pick",0x40CA8E,0x40CB00),
    ("original_cpu_score_helper",0x40B998,0x40BA58),
    ("original_scene_object_dispatch_probe",0x40C2D0,0x40C365),
]
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    data=a.exe.read_bytes()
    if hashlib.sha256(data).hexdigest()!=SHA:
        raise SystemExit("Original Windows v1.014 checksum mismatch")
    base,_,sections=parse_pe(data)
    dis=Cs(CS_ARCH_X86,CS_MODE_32)
    out=[]
    for name,start,stop in RANGES:
        section=next((s for s in sections if s["rva"]<=start-base and
                      stop-base<=s["rva"]+s["raw_size"]),None)
        if not section:raise RuntimeError("source not mapped: "+name)
        off=section["raw_offset"]+start-base-section["rva"]
        ins=[{"addr":hex(x.address),"instruction":x.mnemonic,
              "operands":x.op_str}
             for x in dis.disasm(data[off:off+stop-start],start)]
        if not ins or ins[0]["addr"]!=hex(start):
            raise RuntimeError("original entry not decoded: "+name)
        out.append({"name":name,"start":hex(start),"end":hex(stop),
                    "instructions":ins})
        print("SOURCE",name,hex(start),"instructions",len(ins))
        for x in ins[:90]:
            print(x["addr"],x["instruction"],x["operands"])
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps({
        "schema":"pixtee.original-cpu-and-object-narrow-source-probes.v1",
        "original_sha256":SHA,
        "regions":out,
        "status":"SOURCE_RESEARCH_ONLY_OBJECT_IDENTITY_NOT_ESTABLISHED"
    },indent=2)+"\n")
if __name__=="__main__":main()
