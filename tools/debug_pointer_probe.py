#!/usr/bin/env python3
"""Recover live-variable pointers stored immediately after Sensible Golf debug labels.

The v1.014 Windows build contains an internal debug table where many human-readable
labels are followed by a 32-bit little-endian address. This tool records those
addresses and finds code instructions that reference the live variables.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path

from executable_probe import parse_executable
from pe_string_xrefs import DEFAULT_TARGETS, parse_instruction_address


def pointer_after_label(data: bytes, label: str) -> list[dict[str, int]]:
    needle=label.encode("ascii")+b"\0"
    results=[]
    start=0
    while True:
        pos=data.find(needle,start)
        if pos < 0:
            break
        ptr_off=pos+len(needle)
        if ptr_off+4 <= len(data):
            ptr=int.from_bytes(data[ptr_off:ptr_off+4],"little")
            results.append({"label_offset":pos,"pointer_offset":ptr_off,"pointer":ptr})
        start=pos+1
    return results


def find_code_refs(lines: list[str], address: int, context: int=6) -> list[dict[str,object]]:
    token=f"{address:x}"
    pattern=re.compile(rf"(?<![0-9a-f])(?:0x)?0*{re.escape(token)}(?![0-9a-f])",re.I)
    hits=[]
    for idx,line in enumerate(lines):
        if not pattern.search(line):
            continue
        inst=parse_instruction_address(line)
        start=max(0,idx-context)
        end=min(len(lines),idx+context+1)
        hits.append({
            "instruction_address":inst,
            "line":line.rstrip(),
            "context":[x.rstrip() for x in lines[start:end]],
        })
    return hits


def analyse(executable: Path, targets: list[str]|None=None, context:int=6, objdump:str="objdump")->dict:
    data=executable.read_bytes()
    probe=parse_executable(data)
    if probe.get("format")!="PE":
        raise ValueError("debug_pointer_probe requires a PE executable")

    proc=subprocess.run(
        [objdump,"-d","-Mintel",str(executable)],
        check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,
    )
    lines=proc.stdout.splitlines()

    records=[]
    for label in targets or DEFAULT_TARGETS:
        occurrences=[]
        for row in pointer_after_label(data,label):
            pointer=row["pointer"]
            occurrences.append({
                **row,
                "pointer_hex":f"0x{pointer:08x}",
                "code_references":find_code_refs(lines,pointer,context),
            })
        records.append({"label":label,"occurrences":occurrences})

    return {
        "executable":executable.name,
        "sha256":probe["sha256"],
        "records":records,
    }


def main()->int:
    parser=argparse.ArgumentParser(description="Recover Sensible Golf debug variable pointers")
    parser.add_argument("executable",type=Path)
    parser.add_argument("--target",action="append",default=[])
    parser.add_argument("--context",type=int,default=6)
    parser.add_argument("--objdump",default="objdump")
    parser.add_argument("-o","--output",type=Path)
    args=parser.parse_args()
    try:
        report=analyse(args.executable,args.target or None,args.context,args.objdump)
    except (OSError,ValueError,subprocess.CalledProcessError) as exc:
        parser.error(str(exc))
    text=json.dumps(report,indent=2)+"\n"
    if args.output:
        args.output.write_text(text,encoding="utf-8")
    else:
        print(text,end="")
    return 0


if __name__=="__main__":
    raise SystemExit(main())
