#!/usr/bin/env python3
"""Original Windows v1.014 special green code9 early vs code10 late.

Run verified original code 0x40A403 (pre-update code-9 early stop with
extra stroke counters) or 0x40A40B (putter later code 9/10 direct stop,
without those counters), to original 0x40CD00 UI handoff, with only
irrelevant render and audio CALLEE bodies skipped inside Unicorn memory.
The code9 first-entry branch has a separate source-machine predicate
oracle; no claim for full tree/object interactions or full UI handling.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_ESP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,BALL,PLAYER,STACK,w16,ru16
)
START={0:0x40A40B,1:0x40A403}
STOP=0x40CD00
UI_AND_SFX=(0x40A415,0x40A427)
GATE=0x41F883

def vectors():
    rng=random.Random(0x40a403)
    special=(0,1,2,3,4,5,7,8,0x8000,0x8004,0xfffe,0xffff)
    rows=[]
    for early in (0,1):
        for flag in special:
            for stroke in (0,1,100,65534,65535):
                for total in (0,1,65535):
                    rows.append((early,flag,stroke,total,
                                 rng.randrange(65536),rng.randrange(65536)))
    for _ in range(4500):
        rows.append((rng.randrange(2),rng.choice(special+(rng.randrange(65536),)),
                     rng.randrange(65536),rng.randrange(65536),
                     rng.randrange(65536),rng.randrange(65536)))
    return rows

def original(uc,row):
    early,flags,stroke,total,pause,gate=row
    uc.mem_write(PLAYER,b"\0"*0x100)
    uc.mem_write(BALL,b"\0"*0x100)
    w16(uc,PLAYER,flags)
    w16(uc,PLAYER+0x52,stroke)
    w16(uc,PLAYER+0x56,total)
    w16(uc,BALL+0x1c,pause)
    w16(uc,GATE,gate)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    hit=[]
    def hook(m,addr,size,user):
        if addr==STOP:hit.append(addr);m.emu_stop()
    t=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START[early],0,count=100)
    finally:uc.hook_del(t)
    if hit!=[STOP]:raise RuntimeError("unexpected original code9/10 terminal continuation")
    return (ru16(uc,PLAYER),ru16(uc,PLAYER+0x52),
            ru16(uc,PLAYER+0x56),ru16(uc,BALL+0x1c),
            ru16(uc,GATE))
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original source hash mismatch")
    rows=vectors()
    proc=subprocess.run([str(a.probe)],input="".join(
      " ".join(map(str,r))+"\n" for r in rows
    ),text=True,capture_output=True,check=True)
    lines=proc.stdout.splitlines()
    if len(lines)!=len(rows):raise RuntimeError("native green stop count mismatch")
    uc,_=build_uc(a.exe)
    for addr in UI_AND_SFX:
        raw=bytes(uc.mem_read(addr,5))
        if raw[0]!=0xe8:
            raise RuntimeError(f"unexpected original CALL at {addr:#x}")
        uc.mem_write(addr,b"\x90"*5)
    mismatches=[];early=0;late=0
    for row,line in zip(rows,lines):
        original_row=original(uc,row)
        native=tuple(int(x) for x in line.split())
        if row[0]:early+=1
        else:late+=1
        if original_row!=native:
            mismatches.append({"case":row,"original":original_row,"native":native})
    rep={
        "schema":"pixtee.original-special-green-early-code9-late-code10-accounting-x86.v1",
        "original_sha256":REFERENCE_SHA256,
        "cases":len(rows),"exact":len(rows)-len(mismatches),
        "original_early_code9_0x40A403":early,
        "original_late_code9_or10_0x40A40B":late,
        "ui_and_sfx_source_call_sites_stubbed_in_emulator_only":[hex(x) for x in UI_AND_SFX],
        "mismatches":mismatches[:40],
        "scope":"Original source terminal counter/flag/pause/event-gate only; no original visual UI, mode scheduling or tree interactions."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(rep,indent=2)+"\n")
    print(f"Original early-code9 vs late-code10 terminal: {rep['exact']}/{len(rows)} exact")
    print("Early source paths",early,"late direct source paths",late)
    for x in mismatches[:5]:print("MISMATCH",x)
    if mismatches:raise SystemExit("Original special-green accounting divergence")
    print("PASS exact original special green code9 extra-stroke vs code10 direct stop")
if __name__=="__main__":main()
