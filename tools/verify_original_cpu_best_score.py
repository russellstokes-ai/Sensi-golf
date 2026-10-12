#!/usr/bin/env python3
"""Original x86 complete signed candidate-record winner selection parity.

Source Windows v1.014 0x40CA5A through 0x40CAC9 / 0x40CAE0.
Scores are synthetic test fixtures in 14-byte original-format records.
This proves CPU score *selection*, not how shot trials create those
scores, CPU club choice, or AI game strategy.
"""
from __future__ import annotations
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_EBX
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,w16
ENTRY=0x40CA5A
FOUND=0x40CAE0
NONE=0x40CAC9
BUF=0x41FE89
SPAN=0x41FA0F
STEP=0x41FAD3
BYTES=14*110

def i16(x):return x if x<0x8000 else x-0x10000
def limit(span,step):
    if step==0:return None
    cx=0
    for n in range(1,111):
        cx=(cx+step)&0xffff
        if i16(cx)>=i16(span):return n
    return None
def cases():
    rng=random.Random(0x40ca8e)
    rows=[]
    for span in (0,1,3,9,30,50,100,250,500,512,32768,65535):
     for step in (1,2,5,7,10,25,50,77,154,255,500,32767,65535):
      if limit(span,step) is None:continue
      for mode in range(6):
       rows.append((span,step,rng.randrange(65536),mode))
    for _ in range(40000):
        if len(rows)>=2900:break
        span=rng.randrange(65536);step=rng.randrange(1,65536)
        if limit(span,step) is None:continue
        rows.append((span,step,rng.randrange(65536),
                     rng.randrange(6)))
    return rows

def src_score(row):
    span,step,seed,mode=row
    scratch=bytearray(BYTES)
    for i in range(110):
        a=b=score=0
        if mode==0:score=(0x7fff-i*31)&0xffff
        elif mode==1:score=100
        elif mode==2:a=0xffff;score=i
        elif mode==3:
            a=0 if (i+seed)%3 else 0xffff
            b=0 if (i+seed)%5 else 0x8000
            score=(seed*17+i*7919)&0xffff
        elif mode==4:
            a=0 if i==seed%100 else 0xffff
            score=(i*137)&0xffff
        elif mode==5:score=(1000-i*500)&0xffff
        struct.pack_into("<HH",scratch,i*14,a,b)
        struct.pack_into("<H",scratch,i*14+12,score)
    return scratch

def original(uc,row):
    span,step,seed,mode=row
    uc.mem_write(BUF,bytes(src_score(row)))
    w16(uc,SPAN,span)
    w16(uc,STEP,step)
    hit=[]
    def hook(machine,address,size,user):
        if address in (FOUND,NONE):
            hit.append(address)
            machine.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(ENTRY,0,count=2500)
    finally:uc.hook_del(h)
    if len(hit)!=1:raise RuntimeError(f"source score chooser did not reach terminal: {row}")
    ptr=uc.reg_read(UC_X86_REG_EBX)
    offset=ptr-BUF
    if offset<0 or offset%14!=0:
        raise RuntimeError("source selected record pointer not aligned")
    yes=hit[0]==FOUND
    return (int(yes),offset//14,offset,
            uc.reg_read(UC_X86_REG_EAX)&0xffff if yes else 0)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",required=True,type=Path)
    ap.add_argument("--probe",required=True,type=Path)
    ap.add_argument("-o","--output",required=True,type=Path)
    a=ap.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("Original Windows executable SHA mismatch")
    rows=cases()
    native_process=subprocess.run([str(a.probe)],
      input="".join(" ".join(map(str,r))+"\n" for r in rows),
      text=True,capture_output=True,check=True)
    native=[tuple(map(int,l.split())) for l in native_process.stdout.splitlines()]
    if len(native)!=len(rows):raise RuntimeError("missing source rank selection rows")
    uc,_=build_uc(a.exe)
    mismatches=[]
    for i,(row,got) in enumerate(zip(rows,native)):
        want=original(uc,row)
        if want!=got:mismatches.append({"case":i,"input":row,"original":want,"native":got})
    out={"schema":"pixtee.original-cpu-best-signed-score-winner.x86.v1",
         "original_sha256":REFERENCE_SHA256,
         "source":"0x40CA5A..0x40CAC9/0x40CAE0",
         "cases":len(rows),"exact":len(rows)-len(mismatches),
         "mismatches":mismatches[:20],
         "scope":"Full original source signed score-record winner and stable tie, with synthetic quality values; generation of original quality values and complete AI still unresolved."}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(out,indent=2)+"\n")
    print(f"Original CPU signed score winner: {out['exact']}/{len(rows)} exact")
    if mismatches:
        for m in mismatches[:4]:print("MISMATCH",m)
        raise SystemExit("source best-scored CPU candidate/native mismatch")
    print("PASS original stable minimum-score CPU candidate selector")
if __name__=="__main__":main()
