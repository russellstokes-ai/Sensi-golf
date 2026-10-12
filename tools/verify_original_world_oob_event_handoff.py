#!/usr/bin/env python3
"""Original Windows 1.014 0x40CD00 OOB event handoff source parity.

Verifies numeric source writes and ESI at the *entry* of original
0x40CFE9 renderer/event subsystem. No claim about actual modal message
presentation or dismissal timing beyond this original handoff.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EIP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256
START=0x40CD00;STOP=0x40CFE9
SITES=(0x420fef,0x420544,0x420548)
def run(uc,row):
    b,u,v,esi=row
    uc.mem_write(SITES[0],bytes((b,)))
    uc.mem_write(SITES[1],struct.pack("<I",u))
    uc.mem_write(SITES[2],struct.pack("<I",v))
    uc.reg_write(UC_X86_REG_ESI,esi)
    hit=[]
    def hook(machine,address,size,user):
        if address==STOP:
            hit.append(address);machine.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try: uc.emu_start(START,0,count=25)
    finally:uc.hook_del(h)
    if hit!=[STOP]:
        raise RuntimeError(f"original OOB source event missed target: {row}")
    return (bytes(uc.mem_read(SITES[0],1))[0],
            struct.unpack("<I",bytes(uc.mem_read(SITES[1],4)))[0],
            struct.unpack("<I",bytes(uc.mem_read(SITES[2],4)))[0],
            uc.reg_read(UC_X86_REG_ESI)&0xffffffff)
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original v1.014 executable mismatch")
    rng=random.Random(0x40cd00)
    rows=[(rng.randrange(256),rng.randrange(2**32),
           rng.randrange(2**32),rng.randrange(2**32))
          for _ in range(1024)]
    proc=subprocess.run([str(a.probe)],
        input="".join(f"{i}\n" for i in range(len(rows))),
        text=True,capture_output=True,check=True)
    native=proc.stdout.splitlines()
    if len(native)!=len(rows):raise RuntimeError("native event count mismatch")
    uc,_=build_uc(a.exe)
    mismatch=[]
    for row,line in zip(rows,native):
        expected=run(uc,row)
        actual=tuple(map(int,line.split()))
        if actual!=expected:mismatch.append({"initial":row,"source":expected,"native":actual})
    report={
      "schema":"pixtee.original-world-OOB-UI-event-handoff-source.x86.v1",
      "executable_sha256":REFERENCE_SHA256,
      "cases":len(rows),"exact":len(rows)-len(mismatch),
      "source_start":"0x40CD00","source_end":"0x40CFE9",
      "expected_event_request":{"0x420FEF":1,"0x420544":70,
          "0x420548":65549,"ESI":4327024},
      "mismatches":mismatch[:25],
      "scope":"Verifies source event request numeric memory writes only; not actual GUI rendering, user acknowledgement, or completion timeline."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original world OOB event handoff: {report['exact']}/{len(rows)} exact")
    for m in mismatch[:5]:print("MISMATCH",m)
    if mismatch:raise SystemExit("Original OOB event handoff mismatch")
    print("PASS original x86 OOB display command/message event handoff")

if __name__=="__main__":main()
