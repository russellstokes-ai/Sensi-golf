#!/usr/bin/env python3
"""Zero-tolerance comparator for full original-vs-recovered shot traces."""
from __future__ import annotations
import argparse,json
from pathlib import Path

FIELDS=("x","y","height","vertical_force","horizontal_force","direction","swing_adjuster","adjusted_power")

def compare(original,recovered):
    left=original.get("samples",[]); right=recovered.get("samples",[])
    mismatches=[]
    if len(left)!=len(right):
        mismatches.append({"kind":"sample_count","original":len(left),"recovered":len(right)})
    for a,b in zip(left,right):
        if a.get("tick")!=b.get("tick"):
            mismatches.append({"kind":"tick","original":a.get("tick"),"recovered":b.get("tick")})
            continue
        for field in FIELDS:
            if int(a[field])!=int(b[field]):
                mismatches.append({"kind":"value","tick":a["tick"],"field":field,"original":a[field],"recovered":b[field]})
    for event in ("landing","rest"):
        a=original.get("events",{}).get(event); b=recovered.get("events",{}).get(event)
        if a!=b:
            mismatches.append({"kind":"event","event":event,"original":a,"recovered":b})
    original_holed=bool(original.get("events",{}).get("holed",False))
    recovered_holed=bool(recovered.get("events",{}).get("holed",False))
    if original_holed!=recovered_holed:
        mismatches.append({"kind":"event","event":"holed","original":original_holed,"recovered":recovered_holed})
    if original.get("surface_code")!=recovered.get("surface_code"):
        mismatches.append({"kind":"surface_code","original":original.get("surface_code"),"recovered":recovered.get("surface_code")})
    return {"pass":not mismatches,"sample_count":min(len(left),len(right)),"fields":list(FIELDS),"tolerance":0,
            "mismatch_count":len(mismatches),"mismatches":mismatches[:100]}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument("original",type=Path); ap.add_argument("recovered",type=Path); ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args(); report=compare(json.loads(a.original.read_text()),json.loads(a.recovered.read_text()))
    text=json.dumps(report,indent=2)+"\n"
    if a.output:a.output.write_text(text)
    else:print(text,end="")
    return 0 if report["pass"] else 1
if __name__=="__main__": raise SystemExit(main())
