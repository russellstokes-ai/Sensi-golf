#!/usr/bin/env python3
"""Locate verified original v1.014 tournament/AI/player messages and xrefs.
This is a disassembly index into genuine original functions, not an AI port.
"""
from __future__ import annotations
import argparse,hashlib,json,struct
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from capstone.x86 import X86_OP_MEM,X86_OP_IMM
from pe_target_xrefs import parse_pe,code_sections
EXPECTED="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
PHRASES=[
 b"HDPlayer direction",b"HWPlayer computer",b"No luck with computer shot",
 b"Failed to take shot, in Computer_Quick_Swing",
 b"error in pick next player - tee off section",
 b"Play Tournament",b"Play Season",b"Continue Tournament",b"Continue Season",
 b"Sensible Open Tournament",b"World Open Tournament",
 b"Vigin Top Four Skins Game",b"WATER HAZARD",b"HOLE IN ONE!",
]
def main():
 p=argparse.ArgumentParser()
 p.add_argument("--exe",type=Path,required=True)
 p.add_argument("-o","--output",type=Path,required=True)
 a=p.parse_args();exe=a.exe.read_bytes()
 if hashlib.sha256(exe).hexdigest()!=EXPECTED:raise RuntimeError("Original exe hash mismatch")
 image_base,entry,sections=parse_pe(exe)
 entries=[]
 for word in PHRASES:
    off=exe.find(word)
    if off<0: entries.append({"keyword":word.decode(),"found":False});continue
    va=None
    for s in sections:
       if s["raw_offset"]<=off<s["raw_offset"]+s["raw_size"]:
          va=image_base+s["rva"]+off-s["raw_offset"]
          break
    entries.append({"keyword":word.decode(),"found":True,"source_offset":off,
                   "runtime_va":hex(va) if va else None,"references":[]})
 target={int(x["runtime_va"],16):x for x in entries if x.get("runtime_va")}
 md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
 for sec in code_sections(sections):
    b=exe[sec["raw_offset"]:sec["raw_offset"]+sec["raw_size"]]
    instructions=list(md.disasm(b,image_base+sec["rva"]))
    for idx,ins in enumerate(instructions):
       candidates=[]
       for op in ins.operands:
         if op.type==X86_OP_IMM:candidates.append(op.imm&0xffffffff)
         elif op.type==X86_OP_MEM:candidates.append(op.mem.disp&0xffffffff)
       for va in candidates:
         if va not in target:continue
         before=instructions[max(0,idx-4):idx+7]
         loc={"xref_va":hex(ins.address),
              "context":[{"va":hex(i.address),"op":i.mnemonic,"args":i.op_str}
                         for i in before]}
         if len(target[va]["references"])<8:target[va]["references"].append(loc)
 report={"schema":"pixtee.research.original-cpu-modes-callsite-index.v1",
    "original_sha256":EXPECTED,
    "scope":"Authentic PE strings and static references only; no claim entire AI or mode behaviours reconstructed",
    "terms":entries}
 a.output.parent.mkdir(parents=True,exist_ok=True)
 a.output.write_text(json.dumps(report,indent=2)+"\n")
 for x in entries:
   print(x["keyword"],"VA",x.get("runtime_va"),"xrefs",len(x.get("references",[])))
   for r in x.get("references",[])[:2]:
     print("  reference",r["xref_va"],"near",[(i["va"],i["op"],i["args"]) for i in r["context"]])
 print("PASS source-derived original AI, turn and tournament string/call-site index; AI not yet reimplemented")
if __name__=="__main__":main()
