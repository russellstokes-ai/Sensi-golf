#!/usr/bin/env python3
"""Research-only authenticated original event-script command CFG.

Traces the formerly unexamined 0x40C55A negative-frame command area.
This deliberately performs NO speculative native event implementation.
Only control-flow addresses and register/global access observations are
written; original executable, raw bytes, art, audio and course data
are never added to the repository or report.
"""
from __future__ import annotations
import argparse, hashlib, json
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from pe_target_xrefs import parse_pe

EXPECTED_SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
START=0x40C55A
END=0x40C5F0

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    args=p.parse_args()
    raw=args.exe.read_bytes()
    digest=hashlib.sha256(raw).hexdigest()
    if digest!=EXPECTED_SHA:
        raise SystemExit("Unrecognized original Windows v1.014 binary")
    base,_,sections=parse_pe(raw)
    region=next((s for s in sections if
        s["rva"]<=START-base and
        END-base<=s["rva"]+s["raw_size"]),None)
    if region is None:
        raise SystemExit("Animation command source region absent")
    begin=region["raw_offset"]+START-base-region["rva"]
    data=raw[begin:begin+END-START]
    dis=Cs(CS_ARCH_X86,CS_MODE_32)
    instructions=list(dis.disasm(data,START))
    if not instructions or instructions[0].address!=START:
        raise SystemExit("Original command branch did not disassemble")
    if len(instructions)<12:
        raise SystemExit("Unexpected short negative command source window")
    edges=[]
    field_accesses=[]
    for ins in instructions:
        name=ins.mnemonic.lower()
        if name.startswith("j") or name in ("call","ret","loop","loope","loopne"):
            target=ins.op_str if ins.op_str.startswith("0x") else "indirect_or_register"
            edges.append({"address":hex(ins.address),
                          "operation":name,"target":target})
        if any(token in ins.op_str.lower() for token in (
                "[esi", "[edi", "[ebp", "0x41f8", "0x41f3")):
            field_accesses.append({"address":hex(ins.address),
                                   "operation":name,
                                   "operand_class":"slot_or_global"})
    report={
        "schema":"pixtee.original-negative-animation-command-cfg-research.v1",
        "original_sha256":digest,
        "source_window":[hex(START),hex(END)],
        "disassembled_instructions":len(instructions),
        "branch_edges":edges,
        "slot_or_global_accesses":field_accesses,
        "status":"RESEARCH_ONLY_NOT_ORIGINAL_OPCODE_PARITY",
        "remaining":"Prove signed opcode dispatch via Unicorn, recover script operands and side effects, then compare C++ before enabling live player events"
    }
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+"\n")
    print("Authenticated original negative-script window:",
          report["source_window"],len(instructions),"instructions;",
          len(edges),"control transfers",
          len(field_accesses),"slot/global accesses")
    for e in edges:print(e["address"],e["operation"],e["target"])
    print("Original signed-command effects remain intentionally unimplemented")
if __name__=="__main__":
    main()
