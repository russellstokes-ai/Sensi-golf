#!/usr/bin/env python3
"""Verify original Windows 1.014 ten-slot animation timer+plain script end to end.

Executes actual source x86 entry 0x40C50B until 0x40C52C with synthetic
nonnegative source script assets in transient memory. This proves that
the recovered native scheduler combines the independently verified
countdown and ordinary-frame reader with authentic iteration order.
Negative opcode dispatch and renderer/SFX remain open, not faked.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
from unicorn import UC_HOOK_CODE
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,w16,wu32,ru16,ru32
# The hash-authenticated Unicorn harness already maps PLAYER=0x500000
# for 64 KiB of scratch. Use a nonoverlapping offset within that region
# rather than remapping a PE-address range and triggering UC_ERR_MAP.
BASE=0x41F8F0; SCRIPT=0x502000; ENTRY=0x40C50B; STOP=0x40C52C
def gen():
    rng=random.Random(0x40c50b)
    cases=[]
    for n in range(1500):
        slots=[]
        for i in range(10):
            status=rng.choice((0xffff,0,0,1,2))
            ticks=rng.choice((0,0,1,2,3,4,65535))
            offset=rng.randrange(0,33)*2
            frame=rng.choice((0,1,10,32767,rng.randrange(32768)))
            delay=rng.choice((0,1,4,5,100,255,65535))
            height=rng.randrange(65536)
            width=rng.randrange(65536)
            slots.append((status,ticks,offset,frame,delay,height,width))
        cases.append(slots)
    return cases
def original(uc,slots):
    for i,(status,ticks,offset,frame,delay,height,width) in enumerate(slots):
        slot=BASE+0x14*i
        asset=SCRIPT+128*i
        script=bytearray(offset+8)
        struct.pack_into("<H",script,0,delay)
        struct.pack_into("<H",script,6+offset,frame)
        uc.mem_write(asset,bytes(script))
        w16(uc,slot,status)
        w16(uc,slot+0xa,ticks)
        w16(uc,slot+6,height)
        w16(uc,slot+8,width)
        wu32(uc,slot+0xc,asset)
        wu32(uc,slot+0x10,offset)
    seen=[]
    def hook(m,addr,size,user):
        if addr==STOP:
            seen.append(addr);m.emu_stop()
    h=uc.hook_add(UC_HOOK_CODE,hook)
    try:uc.emu_start(ENTRY,0,count=1000)
    finally:uc.hook_del(h)
    if seen!=[STOP]:raise RuntimeError("original 10-slot dispatcher did not finish")
    values=[]
    for i in range(10):
        slot=BASE+0x14*i
        values.extend((ru16(uc,slot),ru16(uc,slot+0xa),
                       ru16(uc,slot+6),ru32(uc,slot+0x10),
                       ru16(uc,slot+8)))
    return tuple(values)
def main():
    a=argparse.ArgumentParser()
    a.add_argument("--exe",type=Path,required=True)
    a.add_argument("--probe",type=Path,required=True)
    a.add_argument("-o","--output",type=Path,required=True)
    p=a.parse_args()
    if hashlib.sha256(p.exe.read_bytes()).hexdigest()!=REFERENCE_SHA256:
        raise SystemExit("wrong original Windows 1.014 source binary")
    rows=gen()
    inp="".join(" ".join(str(n) for slot in case for n in slot)+"\n"
                for case in rows)
    out=subprocess.run([str(p.probe)],input=inp,text=True,
                       capture_output=True,check=True)
    native=[tuple(map(int,line.split())) for line in out.stdout.splitlines()]
    if len(native)!=len(rows):raise RuntimeError("native mixed event count mismatch")
    uc,_=build_uc(p.exe)
    mismatches=[]
    for k,(row,got) in enumerate(zip(rows,native)):
        expected=original(uc,row)
        if got!=expected:
            mismatches.append({"case":k,"source":expected,"native":got})
    result={"schema":"pixtee.original-10-slot-full-normal-script-tick.x86.v1",
        "original_sha256":REFERENCE_SHA256,
        "source_entry":"0x40C50B",
        "source_stop":"0x40C52C",
        "mixed_ten_slot_cases":len(rows),
        "verified_slot_field_comparisons":len(rows)*10*5,
        "exact":len(rows)-len(mismatches),
        "mismatches":mismatches[:12],
        "limitations":"Only nonnegative script-frame branch. Original negative command dispatch, image decode, sound and host frame pacing not verified."}
    p.output.parent.mkdir(parents=True,exist_ok=True)
    p.output.write_text(json.dumps(result,indent=2)+"\n")
    print(f"Original mixed ten-slot full tick: {result['exact']}/{len(rows)} exact")
    for m in mismatches[:3]:print("MISMATCH",m)
    if mismatches:raise SystemExit("original mixed event animation mismatch")
    print("PASS authentic ten-slot countdown and plain animation script progression")
if __name__=="__main__":main()
