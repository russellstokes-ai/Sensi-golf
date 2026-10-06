#!/usr/bin/env python3
"""Launch GOLFWIN.EXE as a child and sample its live v1.014 ball memory.

Linux ptrace_scope normally permits a parent to inspect its direct child, which
makes this suitable for CI golden-master capture without weakening runner
security settings.
"""

from __future__ import annotations
import argparse
import json
import os
import struct
import subprocess
import time
from pathlib import Path

CURRENT_BALL_PTR=0x41F3AC

def read_at(mem,addr,size):
    mem.seek(addr); b=mem.read(size)
    if len(b)!=size: raise OSError(f"short read {len(b)}/{size} at {addr:#x}")
    return b
def u32(mem,a): return struct.unpack("<I",read_at(mem,a,4))[0]
def i32(mem,a): return struct.unpack("<i",read_at(mem,a,4))[0]
def u16(mem,a): return struct.unpack("<H",read_at(mem,a,2))[0]

def state(mem,tick):
    ball=u32(mem,CURRENT_BALL_PTR)
    row={"tick":tick,"ball_ptr":ball}
    if not ball: return row
    row.update({
      "x":i32(mem,ball+0x00),"y":i32(mem,ball+0x04),
      "vertical_force":i32(mem,ball+0x08),
      "horizontal_force":i32(mem,ball+0x0C),
      "direction":u32(mem,ball+0x10),"height":i32(mem,ball+0x14),
      "distance_to_hole":i32(mem,ball+0x18),"pause":i32(mem,ball+0x1C),
      "surface":u16(mem,ball+0x1E),
      "terrain_direction":u16(mem,ball+0x26),
      "terrain_magnitude":u16(mem,ball+0x2A),
    })
    return row

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    ap.add_argument("-o","--output",type=Path,required=True)
    ap.add_argument("--duration-s",type=float,default=12)
    ap.add_argument("--interval-ms",type=float,default=20)
    ap.add_argument("--stdout",type=Path,default=Path("/tmp/golfwin.stdout"))
    ap.add_argument("--stderr",type=Path,default=Path("/tmp/golfwin.stderr"))
    args=ap.parse_args()
    exe=args.exe.resolve()
    env=os.environ.copy()
    args.stdout.parent.mkdir(parents=True,exist_ok=True)
    args.stderr.parent.mkdir(parents=True,exist_ok=True)
    samples=[]
    errors=[]
    with args.stdout.open("wb") as out, args.stderr.open("wb") as err:
        proc=subprocess.Popen(["wine",str(exe)],cwd=str(exe.parent),env=env,stdout=out,stderr=err)
        pid=proc.pid
        mem_path=Path(f"/proc/{pid}/mem")
        deadline=time.monotonic()+args.duration_s
        tick=0
        mem=None
        while time.monotonic()<deadline:
            if proc.poll() is not None:
                errors.append(f"process exited rc={proc.returncode}")
                break
            if mem is None:
                try: mem=mem_path.open("rb",buffering=0)
                except OSError as exc:
                    errors.append(f"open mem: {exc}")
                    time.sleep(.05); continue
            try: samples.append(state(mem,tick))
            except OSError as exc: errors.append(str(exc))
            tick+=1
            time.sleep(max(0,args.interval_ms)/1000)
        if mem: mem.close()
        still_running=proc.poll() is None
        if still_running:
            proc.terminate()
            try: proc.wait(timeout=2)
            except subprocess.TimeoutExpired: proc.kill(); proc.wait()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps({
      "pid":pid,"returncode":proc.returncode,"sample_count":len(samples),
      "errors":errors[-50:],"samples":samples
    },indent=2)+"\n")
    return 0 if samples else 2

if __name__=="__main__": raise SystemExit(main())
