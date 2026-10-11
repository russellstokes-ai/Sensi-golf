#!/usr/bin/env python3
"""Compare actual Windows v1.014 original flagpole x86 activation
0x40A501..0x40A53A with native. Deliberately vary raw ball X and Y
independently to catch assumed visual-coordinate semantics.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,BALL
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EDI
GREEN=0x41ee7a
FLAGX=0x41f3b2
FLAGY=0x41f3b4
ENTER=0x40a501; HIT=0x40a53a; MISS=0x40a575

def original(uc,row):
    x,y,height,fx,fy,green=row
    uc.mem_write(BALL+0,struct.pack("<i",x<<16))
    uc.mem_write(BALL+4,struct.pack("<i",y<<16))
    uc.mem_write(BALL+0x14,struct.pack("<i",height))
    uc.mem_write(FLAGX,struct.pack("<h",fx))
    uc.mem_write(FLAGY,struct.pack("<h",fy))
    uc.mem_write(GREEN,struct.pack("<H",green))
    uc.reg_write(UC_X86_REG_EDI,BALL)
    stops=[]
    def hook(m,a,size,u):
        if a in (HIT,MISS):
            stops.append(a);m.emu_stop()
    t=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(ENTER,0,count=70)
    finally:uc.hook_del(t)
    if len(stops)!=1:raise RuntimeError(f"flag original x86 not resolved {row}: {stops}")
    return int(stops[0]==HIT)
def vectors():
    rows=[]
    for x in (-10,-1,0,1,20,30):
        for y in (-10,0,1,20,30,31):
            for h in (-0x10000,-1,0,0x10000,0x20000,0xA0000,0xA0001):
                for green in (0,1):
                    rows.append((x,y,h,x,x-(h>>17),green))
                    rows.append((x,y,h,x,y-(h>>17),green))
                    rows.append((x,y,h,x+1,x-(h>>17),green))
    r=random.Random(1014)
    for _ in range(1200):
        x=r.randint(-300,300);y=r.randint(-300,300)
        h=r.choice((-90000,-1,0,1,0x20000,0xa0000,0xa0001))
        fx=r.choice((x,x+1,y))
        fy=r.choice((x-(h>>17),y-(h>>17),x,y))
        rows.append((x,y,h,fx,fy,r.randrange(2)))
    return rows
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",type=Path,required=True)
    p.add_argument("--probe",type=Path,required=True)
    p.add_argument("-o","--output",type=Path,required=True)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    cases=vectors()
    cmd="".join("%d %d %d %d %d %d\n"%r for r in cases)
    v=subprocess.run([str(a.probe)],input=cmd,capture_output=True,text=True,check=True)
    lines=v.stdout.strip().splitlines()
    assert len(lines)==len(cases)
    uc,_=build_uc(a.exe)
    n=0;bad=[];hits=0
    for row,line in zip(cases,lines):
        expected=original(uc,row);hits+=expected
        actual=int(line)
        if expected==actual:n+=1
        else:bad.append({"inputs":row,"original":expected,"native":actual})
    report={"schema":"pixtee.original-flag-contact-x86.v1","sha256":REFERENCE_SHA256,
        "total":len(cases),"matches":n,"original_hits":hits,"mismatches":bad[:80],
        "scope":"flag conditional machine gate, not flag object presence or complete PRNG impact event"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Original flag x86 gate vs C++: {n}/{len(cases)}")
    print("Original original-source hits:",hits)
    for z in bad[:12]:print("MISMATCH",z)
    if bad:raise SystemExit("Original flag x86 activation mismatch")
    print("PASS independent original flag-coordinate x86 branch")
if __name__=="__main__":main()
