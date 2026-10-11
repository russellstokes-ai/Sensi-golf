#!/usr/bin/env python3
"""Run actual Windows v1.014 Welly meter subroutine at original VAs.
No graphics approximation: observe word variables after every original
CPU execution under controlled button input and club selection.
This is an isolated routine test, not full UI input-to-audio/game timing.
"""
import argparse,hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,STACK,SENTINEL
from unicorn.x86_const import UC_X86_REG_ESP

def w16(uc,a,v):uc.mem_write(a,struct.pack("<H",v&65535))
def w32(uc,a,v):uc.mem_write(a,struct.pack("<I",v&0xffffffff))
def w8(uc,a,v):uc.mem_write(a,bytes([v&255]))
def r16(uc,a):return struct.unpack("<H",bytes(uc.mem_read(a,2)))[0]
def r8(uc,a):return int(bytes(uc.mem_read(a,1))[0])
def execute(uc,entry):
    sp=STACK+0xf000-4
    w32(uc,sp,SENTINEL)
    uc.reg_write(UC_X86_REG_ESP,sp)
    uc.emu_start(entry,SENTINEL,count=4000)
def snapshot(uc,tick,club,pressed,entry):
    return {
      "tick":tick,"club":club,"pressed":pressed,"entry":hex(entry),
      "stage":r16(uc,0x41d58c),
      "meter_raw":r16(uc,0x41d592),
      "captured_power":r16(uc,0x41d59e),
      "accuracy_counter":r16(uc,0x41d5a0),
      "capture_flag":r16(uc,0x41d59a),
      "angle":r16(uc,0x41d594)
    }
def session(exe,club,pressed_on_rise,pressed_on_fall):
    uc,digest=build_uc(exe)
    w32(uc,0x41f3a8,PLAYER)
    w16(uc,PLAYER+0x28,club)
    for a in (0x41d592,0x41d594,0x41d59a,0x41d59e,0x41d598,0x41d5a0,0x41d59c):w16(uc,a,0)
    w16(uc,0x41d598,1)
    w16(uc,0x41d58c,1)
    w8(uc,0x42db56,0)
    records=[]
    fell_on=-1
    for t in range(125):
        phase=r16(uc,0x41d58c)
        if phase==0:break
        if phase==1:entry=0x40768b
        elif phase==2:entry=0x407787
        else: break
        pressed=(t==pressed_on_rise) or (fell_on>=0 and t-fell_on==pressed_on_fall)
        w8(uc,0x41f000,0x10 if pressed else 0)
        execute(uc,entry)
        records.append(snapshot(uc,t,club,pressed,entry))
        if phase==1 and r16(uc,0x41d58c)==2:
            fell_on=t
    return {"club":club,"press_rise_tick":pressed_on_rise,
        "press_falling_tick_offset":pressed_on_fall,"fall_started":fell_on,
        "samples":records}
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:raise RuntimeError("wrong original binary")
    scenarios=[]
    for club in (0,12):
      for rise,fall in ((12,10),(0,0),(80,69),(-1,-1)):
        scenarios.append(session(a.exe,club,rise,fall))
    report={"schema":"sensible-v1014-original-welly-machine-code-trace.v1",
      "original_exe_sha256":REFERENCE_SHA256,
      "method":"Unicorn executes raw original x86 subroutines 0x40768b and 0x407787",
      "scope":"isolated meter routine; not full timing/rendering/click-debounce ownership",
      "scenarios":scenarios}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    for x in scenarios:
        vals=x["samples"]
        print(f"club {x['club']} pressR {x['press_rise_tick']} pressF {x['press_falling_tick_offset']} "
              f"calls {len(vals)} firstFall {x['fall_started']} "
              f"first={vals[0] if vals else None} last={vals[-1] if vals else None}")
        print("  capture/phase snapshots:",[q for q in vals if q["pressed"] or q["meter_raw"] in (70,105) or q["stage"]==0][:8])
    print("PASS direct original machine-code Welly meter routine exercised")
if __name__=="__main__":main()
