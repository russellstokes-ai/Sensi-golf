#!/usr/bin/env python3
"""Focused original Windows v1.014 computer shot evaluation entry trace.

SHA-check authentic executable, decode a narrow 0x40C93E evaluation
entrypoint and nearby candidate-scoring branches. This is research
only; branch inspection does not prove original end-to-end CPU logic.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from pe_target_xrefs import parse_pe
SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
START=0x40C93E
END=0x40CA90
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",required=True,type=Path)
    ap.add_argument("-o","--output",required=True,type=Path)
    a=ap.parse_args()
    payload=a.exe.read_bytes()
    digest=hashlib.sha256(payload).hexdigest()
    if digest!=SHA:raise SystemExit("Incorrect Windows v1.014 source")
    base,_,sec=parse_pe(payload)
    s=next((q for q in sec if q["rva"]<=START-base and
        END-base<=q["rva"]+q["raw_size"]),None)
    if s is None:raise RuntimeError("original CPU evaluator not mapped")
    offset=s["raw_offset"]+START-base-s["rva"]
    dis=Cs(CS_ARCH_X86,CS_MODE_32)
    instructions=list(dis.disasm(payload[offset:offset+END-START],START))
    if not instructions or instructions[0].address!=START:
        raise RuntimeError("original CPU evaluator entry mismatch")
    rows=[{"address":hex(i.address),"instruction":i.mnemonic,
           "operands":i.op_str} for i in instructions]
    branches=[r for r in rows if r["instruction"].startswith("j") or
              r["instruction"] in ("call","ret")]
    report={"schema":"pixtee.original-cpu-evaluator-focused-cfg.v1",
      "original_sha256":SHA,"source_start":hex(START),
      "source_end":hex(END),
      "instructions":rows,"branches":branches,
      "status":"RESEARCH_ONLY__CPU_QUALITY_EVALUATION_NOT_RECOVERED"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print("Source CPU candidate evaluator",hex(START),
          "instructions",len(rows),"branches",len(branches))
    for r in rows[:110]:
        print(r["address"],r["instruction"],r["operands"])
    print("No guessed CPU quality scores or machine pointer execution")
if __name__=="__main__":main()
