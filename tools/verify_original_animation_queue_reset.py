#!/usr/bin/env python3
"""Exact Windows Sensible Golf v1.014 original animation queue reset oracle.

Source x86 0x40C5C0..0x40C5E8 clears all ten original 20-byte event
slots via STOSW. Tests genuine machine instructions, not inferred
deserialization or copied binary payloads. Original executable is only
present inside transient SHA-verified CI.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,STACK
ENTRY=0x40C5C0
STOP=0x40C5E8
SLOTS=0x41F8F0

def cases():
    rng=random.Random(0x40c5c0)
    rows=[]
    for special in (0,1,0xffff,0xffffffff):
        row=[]
        for i in range(10):
            row.append(tuple(special & (0xffffffff if j>=6 else 0xffff)
                             for j in range(8)))
        rows.append(row)
    for _ in range(1296):
        row=[]
        for i in range(10):
            row.append(tuple(rng.getrandbits(32 if j>=6 else 16)
                             for j in range(8)))
        rows.append(row)
    return rows

def source(uc,row):
    packed=b"".join(struct.pack("<6H2I",*slot) for slot in row)
    uc.mem_write(SLOTS,packed)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    stops=[]
    def hook(machine,address,size,user):
        if address==STOP:
            stops.append(address)
            machine.emu_stop()
    t=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(ENTRY,0,count=350)
    finally:uc.hook_del(t)
    if stops!=[STOP]:raise RuntimeError("original queue reset did not return")
    raw=bytes(uc.mem_read(SLOTS,200))
    return tuple(y for i in range(10) for y in
                 struct.unpack_from("<6H2I",raw,i*20))

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("--probe",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    a=ap.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Source binary mismatch")
    rows=cases()
    result=subprocess.run([str(a.probe)],
       input="".join(" ".join(str(n) for slot in case for n in slot)+"\n"
                     for case in rows),
       capture_output=True,text=True,check=True)
    native=[tuple(int(x) for x in line.split())
            for line in result.stdout.splitlines()]
    if len(native)!=len(rows):raise RuntimeError("missing native slots")
    uc,_=build_uc(a.exe)
    mismatches=[]
    for idx,(case,got) in enumerate(zip(rows,native)):
        src=source(uc,case)
        if src!=got:mismatches.append({"case":idx,"original":src[:16],"native":got[:16]})
    report={"schema":"pixtee.original-ten-slot-queue-reset-x86.v1",
       "original_sha256":REFERENCE_SHA256,
       "source":"0x40C5C0..0x40C5E8",
       "reset_cases":len(rows),
       "exact":len(rows)-len(mismatches),
       "individual_slot_field_checks":len(rows)*10*8,
       "mismatches":mismatches[:16],
       "scope":"Original full event-slot reset, not callback effects, sprite/host tick scheduling, other game systems."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original event queue reset: {report['exact']}/{len(rows)} exact")
    if mismatches:
        for m in mismatches[:3]:print("MISMATCH",m)
        raise SystemExit("original queue reset mismatch")
    print("PASS all source-machine ten-slot fields match recovered native reset")
if __name__=="__main__":main()
