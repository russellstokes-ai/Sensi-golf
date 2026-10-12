#!/usr/bin/env python3
"""Locate original Windows v1.014 Welly-o-Meter *outer* caller and press edges.

The previously verified per-callback producer is NOT a proof that a user
button press should be sampled at every rendered frame. This source
audit locates all cross-function CALLs into the original rising/falling
producers and reads/writes of the original input byte/global state,
plus surrounding scheduling branch instructions. Nothing is guessed.
"""
from __future__ import annotations
import argparse,hashlib,json
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from pe_target_xrefs import parse_pe
ORIGINAL="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
PRODUCERS=("0x40768b","0x407787")
STATE=("0x41d58c","0x41d592","0x41d596","0x41d59a","0x41d59e","0x41d5a0","0x41f000")
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    exe=a.exe.read_bytes()
    if hashlib.sha256(exe).hexdigest()!=ORIGINAL:
        raise SystemExit("original Sensible Golf v1.014 checksum mismatch")
    base,_,secs=parse_pe(exe)
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    records=[]
    for sec in secs:
        rva=0x40768b-base
        if not(sec["rva"]<=rva<sec["rva"]+
               max(sec["virtual_size"],sec["raw_size"])):
            continue
        ins=list(md.disasm(
            exe[sec["raw_offset"]:sec["raw_offset"]+sec["raw_size"]],
            base+sec["rva"]))
        for k,i in enumerate(ins):
            tokens=i.op_str.lower()
            knd="producer_call" if i.mnemonic=="call" and tokens in PRODUCERS else (
                "meter_state_reference" if any(addr in tokens for addr in STATE)
                else "")
            if not knd:continue
            nearby=ins[max(0,k-18):min(len(ins),k+24)]
            records.append({"address":hex(i.address),"kind":knd,
               "instruction":i.mnemonic+" "+i.op_str,
               "context":[f"{z.address:#x} {z.mnemonic} {z.op_str}"
                          for z in nearby]})
    out={"schema":"pixtee.original-welly-outer-scheduler-xref.v1",
        "source_sha256":ORIGINAL,"refs":records,
        "producer_call_count":sum(r["kind"]=="producer_call" for r in records),
        "meter_global_reference_count":sum(r["kind"]=="meter_state_reference" for r in records),
        "scope":"Static source CALL and input/state-xref audit. No timer interval, debounce or event-loop cadence claimed absent dynamic original execution."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print("Original Welly producer callers:",out["producer_call_count"],
          "original meter globals referenced:",out["meter_global_reference_count"])
    for record in records:
        if record["kind"]=="producer_call":
            print("\nSOURCE ORIGINAL PRODUCER CALL",record["instruction"],
                  "@",record["address"])
            print("\n".join(record["context"]))
    for record in records:
        if record["kind"]=="meter_state_reference":
            print("GLOBAL",record["address"],record["instruction"])
    print("Source-only scheduler inventory; no timing signoff")
if __name__=="__main__":main()
