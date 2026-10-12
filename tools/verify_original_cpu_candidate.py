#!/usr/bin/env python3
"""Execute exact v1.014 x86 CPU candidate scan (0x40C78B..0x40C7C4).
An isolated selector, NOT original whole AI or difficulty policy.
"""
import argparse,hashlib,json,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EBX,UC_X86_REG_EDX,UC_X86_REG_ECX

def original(uc,rows):
    base=PLAYER+0x200
    for i,(a,b,m) in enumerate(rows):
        record=bytearray(14)
        struct.pack_into("<hh",record,0,a,b)
        struct.pack_into("<h",record,12,m)
        uc.mem_write(base+14*i,bytes(record))
    uc.reg_write(UC_X86_REG_EBX,base)
    uc.reg_write(UC_X86_REG_EDX,base)
    uc.reg_write(UC_X86_REG_ECX,len(rows))
    outcome={"stop":None}
    def hook(machine,address,size,user):
        if address in (0x40c7c4,0x40c883):
            outcome["stop"]=address
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try: uc.emu_start(0x40c78b,0,count=300)
    finally:uc.hook_del(token)
    if outcome["stop"]==0x40c883:return -1
    if outcome["stop"]!=0x40c7c4:
        raise RuntimeError(f"unrecognised original candidate control flow {outcome}")
    edx=uc.reg_read(UC_X86_REG_EDX)
    assert (edx-base)%14==0
    return (edx-base)//14

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    args=p.parse_args()
    assert hashlib.sha256(args.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    uc,_=build_uc(args.exe)
    cases=[]
    for count in range(1,7):
        for mask in range(1<<count):
            for secondary in (0,1,2):
                rows=[(-1 if (mask>>i)&1 else i,i%2-1 if secondary==1 else
                       (1 if secondary==2 else 0),(count-i)*11)
                      for i in range(count)]
                cases.append(rows)
    inp="".join(str(len(rows))+" "+
                " ".join(f"{a} {b} {m}" for a,b,m in rows)+"\n" for rows in cases)
    response=subprocess.run([str(args.probe)],input=inp,text=True,capture_output=True,check=True)
    outputs=response.stdout.strip().splitlines()
    assert len(outputs)==len(cases)
    failed=[]
    for idx,(rows,line) in enumerate(zip(cases,outputs)):
        ref=original(uc,rows)
        got=int(line)
        if ref!=got:failed.append({"index":idx,"input":rows,"original":ref,"native":got})
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps({"schema":"pixtee.original-cpu-candidate-scan.v1",
       "original_sha256":REFERENCE_SHA256,"test_vectors":len(cases),
       "exact_matches":len(cases)-len(failed),"mismatches":failed[:100],
       "scope":"candidate priority scan only; not original full computer-shot planner"},indent=2)+"\n")
    print(f"Original CPU candidate x86 scan: {len(cases)-len(failed)}/{len(cases)}")
    for x in failed[:8]:print("MISMATCH",x)
    if failed:raise SystemExit("Original CPU candidate source parity failed")
    print("PASS original x86 candidate traversal; no claim of full CPU opponent AI")
if __name__=="__main__":main()
