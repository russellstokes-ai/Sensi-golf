#!/usr/bin/env python3
"""Compare original v1.014 0x40A7E0..0x40A7F1 pause tick to C++.
Does not model downstream original terrain refresh and player relocation.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,BALL,PLAYER,ru16,w16
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI
PAUSE=BALL+0x1c;LATCH=0x41f886;START=0x40a7e0;WAIT=0x40aa78;EXPIRED=0x40a7f1
def source(uc,paused,gate):
    w16(uc,PAUSE,paused);uc.mem_write(LATCH,bytes([gate]))
    uc.reg_write(UC_X86_REG_ESI,PLAYER);uc.reg_write(UC_X86_REG_EDI,BALL)
    stops=[]
    def hook(m,addr,size,unused):
        if addr in (WAIT,EXPIRED):stops.append(addr);m.emu_stop()
    token=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(START,0,count=15)
    finally:uc.hook_del(token)
    if len(stops)!=1:raise RuntimeError(f"original pause branch unexpected {paused}, {gate}: {stops}")
    return [ru16(uc,PAUSE),bytes(uc.mem_read(LATCH,1))[0],int(stops[0]==EXPIRED)]
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path);p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path);a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    rng=random.Random(1014)
    cases=[(n,g) for n in (0,1,2,3,99,100,101,32767,32768,65535) for g in (0,1,255)]
    cases.extend((rng.randrange(65536),rng.randrange(256)) for _ in range(1200))
    out=subprocess.run([str(a.probe)],input="".join(f"{n} {g}\n" for n,g in cases),capture_output=True,text=True,check=True)
    lines=out.stdout.strip().splitlines();assert len(lines)==len(cases)
    uc,_=build_uc(a.exe);issues=[];good=0
    for case,row in zip(cases,lines):
        expected=source(uc,*case);actual=list(map(int,row.split()))
        if actual==expected:good+=1
        else:issues.append({"input":case,"original":expected,"native":actual})
    report={"schema":"pixtee.original-special-green-pause-x86.v1","exe_sha256":REFERENCE_SHA256,
        "cases":len(cases),"matches":good,"mismatches":issues[:40],
        "scope":"paused ball WORD decrement/sign expiry and original UI latch only; post-pause terrain/ball/game state not recovered"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print(f"Actual original x86 special ball pause tick vs C++: {good}/{len(cases)}")
    print("Original timer zero boundary:",source(uc,0,1))
    for row in issues[:6]:print("MISMATCH",row)
    if issues:raise SystemExit("special pause tick not identical to original")
    print("PASS original countdown one extra zero tick before next continuation")
if __name__=="__main__":main()
