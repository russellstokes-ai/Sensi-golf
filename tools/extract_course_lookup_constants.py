#!/usr/bin/env python3
"""Extract the 16-word course lookup direction table from GOLFWIN v1.014."""

from __future__ import annotations
import argparse,json,struct
from pathlib import Path
from pe_target_xrefs import parse_pe,rva_to_file_offset

TABLE_VA=0x41D9DF
COUNT=16

def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()
    data=a.exe.read_bytes()
    image,_,sections=parse_pe(data)
    off=rva_to_file_offset(TABLE_VA-image,sections)
    if off is None: raise SystemExit("table VA is not file-backed")
    vals=list(struct.unpack_from("<16H",data,off))
    report={"table_va":TABLE_VA,"values":vals}
    text=json.dumps(report,indent=2)+"\n"
    if a.output:a.output.write_text(text,encoding="utf-8")
    else:print(text,end="")
    return 0
if __name__=="__main__":raise SystemExit(main())
