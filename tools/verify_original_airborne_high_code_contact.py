#!/usr/bin/env python3
"""Original Windows v1.014 0x40A5D3 entry versus C++ airborne high-code path.

Source 0x40A5B3 excluded codes <=10 prior to this tested entry. The
authentic executable's sprite/host-audio CALL instructions are stubbed
in Unicorn memory only; branch and ball/player state are executed raw.
"""
from __future__ import annotations
import argparse,hashlib,json,random,subprocess,sys
from itertools import product
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import verify_original_special_terminal_terrain as source
from original_v1014_oracle import build_uc,REFERENCE_SHA256

START=0x40A5D3

def vectors():
    codes=(11,35,36,49,50,51,60,65535)
    flags=(0,1,2,0x8000,0x8001,0xffff)
    clubs=(0,4,12)
    heights=(-2147483648,-65536,0,199<<16,200<<16,
             (200<<16)|0xffff,201<<16,2147483647)
    rows=list(product(codes,clubs,flags,(-65536,0,65536),(0,65536),heights))
    rng=random.Random(10143551)
    for _ in range(4200):
        rows.append((
            rng.choice(codes),
            rng.choice(clubs),
            rng.choice(flags),
            rng.randint(-2147483648,2147483647),
            rng.randint(-2147483648,2147483647),
            rng.choice(heights+[rng.randint(-2147483648,2147483647)])
        ))
    return rows

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("incorrect original GOLFWIN.EXE")
    rows=vectors()
    cmd=subprocess.run([str(a.probe)],
         input="".join(" ".join(map(str,r))+"\n" for r in rows),
         text=True,capture_output=True,check=True)
    lines=cmd.stdout.splitlines()
    if len(lines)!=len(rows):raise RuntimeError("native original airborne case-count mismatch")
    uc,_=build_uc(a.exe)
    for address in source.PATCH_CALLS:
        raw=bytes(uc.mem_read(address,5))
        if raw[0]!=0xE8:raise RuntimeError(f"unexpected executable CALL {address:#x}")
        uc.mem_write(address,b"\x90"*5)
    source.START=START
    bad=[];exits={};events={};codes={}
    for row,line in zip(rows,lines):
        expected,destination=source.original(uc,row)
        actual=tuple(map(int,line.split()))
        exits[destination]=exits.get(destination,0)+1
        events[str(expected[-1])]=events.get(str(expected[-1]),0)+1
        codes[str(row[0])]=codes.get(str(row[0]),0)+1
        if actual!=expected:
            bad.append({"input":row,"original":expected,
                        "recovered":actual,"destination":destination})
    evidence={
        "schema":"pixtee.original-airborne-high-terrain-x86.v1",
        "original_sha256":REFERENCE_SHA256,
        "total":len(rows),"exact":len(rows)-len(bad),
        "original_branch_exits":exits,
        "original_event_call_arguments":events,
        "tested_numeric_landing_codes":codes,
        "source_entry":"0x40A5D3 (after upstream code <=10 exclusions)",
        "source_calls_stubbed_in_emulator_only":[hex(a) for a in source.PATCH_CALLS],
        "mismatches":bad[:50],
        "scope":"Original airborne high-terrain event and rebound branch. Does not verify course-object lookup, drawing, audio callback, real tree objects or complete movement."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(evidence,indent=2)+"\n")
    print(f"Original airborne high-terrain route: {evidence['exact']}/{len(rows)} exact")
    print("Source exits:",exits,"event arguments:",events)
    for b in bad[:8]:print("MISMATCH",b)
    if bad:raise SystemExit("original airborne high-terrain mismatch")
    print("PASS original airborne code>10 numeric-terrain path versus native")

if __name__=="__main__":main()
