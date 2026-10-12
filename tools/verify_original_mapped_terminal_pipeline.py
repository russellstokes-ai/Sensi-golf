#!/usr/bin/env python3
"""Original v1.014 live MAPI lookup -> grounded terminal collision parity.

Executes SHA-256-verified machine code starting at 0x40A65E including the
unaltered original 0x409535 course subcell lookup and original terrain
descriptor pointer table. Synthetic 8-byte MAPI descriptor/selector tiles
exercise all four selector word offsets. Only sprite/audio callbacks are
NOP-stubbed *in Unicorn memory*, preserving their event-call arguments.
The source does not execute the original gameplay *before* 0x40A65E:
this validates the scoped integration, not when a real tree is encountered.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import random
import struct
import subprocess
import sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import (
    BALL,PLAYER,STACK,build_uc,w16,wu32,wi32,REFERENCE_SHA256
)
from original_v1014_course_lookup_oracle import (
    MAPBUF,DESC,MASK,MAP_PTR_VA,MAP_ROW_STRIDE_VA,
    MAP_X_EXTENT_VA,MAP_Y_EXTENT_VA,GREEN_MODE_VA,
    DESC_BANK_PTR_VA,MASK_BANK_PTR_VA,
)
from unicorn import UC_HOOK_CODE,UC_PROT_ALL
from unicorn.x86_const import (
    UC_X86_REG_EBX,UC_X86_REG_EDI,UC_X86_REG_ESI,UC_X86_REG_ESP
)

ENTRY=0x40A65E
DESCRIPTOR_CAPTURE=0x40A679
LANDING_CAPTURE=0x40A686
EXIT={0x40A626:1,0x40A7E0:0}
EVENT_CALLS={0x40A6D1:1,0x40A720:11,0x40A7B7:0}
RENDER_CALLS=(0x40A6C0,0x40A6F8)
PATCHES=RENDER_CALLS+tuple(EVENT_CALLS)
TERRAIN_INDICES=(0,1,2,3,4,5,6,7,8,14,15,21,22,23,30,
                 31,38,39,46,47,54,55,62,63,64,70,72,75,76,77,127,255)

def choose_word(words,mask,x,y):
    bit=0x80>>x
    offset=(2 if mask[y]&bit else 0)+(4 if mask[y+4]&bit else 0)
    return words[offset//2]&0xff

def vectors():
    rows=[]
    rng=random.Random(409535)
    heights=(-2147483648,-65536,0,1,199<<16,200<<16,
             (200<<16)|65535,201<<16,2147483647)
    velocities=(-2147483648,-32768,-8448,-5,-1,1,5,8448,32768,
                1073741824,2147483647)
    # Bounded exhaustive subcell cases: top 3 planes plus all 8x4
    # selector positions with varied source indices, heights and clubs.
    for tile in (0,1,7,31):
        for x in range(8):
            for y in range(4):
                for selector in range(4):
                    idx=TERRAIN_INDICES[(tile*11+x*5+y*3+selector)%len(TERRAIN_INDICES)]
                    if idx==7:idx=6 # code8 cup is separately recovered, not this proof
                    words=[((rng.randrange(256)<<8)|rng.choice(TERRAIN_INDICES)) for _ in range(4)]
                    words[selector]=((rng.randrange(256)<<8)|idx)
                    masks=[rng.randrange(256) for _ in range(8)]
                    bit=0x80>>x
                    masks[y]=(masks[y]&~bit)|(bit if selector&1 else 0)
                    masks[y+4]=(masks[y+4]&~bit)|(bit if selector&2 else 0)
                    rows.append((tile,x,y,*words,*masks,
                                 rng.choice((0,5,12)),
                                 rng.choice((0,1,0x8000,0x8001)),
                                 rng.choice((0,1,2,3,0x8000,0xffff)),
                                 rng.choice(velocities),
                                 rng.choice(velocities),
                                 rng.choice(heights)))
    for _ in range(4800):
        tile=rng.randrange(64)
        x=rng.randrange(8)
        y=rng.randrange(4)
        masks=[rng.randrange(256) for _ in range(8)]
        words=[(rng.randrange(256)<<8)|rng.choice(TERRAIN_INDICES)
               for _ in range(4)]
        selected=choose_word(words,masks,x,y)
        if selected==7: # source code8 enters independent cup state machine
            selected_position=(2 if masks[y]&(0x80>>x) else 0)+(
                4 if masks[y+4]&(0x80>>x) else 0)
            words[selected_position//2]=(words[selected_position//2]&0xff00)|6
        vertical=rng.choice(velocities+(rng.randint(-2147483648,2147483647),))
        if vertical==0:vertical=-1
        rows.append((tile,x,y,*words,*masks,
                     rng.choice((0,1,5,11,12)),
                     rng.choice((0,1,4,0x8000,0xffff)),
                     rng.choice((0,1,2,3,0x8000,0xffff)),
                     vertical,
                     rng.choice(velocities+(rng.randint(-2147483648,2147483647),)),
                     rng.choice(heights+(rng.randint(-2147483648,2147483647),))))
    return rows

def setup_original(uc):
    uc.mem_map(MAPBUF,0x30000,UC_PROT_ALL)
    wu32(uc,MAP_PTR_VA,MAPBUF)
    wu32(uc,DESC_BANK_PTR_VA,DESC)
    wu32(uc,MASK_BANK_PTR_VA,MASK)
    wu32(uc,MAP_ROW_STRIDE_VA,2)
    wu32(uc,MAP_X_EXTENT_VA,0)
    wu32(uc,MAP_Y_EXTENT_VA,0)
    w16(uc,GREEN_MODE_VA,0)
    for va in PATCHES:
        raw=bytes(uc.mem_read(va,5))
        if raw[0]!=0xe8:
            raise RuntimeError(f"unexpected original call at {va:#x}: {raw.hex()}")
        uc.mem_write(va,b"\x90"*5)

def original(uc,row,*,world_xy=None):
    (tile,x,y,*tail)=row
    words=tail[:4]
    masks=tail[4:12]
    club,pf,cf,v,h,z=tail[12:]
    assert v!=0
    if world_xy is None:
        # Synthetic one-tile course resource; do not overwrite the real
        # original MAPM or MAPI banks in the real-resource oracle mode.
        uc.mem_write(MAPBUF+0x60,struct.pack(">H",tile))
        uc.mem_write(DESC+tile*8,struct.pack(">4H",*words))
        uc.mem_write(MASK+tile*8,bytes(masks))
        integer_x,integer_y=x*2,y*2
    else:
        integer_x,integer_y=world_xy
    uc.mem_write(PLAYER,b"\x00"*0x100)
    uc.mem_write(BALL,b"\x00"*0x100)
    w16(uc,PLAYER,pf)
    w16(uc,PLAYER+0x60,cf)
    w16(uc,PLAYER+0x28,club)
    wi32(uc,BALL,integer_x<<16)
    wi32(uc,BALL+4,integer_y<<16)
    wi32(uc,BALL+8,v)
    wi32(uc,BALL+12,h)
    wi32(uc,BALL+20,z)
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)
    found_descriptor=[];found_code=[];events=[];exits=[]
    def hook(machine,address,size,user):
        if address==DESCRIPTOR_CAPTURE:
            found_descriptor.append(machine.reg_read(UC_X86_REG_EBX)&0xffff)
        if address==LANDING_CAPTURE:
            found_code.append(machine.reg_read(UC_X86_REG_EBX)&0xffff)
        if address in EVENT_CALLS:
            events.append(EVENT_CALLS[address])
        if address in EXIT:
            exits.append(address)
            machine.emu_stop()
        if address==0x40A736:
            raise RuntimeError(f"source code8 cup encountered in excluded case {row}")
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(ENTRY,0,count=2000)
    finally:
        uc.hook_del(token)
    if len(found_descriptor)!=1 or len(found_code)!=1 or len(exits)!=1 or len(events)>1:
        raise RuntimeError(f"unexpected original source map-to-terminal trace {row}: {(found_descriptor,found_code,exits,events)}")
    def i32(addr):
        return struct.unpack("<i",bytes(uc.mem_read(addr,4)))[0]
    def u16(addr):
        return struct.unpack("<H",bytes(uc.mem_read(addr,2)))[0]
    return (
        1,found_descriptor[0],found_code[0],
        u16(PLAYER),u16(PLAYER+0x60),
        i32(BALL+8),i32(BALL+12),i32(BALL+20),
        EXIT[exits[0]],events[0] if events else -1
    ),hex(exits[0])

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("original Windows v1.014 executable SHA mismatch")
    rows=vectors()
    probe=subprocess.run([str(a.probe)],
        input="".join(" ".join(str(x) for x in row)+"\n" for row in rows),
        text=True,capture_output=True,check=True)
    output=probe.stdout.splitlines()
    if len(output)!=len(rows):
        raise RuntimeError(f"native map-to-terminal case count {len(output)}/{len(rows)}")
    uc,_=build_uc(a.exe)
    setup_original(uc)
    mismatches=[];routes={};events={};terrain={}
    for row,line in zip(rows,output):
        expected,exit_addr=original(uc,row)
        got=tuple(int(x) for x in line.split())
        routes[exit_addr]=routes.get(exit_addr,0)+1
        events[str(expected[-1])]=events.get(str(expected[-1]),0)+1
        terrain[str(expected[2])]=terrain.get(str(expected[2]),0)+1
        if got!=expected:
            mismatches.append({"input":row,"original":expected,
                               "native":got,"source_exit":exit_addr})
    result={
        "schema":"pixtee.original-mapi-lookup-to-terminal-x86.v1",
        "original_sha256":REFERENCE_SHA256,
        "vectors":len(rows),"exact":len(rows)-len(mismatches),
        "x86_exit_routes":routes,"event_call_codes":events,
        "terrain_landing_codes":terrain,
        "source_entry":"0x40A65E -> 0x409535 -> 0x40A686..0x40A7E0",
        "original_code8_cup":"excluded and kept behind separate original source gate",
        "emulator_only_sprite_audio_stubs":[hex(x) for x in PATCHES],
        "mismatches":mismatches[:35],
        "scope":"Verified original source map decoder, numeric terrain table and terminal physics. Synthetic MAPI bank, not actual tree/objects or complete upstream game dispatch."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(result,indent=2)+"\n")
    print(f"Original MAPI lookup to terminal collision: {result['exact']}/{len(rows)} exact")
    print("Authentic x86 exit routes:",routes,"events:",events)
    print("Observed original landing codes:",terrain)
    for row in mismatches[:8]:print("MISMATCH",row)
    if mismatches:raise SystemExit("MAPI-to-terminal C++ parity mismatch")
    print("PASS exact authentic map lookup/terrain dispatch/ball-state pipeline")

if __name__=="__main__":
    main()
