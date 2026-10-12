#!/usr/bin/env python3
"""Research original v1.014 CPU penalty lookup table initialization.

Scoring 0x40CA04 reads WORD[0x41FADF + 2*source_hit_code].
This scans source references and reports the original loader's static
memory snapshot, NOT a claim that these are runtime/course-specific
weights. Source binary only in transient CI, not committed.
"""
from __future__ import annotations
import argparse,hashlib,json,struct,sys
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256
from pe_target_xrefs import parse_pe
TABLE=0x41fadf
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    a=ap.parse_args()
    data=a.exe.read_bytes()
    if hashlib.sha256(data).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("not original v1.014")
    uc,_=build_uc(a.exe)
    image_words=list(struct.unpack("<32H",bytes(uc.mem_read(TABLE,64))))
    base,_,secs=parse_pe(data)
    source=next((s for s in secs if s["rva"]<=0x40ca04-base<
        s["rva"]+max(s["raw_size"],s["virtual_size"])),None)
    if source is None:raise RuntimeError("source executable section unknown")
    ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(
       data[source["raw_offset"]:source["raw_offset"]+source["raw_size"]],
       base+source["rva"]))
    candidates=[]
    for i,op in enumerate(ins):
        if "0x41fadf" in op.op_str.lower():
            candidates.append({"at":hex(op.address),
                "mnemonic":op.mnemonic+" "+op.op_str,
                "nearby":[f"{x.address:#x} {x.mnemonic} {x.op_str}"
                    for x in ins[max(0,i-25):min(len(ins),i+35)]]})
    # Full 32 source words included for local source diagnosis only;
    # zero means *pre-initialized* table, not a proved scoring policy.
    report={"schema":"pixtee.original-CPU-penalty-table-lifecycle-research.v1",
       "sha256":REFERENCE_SHA256,
       "source_scoring_read":"0x40CA04 -> 0x41FADF+2*contact_code",
       "pre_run_static_table_words":image_words,
       "pre_run_nonzero_count":sum(x!=0 for x in image_words),
       "absolute_references":candidates,
       "limits":"Static initial table is not proof of original course/mode initialization; dynamic writers and full original CPU candidate outcome remain unverified."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print("Original CPU penalty static table 32 WORDs:",image_words)
    print("Initial nonzero WORDs:",report["pre_run_nonzero_count"])
    print("Original direct global xrefs:",len(candidates))
    for ref in candidates:
        print("REFERENCE",ref["at"],ref["mnemonic"])
        print("\n".join(ref["nearby"]))
    print("DIAGNOSTIC ONLY: original runtime penalty table lifecycle not proved")
if __name__=="__main__":main()
