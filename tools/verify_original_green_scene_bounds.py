#!/usr/bin/env python3
"""Verify Windows v1.014 original green scene multi-object region gate.

Executes authentic source 0x40C35B with 1..6 source objects at
0x4287D6+0x2C*n, exiting at original 0x40C3F0 if ALL within the
region, or 0x40C40D if any failed. Does not invent tree hitboxes
or assert renderer/coordinate-transition callback side effects.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EBP,UC_X86_REG_EIP
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,w16
ENTRY=0x40c35b
PASS=0x40c3f0
FAIL=0x40c40d
OBJECTS=0x4287d6
def original(uc,row):
    origin_x,origin_y,green,objects=row
    w16(uc,0x41e5ec,origin_x)
    w16(uc,0x41e5f0,origin_y)
    w16(uc,0x41ee7a,green)
    uc.reg_write(UC_X86_REG_EBP,len(objects))
    for i,(x,y) in enumerate(objects):
        w16(uc,OBJECTS+0x2c*i+2,x)
        w16(uc,OBJECTS+0x2c*i+6,y)
    exits=[]
    def stop(m,address,size,user):
        if address in (PASS,FAIL):
            exits.append(address);m.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,stop)
    try:uc.emu_start(ENTRY,0,count=500)
    finally:uc.hook_del(token)
    if len(exits)!=1:
        raise RuntimeError(f"source never reached scene rectangle result {row}")
    return int(exits[0]==PASS)
def vectors():
    rng=random.Random(0x40c35b)
    rows=[]
    for g in (0,1):
        for ox in (0,1,100,32767,32768,65535):
          for oy in (0,50,32768,65535):
            for count in (1,2,3,4,6):
                coords=[]
                for i in range(count):
                    if g:
                        center_x=rng.choice((15,16,17,100,225,226,227))
                        center_y=rng.choice((15,16,17,100,169,170,171))
                    else:
                        center_x=(ox+rng.choice((7,8,9,50,111,112,113)))&0xffff
                        center_y=(oy+rng.choice((7,8,9,50,83,84,85)))&0xffff
                    coords.append((center_x,center_y))
                rows.append((ox,oy,g,coords))
    for _ in range(4500):
        ox=rng.randrange(65536);oy=rng.randrange(65536)
        g=rng.randrange(2);n=rng.randrange(1,7)
        coords=[]
        for i in range(n):
            if rng.randrange(4):
                if g:
                    x=rng.randrange(8,240)
                    y=rng.randrange(8,180)
                else:
                    x=(ox+rng.randrange(0,125))&0xffff
                    y=(oy+rng.randrange(0,96))&0xffff
            else:
                x=rng.randrange(65536);y=rng.randrange(65536)
            coords.append((x,y))
        rows.append((ox,oy,g,coords))
    return rows
def main():
    a=argparse.ArgumentParser()
    a.add_argument("--exe",type=Path,required=True)
    a.add_argument("--probe",type=Path,required=True)
    a.add_argument("-o","--output",type=Path,required=True)
    args=a.parse_args()
    if hashlib.sha256(args.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("expected original Windows v1.014 executable")
    rows=vectors()
    data="".join(" ".join(map(str,[ox,oy,g,len(objs)]+[
        val for pt in objs for val in pt]))+"\n" for ox,oy,g,objs in rows)
    proc=subprocess.run([str(args.probe)],input=data,text=True,
                        capture_output=True,check=True)
    got=[int(x) for x in proc.stdout.splitlines()]
    if len(got)!=len(rows):raise RuntimeError("native source scene count mismatch")
    uc,_=build_uc(args.exe)
    bad=[];inside=0;outside=0
    for row,native in zip(rows,got):
        expected=original(uc,row)
        inside+=expected
        outside+=1-expected
        if native!=expected:
            bad.append({"original_input":row,
                        "source_result":expected,"native":native})
    report={"schema":"pixtee.original-green-scene-multi-object-check.x86.v1",
      "original_sha256":REFERENCE_SHA256,
      "source_start":"0x40C35B",
      "exit_all_inside":"0x40C3F0",
      "exit_any_outside":"0x40C40D",
      "vectors":len(rows),"exact":len(rows)-len(bad),
      "all_inside":inside,"any_outside":outside,
      "mismatches":bad[:25],
      "limits":"Only course-object rectangular source gate, not original source green activation or object-specific collision/trees."}
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original multi-object green region: {report['exact']}/{len(rows)} exact")
    print("Original source branch inside",inside,"outside",outside)
    for err in bad[:6]:print("MISMATCH",err)
    if bad:raise SystemExit("original x86 green object region mismatch")
    print("PASS original game green bounds for ALL source scene objects")
if __name__=="__main__":main()
