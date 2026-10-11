#!/usr/bin/env python3
"""Zero-tolerance original 32-bit Windows v1.014 machine-code launch parity.
Runs authenticated original GOLFWIN.EXE via Unicorn and compares the
actual instruction execution to native recovered C++, for every club/lie
profile at original legal boundary accuracy values and several powers.
Original EXE cannot be replaced by a fake formula or a source table.
"""
import argparse,collections,hashlib,json,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"tools"))
from original_v1014_oracle import build_uc,club_values,profile_for,bounds_for,run_oracle,REFERENCE_SHA256

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("--probe",type=Path,required=True)
    ap.add_argument("-o","--output",required=True,type=Path)
    a=ap.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    uc,_=build_uc(a.exe)
    inputs=[]
    for club in range(13):
        for lie in range(10):
            prof=profile_for(uc,club,lie)
            low,high=bounds_for(uc,prof)
            for accuracy in sorted(set((low,63,high))):
                inputs.append((club,lie,105,accuracy,777))
            for power in (0,50):
                inputs.append((club,lie,power,63,0))
    p=subprocess.run([str(a.probe)],input="".join("%d %d %d %d %d\n"%x for x in inputs),
                     text=True,capture_output=True,check=True)
    lines=p.stdout.strip().splitlines()
    assert len(lines)==len(inputs),f"Native core returned {len(lines)} for {len(inputs)} original oracle inputs"
    mismatches=[]
    checked=0
    for inp,line in zip(inputs,lines):
        if line.startswith("ERR"):
            mismatches.append({"input":inp,"native_error":line})
            continue
        source=run_oracle(a.exe,*inp,ticks=0)
        oracle=source["samples"][0]
        expected=[oracle["x"],oracle["y"],oracle["height"],
                  oracle["vertical_force"],oracle["horizontal_force"],
                  oracle["direction"],oracle["swing_adjuster"],
                  oracle["adjusted_power"]]
        actual=list(map(int,line.split()))
        if actual!=expected:
            mismatches.append({"input":inp,"original":expected,"recovered":actual})
        else: checked+=1
    summary={
        "original_executable_sha256":REFERENCE_SHA256,
        "source":"Actual Unicorn execution of Windows GOLFWIN.EXE v1.014 0x40AD1D",
        "samples":len(inputs),"zero_tolerance_matches":checked,
        "mismatches":mismatches[:40],
        "all_clubs_and_lie_slots":True,
        "accuracy_samples":"each original legal minimum, centre and maximum",
        "power_samples":[0,50,105],
        "legal_reuse":"Reference testing only, not commercial licensing",
        "extreme_accuracy_beyond_original_profile_bounds":"UNVERIFIED and not falsely claimed"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(summary,indent=2)+"\n")
    print(f"Original-machine-code launch matrix: {checked}/{len(inputs)} exact matches")
    for issue in mismatches[:12]:print("MISMATCH",issue)
    if mismatches: raise SystemExit("Original launch vs recovered C++ had nonzero errors; DO NOT CLAIM PARITY")
    print("PASS 13 clubs x 10 lies at original legal accuracy profile edges / multiple powers, zero tolerance")
if __name__=="__main__":main()
