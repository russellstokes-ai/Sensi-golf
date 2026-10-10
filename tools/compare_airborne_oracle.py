#!/usr/bin/env python3
"""Zero-tolerance original-machine-code vs portable live-shot comparator."""
from __future__ import annotations
import argparse,json
from pathlib import Path

FIELDS=("x","y","height","vertical_force","horizontal_force","direction","swing_adjuster","adjusted_power")

def compare(original,recovered):
    a=original.get("samples",[]); b=recovered.get("samples",[])
    mismatches=[]
    if len(a)!=len(b): mismatches.append({"kind":"sample_count","original":len(a),"recovered":len(b)})
    for left,right in zip(a,b):
        if left.get("tick")!=right.get("tick"):
            mismatches.append({"kind":"tick","original":left.get("tick"),"recovered":right.get("tick")}); continue
        for field in FIELDS:
            if int(left[field])!=int(right[field]):
                mismatches.append({"kind":"value","tick":left["tick"],"field":field,"original":left[field],"recovered":right[field]})
    return {"pass":not mismatches,"sample_count":min(len(a),len(b)),"fields":list(FIELDS),"tolerance":0,
            "mismatch_count":len(mismatches),"mismatches":mismatches[:100]}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument("original",type=Path); ap.add_argument("recovered",type=Path); ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()
    report=compare(json.loads(a.original.read_text()),json.loads(a.recovered.read_text()))
    text=json.dumps(report,indent=2)+"\n"
    if a.output:a.output.write_text(text)
    else:print(text,end="")
    return 0 if report["pass"] else 1
if __name__=="__main__": raise SystemExit(main())
