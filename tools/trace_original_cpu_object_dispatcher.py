#!/usr/bin/env python3
"""Trace original Windows v1.014 CPU simulator and object collision callers.
Read-only, SHA-checked analysis; no commercial game data committed.
Focus on call graph and state data-flow needed before implementing the
missing original CPU trajectory candidate evaluation and tree hits.
"""
import argparse,hashlib,json
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from pe_target_xrefs import parse_pe
EXE_SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
WINDOWS=[
    ("original_cpu_simulation",0x40c5f0,0x40c890),
    ("original_cpu_shot_evaluator",0x40c880,0x40cb20),
    ("alternate_original_ball_tick",0x40cb00,0x40cd00),
    ("original_flag_object_update",0x40b930,0x40ba80),
    ("original_code9_object_rectangle_checker",0x40c4c0,0x40c5c0),
    ("original_trees_sprite_object_resource",0x40c2d0,0x40c4c0),
    ("original_ball_pre_collision",0x40a2f0,0x40a560),
    ("original_player_tick_input",0x40adf0,0x40b060),
]
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    a=ap.parse_args()
    exe=a.exe.read_bytes()
    if hashlib.sha256(exe).hexdigest()!=EXE_SHA:
        raise SystemExit("not authentic original Windows v1.014")
    base,_,sections=parse_pe(exe)
    md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=False
    chunks=[]
    for name,first,last in WINDOWS:
        region=next((sec for sec in sections if sec["rva"]<=first-base
            and last-base<=sec["rva"]+max(sec["raw_size"],sec["virtual_size"])),None)
        if not region: raise RuntimeError("original machine range not mapped "+name)
        offset=region["raw_offset"]+first-base-region["rva"]
        code=exe[offset:offset+last-first]
        ins=[{"address":hex(i.address),"asm":i.mnemonic+" "+i.op_str}
             for i in md.disasm(code,first)]
        calls=[ins[k] for k in range(len(ins)) if ins[k]["asm"].startswith("call ")]
        chunks.append({"name":name,"first":hex(first),"last":hex(last),
                       "instructions":ins,"calls":calls})
    out={"schema":"pixtee.original-cpu-object-source-windows.v1",
        "sha256":EXE_SHA,"regions":chunks,
        "limits":"Static disassembly is not proof of complete CPU simulation or object geometry."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    for ch in chunks:
        print("\n== ORIGINAL",ch["name"],ch["first"],ch["last"],"==")
        for row in ch["instructions"]:
            print(row["address"],row["asm"])
if __name__=="__main__":main()
