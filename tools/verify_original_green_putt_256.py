#!/usr/bin/env python3
"""Original Sensible Golf v1.014 executed per-tick green putting against
recovered native C++; all 16 directions x all 16 magnitudes, real code.
No sprite/green topology imported. Original oracle is independent of C++.
"""
import argparse,hashlib,json,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_putter_oracle import run_putter
from original_v1014_oracle import REFERENCE_SHA256
def main():
 p=argparse.ArgumentParser()
 p.add_argument("--exe",required=True,type=Path)
 p.add_argument("--probe",required=True,type=Path)
 p.add_argument("-o","--output",required=True,type=Path)
 a=p.parse_args()
 assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
 checks=[(direction*256,mag,30) for direction in range(16) for mag in range(16)]
 native=subprocess.run([str(a.probe)],input="".join("%d %d %d\n"%c for c in checks),
                       capture_output=True,text=True,check=True)
 rows=native.stdout.strip().splitlines()
 assert len(rows)==len(checks),(len(rows),len(checks))
 mismatches=[];equal=0
 for params,row in zip(checks,rows):
   direction,mag,power=params
   if row.startswith("ERR"):
     mismatches.append({"params":params,"native_error":row});continue
   oracle=run_putter(a.exe,power,63,0,256,31,direction,mag,1)
   endpoint=oracle["events"]["rest"]
   expected=[endpoint["x"],endpoint["y"],endpoint["tick"]]
   actual=list(map(int,row.split()))
   if actual!=expected:
     mismatches.append({"params":params,"original":expected,"recovered":actual})
   else:equal+=1
 report={"source":"actual Windows Sensible Golf 1.014 original putt machine code",
   "exe_sha256":REFERENCE_SHA256,
   "vector_count":len(checks),"exact_rest_endpoint_matches":equal,
   "mismatches":mismatches[:80],
   "scope":"16 arrows x 16 magnitudes, club 12, power 30, accuracy 63, flat green H4; not all clubs/powers/course modes"}
 a.output.parent.mkdir(parents=True,exist_ok=True)
 a.output.write_text(json.dumps(report,indent=2)+"\n")
 print(f"Original vs recovered full 16x16 green putt matrix: {equal}/{len(checks)} zero-tolerance")
 for x in mismatches[:10]:print("MISMATCH",x)
 if mismatches:raise SystemExit("Green slope putt parity mismatch; not complete")
 print("PASS original-game executable and recovered C++ agree for all 256 slope arrows/strengths")
if __name__=="__main__":main()
