#!/usr/bin/env python3
"""Execute the original Sensible Golf v1.014 live-player launch and clear-air code."""

from __future__ import annotations
import argparse, hashlib, json, struct
from pathlib import Path
from pe_target_xrefs import parse_pe

REFERENCE_SHA256 = "3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
IMAGE_MIN=0x400000
PLAYER=0x500000
BALL=0x501000
STACK=0x600000
SENTINEL=0x700000

CLUB_TABLE_VA=0x41F108
PROFILE_SELECTOR_VA=0x41D4C4
PROFILE_BOUNDS_VA=0x41D546
ACCURACY_LOWER_VA=0x41D4C0
ACCURACY_UPPER_VA=0x41D4C2
DROP_POWER_VA=0x41D59E
ACCURACY_TICK_VA=0x41D5A0
SWING_ADJ_VA=0x41F32C
CURRENT_PLAYER_VA=0x41F3A8
CURRENT_BALL_VA=0x41F3AC
GREEN_MODE_VA=0x41EE7A
SPECIAL_MODE_VA=0x41EF66
ADJ_X_VA=0x41F0C5
ADJ_Y_VA=0x41F0C9

LIVE_LAUNCH_VA=0x40AD1D
AIR_TICK_VA=0x40A603
AIR_TICK_END_VA=0x40AA1C

def align_up(v,a=0x1000): return (v+a-1)&~(a-1)
def w16(uc,a,v): uc.mem_write(a,struct.pack("<H",v&0xffff))
def wi32(uc,a,v): uc.mem_write(a,struct.pack("<i",int(v)))
def wu32(uc,a,v): uc.mem_write(a,struct.pack("<I",v&0xffffffff))
def ri32(uc,a): return struct.unpack("<i",bytes(uc.mem_read(a,4)))[0]
def ru32(uc,a): return struct.unpack("<I",bytes(uc.mem_read(a,4)))[0]
def ru16(uc,a): return struct.unpack("<H",bytes(uc.mem_read(a,2)))[0]

def build_uc(exe: Path):
    try:
        from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_PROT_ALL
    except ImportError as exc:
        raise RuntimeError("unicorn package is required") from exc
    data=exe.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    if digest != REFERENCE_SHA256: raise ValueError(f"wrong GOLFWIN.EXE build: {digest}")
    image_base,_,sections=parse_pe(data)
    max_end=image_base+0x1000
    for s in sections:
        max_end=max(max_end,image_base+s["rva"]+max(s["virtual_size"],s["raw_size"]))
    uc=Uc(UC_ARCH_X86,UC_MODE_32)
    uc.mem_map(image_base,align_up(max_end-image_base),UC_PROT_ALL)
    first_raw=min((s["raw_offset"] for s in sections if s["raw_offset"]),default=0x1000)
    uc.mem_write(image_base,data[:first_raw])
    for s in sections:
        if s["raw_size"]:
            uc.mem_write(image_base+s["rva"],data[s["raw_offset"]:s["raw_offset"]+s["raw_size"]])
    uc.mem_map(PLAYER,0x10000,UC_PROT_ALL)
    uc.mem_map(STACK,0x10000,UC_PROT_ALL)
    uc.mem_map(SENTINEL,0x1000,UC_PROT_ALL)
    return uc,digest

def club_values(uc,club):
    raw=bytes(uc.mem_read(CLUB_TABLE_VA+club*12,12))
    v,h,scale=struct.unpack("<iii",raw)
    return v//2,h//2,scale

def profile_for(uc,club,lie):
    return int(bytes(uc.mem_read(PROFILE_SELECTOR_VA+club*10+lie,1))[0])

def bounds_for(uc,profile_id):
    off=PROFILE_BOUNDS_VA+(profile_id-1)*4
    return ru16(uc,off),ru16(uc,off+2)

def setup(uc,club,lie,power,accuracy,direction):
    from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_ESP
    if not 0<=club<13: raise ValueError("club must be original index 0..12")
    if not 0<=lie<10: raise ValueError("lie must be 0..9")
    if not 0<=power<=105: raise ValueError("power must be 0..105")

    vbase,hbase,scale=club_values(uc,club)
    profile=profile_for(uc,club,lie)
    lower,upper=bounds_for(uc,profile)
    if not lower<=accuracy<=upper:
        raise ValueError(f"accuracy {accuracy} outside scoped profile {profile} bounds {lower}..{upper}")

    uc.mem_write(PLAYER,b"\0"*0x100)
    uc.mem_write(BALL,b"\0"*0x100)
    wu32(uc,PLAYER+0x02,BALL)
    wu32(uc,PLAYER+0x12,direction&0x0fff)
    w16(uc,PLAYER+0x28,club)
    wi32(uc,PLAYER+0x2a,vbase)
    wi32(uc,PLAYER+0x2e,hbase)
    wi32(uc,PLAYER+0x32,scale)
    w16(uc,PLAYER+0x5a,0)
    w16(uc,BALL+0x20,lie)

    wu32(uc,CURRENT_PLAYER_VA,PLAYER)
    wu32(uc,CURRENT_BALL_VA,BALL)
    w16(uc,DROP_POWER_VA,power)
    w16(uc,ACCURACY_TICK_VA,accuracy)
    w16(uc,ACCURACY_LOWER_VA,lower)
    w16(uc,ACCURACY_UPPER_VA,upper)
    wi32(uc,SWING_ADJ_VA,0)
    w16(uc,GREEN_MODE_VA,0)
    w16(uc,SPECIAL_MODE_VA,0)
    wi32(uc,ADJ_X_VA,0)
    wi32(uc,ADJ_Y_VA,0)
    uc.mem_write(0x42558E,b"\x00")

    sp=STACK+0xf000-4
    wu32(uc,sp,SENTINEL)
    uc.reg_write(UC_X86_REG_ESP,sp)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    return profile,lower,upper

def sample(uc,tick):
    return {
        "tick":tick,
        "x":ri32(uc,BALL+0x00),
        "y":ri32(uc,BALL+0x04),
        "height":ri32(uc,BALL+0x14),
        "vertical_force":ri32(uc,BALL+0x08),
        "horizontal_force":ri32(uc,BALL+0x0c),
        "direction":ru32(uc,BALL+0x10)&0x0fff,
        "swing_adjuster":ri32(uc,SWING_ADJ_VA),
        "adjusted_power":ru16(uc,DROP_POWER_VA),
    }

def run_oracle(exe,club,lie,power,accuracy,direction,ticks):
    from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI
    uc,digest=build_uc(exe)
    profile,lower,upper=setup(uc,club,lie,power,accuracy,direction)
    uc.emu_start(LIVE_LAUNCH_VA,SENTINEL,count=5000)
    samples=[sample(uc,0)]

    for tick in range(1,ticks+1):
        cur=sample(uc,tick-1)
        if cur["height"]+cur["vertical_force"]-0x2100<=0:
            raise RuntimeError("requested oracle length reaches landing branch")
        w16(uc,GREEN_MODE_VA,0)
        w16(uc,SPECIAL_MODE_VA,0)
        wi32(uc,ADJ_X_VA,0)
        wi32(uc,ADJ_Y_VA,0)
        uc.reg_write(UC_X86_REG_ESI,PLAYER)
        uc.reg_write(UC_X86_REG_EDI,BALL)
        uc.emu_start(AIR_TICK_VA,AIR_TICK_END_VA,count=5000)
        samples.append(sample(uc,tick))

    return {
        "oracle":"original-v1.014-live-player-machine-code",
        "build_sha256":digest,
        "profile_id":profile,
        "profile_bounds":[lower,upper],
        "input":{"club":club,"lie":lie,"power":power,"accuracy_tick":accuracy,"direction":direction&0xfff,"ticks":ticks},
        "samples":samples,
    }

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("exe",type=Path)
    for name in ("club","lie","power","accuracy","direction","ticks"):
        ap.add_argument(f"--{name}",type=int,required=True)
    ap.add_argument("-o","--output",type=Path)
    a=ap.parse_args()
    report=run_oracle(a.exe,a.club,a.lie,a.power,a.accuracy,a.direction,a.ticks)
    text=json.dumps(report,indent=2)+"\n"
    if a.output:a.output.write_text(text,encoding="utf-8")
    else:print(text,end="")
    return 0
if __name__=="__main__": raise SystemExit(main())
