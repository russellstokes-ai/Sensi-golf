#!/usr/bin/env python3
"""Inspect original x86 event script negative command address table.

Reads numeric x86 target addresses only from verified original game,
never exports resource bytes. These are a map for independent command
recovery, NOT permission to assume command semantics from opcode names.
"""
from __future__ import annotations
import argparse,hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256
BASE=0x41F89C
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original Windows 1.014 SHA mismatch")
    uc,_=build_uc(a.exe)
    rows=[]
    for offset in range(0,32,4):
        ptr=struct.unpack("<I",bytes(uc.mem_read(BASE+offset,4)))[0]
        rows.append({"opcode":f"0x{0x8000+offset:04x}",
                     "table_offset_bytes":offset,
                     "source_target":hex(ptr)})
    result={
        "schema":"pixtee.original-animation-negative-script-command-xrefs.v1",
        "original_sha256":REFERENCE_SHA256,
        "source_table":hex(BASE),
        "commands":rows,
        "scope":"Pointer-table target inventory only. Command bodies and side effects require independent x86 execution; no UI or audio claimed."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(result,indent=2)+"\n")
    print("Original script negative command jump table:")
    for row in rows:print(row)
if __name__=="__main__":main()
