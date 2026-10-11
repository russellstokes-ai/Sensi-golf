#!/usr/bin/env python3
"""Machine-verify the original Windows 1.014 0x40A575 pre-putter gate.

This is a SOURCE-LEVEL branch oracle, not a simulation of an entire shot.
Runs the real signed high-word height test clearing the contact latch,
the club-12 and exact byte=1 bounce admission tests, then the original
0x409535 MAPI lookup if admitted. Stops at one of six authentic
continuation addresses *before* external effects or physics execute.

Modes: thousands of synthetic tile vectors, and optional genuine source
first-hole MAPM/MAPI vectors. No proprietary map bytes are committed.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import verify_original_mapped_terminal_pipeline as source
import verify_original_real_course_mapped_terminal as real
from original_v1014_oracle import (
    build_uc,REFERENCE_SHA256,PLAYER,BALL,STACK,w16,wi32,wu32
)
from original_v1014_course_lookup_oracle import (
    MAPBUF,DESC,MASK,MAP_ROW_STRIDE_VA,MAP_X_EXTENT_VA,MAP_Y_EXTENT_VA
)
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_ESP,UC_X86_REG_EBX

START=0x40A575
DEST={0x40A603:0,0x40A5F7:1,0x40A730:2,0x40A40B:3,
      0x40A6E3:4,0x40A6AB:5}
DESC_CAPTURE=0x40A5A2
CODE_CAPTURE=0x40A5AF
BOUNCE_MARKER=0x41F886

def synthetic_rows():
    rng=random.Random(20261011)
    out=[]
    heights=(-2147483648,-1,0,19<<16, (19<<16)|65535,
             20<<16,(20<<16)|65535,199<<16,200<<16,
             (200<<16)|65535,201<<16,2147483647)
    base=source.vectors()
    for i,v in enumerate(base):
        tile,x,y,*tail=v
        words=tail[:4];mask=tail[4:12]
        pf=tail[13];cf=tail[14]
        # Previous fixture deliberately excludes code8. New gate
        # includes code8 by inserting source descriptor index 7 in
        # selected MAPI subcells, one per four synthetic cases.
        if i%4==0:
            k=(2 if mask[y]&(0x80>>x) else 0)+(4 if mask[y+4]&(0x80>>x) else 0)
            words[k//2]=(words[k//2]&0xff00)|7
        out.append((tile,x,y,*words,*mask,
                    (12 if i%3 else rng.choice((0,5,11,13))),
                    pf,cf,rng.choice(heights),
                    rng.choice((0,1,2,255))))
    # All six routes and boundary behaviours, including exact byte=1
    # gating and signed height high-word >=20 latch release.
    for i in range(1400):
        tile=rng.randrange(64);x=rng.randrange(8);y=rng.randrange(4)
        words=[(rng.randrange(256)<<8)|rng.choice(
            (0,1,6,7,8,15,23,31,47,55,63,64,70,76,77,127))
            for _ in range(4)]
        masks=[rng.randrange(256) for _ in range(8)]
        out.append((tile,x,y,*words,*masks,
                    rng.choice((0,5,11,12,12,12,13)),
                    rng.randrange(65536),rng.randrange(65536),
                    rng.choice(heights),rng.choice((0,1,1,2,3,255))))
    return [(row,None) for row in out],{"synthetic_tile_count":len(out)}

def real_rows(map_data,desc,masks):
    rng=random.Random(101450)
    src,metrics=real.extract_vectors(map_data,desc,masks,3600)
    selected=[]
    for i,(v,xy) in enumerate(src):
        tile,x,y,*tail=v
        word=tail[:4];mask=tail[4:12]
        pf=tail[13];cf=tail[14];z=tail[17]
        selected.append(((tile,x,y,*word,*mask,
                     (12 if i%3 else rng.choice((0,5,11,13))),
                     pf,cf,z,rng.choice((0,1,2,255))),xy))
    # This is a REAL course source, unlike synthetic. Scan all MAPM
    # coordinates for cup descriptor 7 to cover its genuine admission
    # if present. Do not infer anything about actual cup capture.
    width,height=struct.unpack_from(">HH",map_data,0x54)
    cup=[]
    for my in range(height):
        for mx in range(width):
            tile=struct.unpack_from(">H",map_data,0x60+(my*width+mx)*2)[0]&0x3ff
            words=struct.unpack_from(">4H",desc,tile*8)
            planes=tuple(masks[tile*8:tile*8+8])
            for y in range(4):
                for x in range(8):
                    if source.choose_word(words,planes,x,y)==7:
                        cup.append(((tile,x,y,*words,*planes,12,
                                    3,1,0,0),(mx*16+x*2,my*8+y*2)))
    rng.shuffle(cup)
    selected.extend(cup[:250])
    rng.shuffle(selected)
    return selected,{"selection":metrics["eligible_groups"],
                     "actual_descriptor7_subcells":len(cup),
                     "sample_count":len(selected),
                     "width":width,"height":height}

def original(uc,row,world_xy=None):
    tile,x,y,*tail=row
    words=tail[:4];masks=tail[4:12]
    club,pf,cf,z,bounce=tail[12:]
    if world_xy is None:
        uc.mem_write(MAPBUF+0x60,struct.pack(">H",tile))
        uc.mem_write(DESC+tile*8,struct.pack(">4H",*words))
        uc.mem_write(MASK+tile*8,bytes(masks))
        ix,iy=x*2,y*2
    else:
        ix,iy=world_xy
    uc.mem_write(PLAYER,b"\x00"*0x100)
    uc.mem_write(BALL,b"\x00"*0x100)
    w16(uc,PLAYER,pf)
    w16(uc,PLAYER+0x28,club)
    w16(uc,PLAYER+0x60,cf)
    wi32(uc,BALL,ix<<16)
    wi32(uc,BALL+4,iy<<16)
    wi32(uc,BALL+20,z)
    uc.mem_write(BOUNCE_MARKER,bytes((bounce,)))
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.reg_write(UC_X86_REG_EDI,BALL)
    uc.reg_write(UC_X86_REG_ESP,STACK+0xf000)

    desc=[];codes=[];dest=[]
    def hook(machine,address,size,user):
        if address==DESC_CAPTURE:
            desc.append(machine.reg_read(UC_X86_REG_EBX)&0xffff)
        if address==CODE_CAPTURE:
            codes.append(machine.reg_read(UC_X86_REG_EBX)&0xffff)
        if address in DEST:
            dest.append(address)
            machine.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:
        uc.emu_start(START,0,count=550)
    finally:
        uc.hook_del(token)
    if len(dest)!=1 or len(desc)!=len(codes) or len(desc)>1:
        raise RuntimeError(f"original source gate escaped:{row}, {dest,desc,codes}")
    looked=len(desc)==1
    return (DEST[dest[0]],
            struct.unpack("<H",bytes(uc.mem_read(PLAYER+0x60,2)))[0],
            int(looked),desc[0] if looked else 0xffff,
            codes[0] if looked else 0xffff),hex(dest[0])

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("--mapm",type=Path)
    p.add_argument("--descriptor-bank",type=Path)
    p.add_argument("--selector-bank",type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong original Windows 1.014 executable")
    inputs=(a.mapm,a.descriptor_bank,a.selector_bank)
    if any(x is not None for x in inputs) and not all(x is not None for x in inputs):
        raise SystemExit("must specify all real course files or none")
    if all(x is not None for x in inputs):
        data=a.mapm.read_bytes()
        bank=a.descriptor_bank.read_bytes()
        mask=a.selector_bank.read_bytes()
        rows,metrics=real_rows(data,bank,mask)
    else:
        rows,metrics=synthetic_rows()
    command=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,row))+"\n" for row,_ in rows),
        text=True,capture_output=True,check=True)
    native=command.stdout.splitlines()
    if len(native)!=len(rows):
        raise RuntimeError(f"native cases incomplete: {len(native)} != {len(rows)}")
    uc,_=build_uc(a.exe)
    source.setup_original(uc)
    if all(x is not None for x in inputs):
        uc.mem_write(MAPBUF,data)
        uc.mem_write(DESC,bank)
        uc.mem_write(MASK,mask)
        width,height=metrics["width"],metrics["height"]
        wu32(uc,MAP_ROW_STRIDE_VA,width*2)
        wu32(uc,MAP_X_EXTENT_VA,width*16-0x100)
        wu32(uc,MAP_Y_EXTENT_VA,height*8-0xD0)
    mismatch=[];route_count={};lookups=0;resets=0
    for (row,xy),line in zip(rows,native):
        expected,dest=original(uc,row,xy)
        got=tuple(map(int,line.split()))
        route_count[dest]=route_count.get(dest,0)+1
        lookups+=expected[2]
        resets+=(row[17]&1)!=0 and (expected[1]&1)==0
        if expected!=got:
            mismatch.append({"inputs":row[:3]+row[15:],
                             "original":expected,"recovered":got,
                             "destination":dest})
    evidence={
        "schema":"pixtee.original-preterminal-putter-admission-source-x86.v1",
        "original_sha256":REFERENCE_SHA256,
        "mode":"genuine_MAPM01" if all(x is not None for x in inputs) else "synthetic_MAPI",
        "cases":len(rows),"exact_matches":len(rows)-len(mismatch),
        "x86_routes":route_count,"map_lookups":lookups,
        "latch_resets":resets,
        "data_scope":metrics,
        "mismatches":mismatch[:32],
        "source_scoping":"0x40A575 to first terminal or normal-flight branch, does not execute downstream shot physics, tree identity or collision scheduling"
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(evidence,indent=2)+"\n")
    print(f"Original preterminal putter admission {evidence['mode']}: {evidence['exact_matches']}/{len(rows)} exact")
    print("Original source routes:",route_count,
          "MAPI lookups:",lookups,"latch clears:",resets)
    print("Original source data:",metrics)
    for item in mismatch[:6]:print("MISMATCH",item)
    if mismatch:raise SystemExit("source pre-putter gate/native parity mismatch")
    print("PASS exact original x86 source branch and contact-latch admission")

if __name__=="__main__":main()
