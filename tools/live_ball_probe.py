#!/usr/bin/env python3
"""Read live Sensible Golf v1.014 ball state from a running Wine process.

This tool is for controlled golden-master capture only. It never modifies
process memory. Addresses are specific to the verified GOLFWIN.EXE v1.014.
"""

from __future__ import annotations
import argparse
import json
import os
import struct
import time
from pathlib import Path

CURRENT_BALL_PTR = 0x41F3AC
BALL_STRIDE = 0x2C


def read_mem(mem, address: int, size: int) -> bytes:
    mem.seek(address)
    data = mem.read(size)
    if len(data) != size:
        raise OSError(f"short read at 0x{address:x}: {len(data)} != {size}")
    return data


def u32(mem, address: int) -> int:
    return struct.unpack("<I", read_mem(mem,address,4))[0]


def i32(mem, address: int) -> int:
    return struct.unpack("<i", read_mem(mem,address,4))[0]


def u16(mem, address: int) -> int:
    return struct.unpack("<H", read_mem(mem,address,2))[0]


def sample(mem, tick: int) -> dict:
    ball=u32(mem,CURRENT_BALL_PTR)
    if ball == 0:
        return {"tick":tick,"ball_ptr":0}
    return {
        "tick":tick,
        "ball_ptr":ball,
        "x":i32(mem,ball+0x00),
        "y":i32(mem,ball+0x04),
        "vertical_force":i32(mem,ball+0x08),
        "horizontal_force":i32(mem,ball+0x0C),
        "direction":u32(mem,ball+0x10),
        "height":i32(mem,ball+0x14),
        "distance_to_hole":i32(mem,ball+0x18),
        "pause":i32(mem,ball+0x1C),
        "surface":u16(mem,ball+0x1E),
        "terrain_direction":u16(mem,ball+0x26),
        "terrain_magnitude":u16(mem,ball+0x2A),
    }


def main()->int:
    ap=argparse.ArgumentParser()
    ap.add_argument("pid",type=int)
    ap.add_argument("-o","--output",type=Path,required=True)
    ap.add_argument("--interval-ms",type=float,default=5.0)
    ap.add_argument("--duration-s",type=float,default=10.0)
    args=ap.parse_args()

    mem_path=Path(f"/proc/{args.pid}/mem")
    samples=[]
    deadline=time.monotonic()+args.duration_s
    tick=0
    with mem_path.open("rb",buffering=0) as mem:
        while time.monotonic()<deadline:
            try:
                row=sample(mem,tick)
            except OSError as exc:
                row={"tick":tick,"error":str(exc)}
            samples.append(row)
            tick+=1
            time.sleep(max(0,args.interval_ms)/1000.0)

    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps({
        "pid":args.pid,
        "current_ball_ptr_va":CURRENT_BALL_PTR,
        "interval_ms":args.interval_ms,
        "samples":samples,
    },indent=2)+"\n",encoding="utf-8")
    return 0


if __name__=="__main__":
    raise SystemExit(main())
