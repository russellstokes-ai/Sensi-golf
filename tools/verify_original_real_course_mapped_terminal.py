#!/usr/bin/env python3
"""Original Windows v1.014 authentic MAPM01/MAPI01+02 -> terminal parity.

The SHA-verified original EXE is executed against original course resources
extracted to a private GitHub Actions workspace from the original archive.
All protected map and tile bank bytes stay outside the repository and
uploaded evidence: only parity counts and path distributions are retained.

This verifies the original source 0x409535 map lookup at ACTUAL MAPM01
positions, followed by 0x40A686..0x40A7E0 grounded terminal effects.
It cannot establish original tree objects or the upstream world collision
scheduler, and it deliberately excludes original code-8 cup capture.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parent))
import verify_original_mapped_terminal_pipeline as source
from original_v1014_course_lookup_oracle import (
    MAPBUF,DESC,MASK,MAP_ROW_STRIDE_VA,MAP_X_EXTENT_VA,MAP_Y_EXTENT_VA
)
from original_v1014_oracle import build_uc,wu32,REFERENCE_SHA256

TERRAIN_35={6,14,22,30,38,46,54,62,70}
TERRAIN_50={47,55,63,71}

def extract_vectors(map_data, desc, masks, count):
    width,height=struct.unpack_from(">HH",map_data,0x54)
    if not 0<width<4000 or not 0<height<4000:
        raise ValueError("original MAPM header dimensions invalid")
    if len(map_data)<0x60+width*height*2:
        raise ValueError("original MAPM data truncated")
    if len(desc)!=len(masks) or len(desc)!=640*8:
        raise ValueError("source MAPI pair not authentic 640-tile length")

    rng=random.Random(0x1014)
    pools={"code35":[],"code50":[],"other":[]}
    per_landing={}
    # Source verified grid decoding: MAPM big-endian word, low 10 bits
    # tile index; each tile four 16-bit descriptor words controlled by
    # the original two MAPI selector bitplanes.
    for my in range(height):
        for mx in range(width):
            # Keep interior of original whole-course lookup coordinates.
            if not (0<=mx*16<width*16 and 0<=my*8<height*8):
                continue
            tile=struct.unpack_from(">H",map_data,0x60+(my*width+mx)*2)[0]&0x3ff
            if tile>=640:
                raise ValueError("original course references invalid MAPI tile")
            words=struct.unpack_from(">4H",desc,tile*8)
            sel=tuple(masks[tile*8:tile*8+8])
            for y in range(4):
                for x in range(8):
                    low=source.choose_word(words,sel,x,y)
                    index=low if low<77 else 4
                    if index==7:continue # original cup handled separately
                    category=("code35" if index in TERRAIN_35 else
                              "code50" if index in TERRAIN_50 else "other")
                    per_landing[str(index)]=per_landing.get(str(index),0)+1
                    pools[category].append((tile,x,y,*words,*sel,mx,my))
    total_available=sum(len(v) for v in pools.values())
    if total_available==0:raise RuntimeError("no original MAPM collision cells to test")
    # Choose source real cells in each available terrain category, no
    # assumption the first original hole contains a particular code.
    selections=[]
    allocations={"code35":count//3,"code50":count//3,
                 "other":count-2*(count//3)}
    for key,amount in allocations.items():
        candidates=pools[key]
        rng.shuffle(candidates)
        if not candidates:continue
        selections.extend(candidates[:amount])
    if len(selections)<min(count//3,total_available):
        raise RuntimeError("insufficient real-course valid non-code8 samples")
    rng.shuffle(selections)
    heights=(-2147483648,-1,0,1,199<<16,200<<16,
             (200<<16)|65535,201<<16,2147483647)
    velocities=(-2147483648,-65536,-8448,-1,1,5,8448,65536,2147483647)
    rows=[]
    for sample in selections:
        tile,x,y,*tail=sample
        words=tail[:4];masks=tail[4:12];mx,my=tail[12:]
        vertical=rng.choice(velocities)
        row=(tile,x,y,*words,*masks,
             rng.choice((0,5,11,12)),
             rng.choice((0,1,2,0x8000,0xffff)),
             rng.choice((0,1,2,3,0x8000,0xffff)),
             vertical,rng.choice(velocities),rng.choice(heights))
        world_xy=(mx*16+x*2,my*8+y*2)
        rows.append((row,world_xy))
    return rows,{"map_width":width,"map_height":height,
                 "eligible_real_cells":total_available,
                 "eligible_groups":{k:len(v) for k,v in pools.items()},
                 "descriptor_occurrences":per_landing}

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--mapm",required=True,type=Path)
    p.add_argument("--descriptor-bank",required=True,type=Path)
    p.add_argument("--selector-bank",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("--count",type=int,default=3600)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("incorrect original Windows executable checksum")
    map_data=a.mapm.read_bytes()
    desc=a.descriptor_bank.read_bytes()
    selectors=a.selector_bank.read_bytes()
    cases,metrics=extract_vectors(map_data,desc,selectors,a.count)
    rows=[r for r,_ in cases]
    probe=subprocess.run([str(a.probe)],
        input="".join(" ".join(map(str,r))+"\n" for r in rows),
        text=True,capture_output=True,check=True)
    lines=probe.stdout.splitlines()
    if len(lines)!=len(rows):
        raise RuntimeError("native original real-course case count mismatch")
    uc,_=build_uc(a.exe)
    source.setup_original(uc)
    uc.mem_write(MAPBUF,map_data)
    uc.mem_write(DESC,desc)
    uc.mem_write(MASK,selectors)
    wu32(uc,MAP_ROW_STRIDE_VA,metrics["map_width"]*2)
    wu32(uc,MAP_X_EXTENT_VA,metrics["map_width"]*16-0x100)
    wu32(uc,MAP_Y_EXTENT_VA,metrics["map_height"]*8-0xD0)
    mismatches=[];routes={};events={};landings={}
    for (row,world_xy),line in zip(cases,lines):
        expected,exit_addr=source.original(uc,row,world_xy=world_xy)
        actual=tuple(int(x) for x in line.split())
        routes[exit_addr]=routes.get(exit_addr,0)+1
        events[str(expected[-1])]=events.get(str(expected[-1]),0)+1
        landings[str(expected[2])]=landings.get(str(expected[2]),0)+1
        if actual!=expected:
            mismatches.append({
                "tile":row[0],"subcell":row[1:3],
                "position":world_xy,
                "original":expected,"native":actual,"x86_exit":exit_addr
            })
    evidence={
      "schema":"pixtee.original-real-MAPM01-map-to-terminal-parity.v1",
      "original_exe_sha256":REFERENCE_SHA256,
      "source_course":"MAPM01.MAP with MAPI01.RAW and MAPI02.RAW",
      "map_file_sha256":hashlib.sha256(map_data).hexdigest(),
      "mapi_pair_sha256":[hashlib.sha256(desc).hexdigest(),
                          hashlib.sha256(selectors).hexdigest()],
      "candidate_summary":metrics,
      "total_tested":len(cases),"exact":len(cases)-len(mismatches),
      "source_x86_routes":routes,"source_events":events,
      "source_landing_codes":landings,
      "mismatches":mismatches[:35],
      "scope":"Real original course bytes decoded by authentic source machine 0x409535 -> grounded original terminal 0x40A7E0; independently injected hypothetical ball heights/forces; no actual object/tree activation or full event side effects claimed."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(evidence,indent=2)+"\n")
    print(f"Original real MAPM01 terrain-to-terminal collision: {evidence['exact']}/{len(cases)} exact")
    print("Original candidate distribution:",metrics["eligible_groups"])
    print("Original landing routes:",landings)
    print("Source events:",events,"branch exits:",routes)
    for problem in mismatches[:7]:print("MISMATCH",problem)
    if mismatches:raise SystemExit("genuine MAPM01 source/native collision discrepancy")
    print("PASS source-authenticated original map bytes and lookup to terminal collision state")

if __name__=="__main__":main()
