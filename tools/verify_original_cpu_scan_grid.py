#!/usr/bin/env python3
"""Original Windows Sensible Golf 1.014 CPU scan configuration and exact table.
Execute original 0x40C6B2..0x40C71B, compare raw register writes with native.
Includes authentic original 24 table entries plus controlled artificial table
word vectors to exercise edge cases. DOES NOT claim whole CPU shot strategy.
"""
import argparse,hashlib,json,random,struct,subprocess,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc,REFERENCE_SHA256,PLAYER,ru16,w16
START=0x40c6b2
END=0x40c71b
BASE=0x41fa11
BAND=0x420537
FIELDS=(0x41fad1,0x41fad5,0x41fad9,0x41fad3,0x420533,0x420531)
def entry(mode,band):
    return BASE+(mode-4)*8+band*48
def original(uc,mode,band,table):
    w16(uc,PLAYER+0x5a,mode)
    uc.mem_write(BAND,bytes([band]))
    uc.mem_write(entry(mode,band),struct.pack("<4H",*table))
    from unicorn.x86_const import UC_X86_REG_ESI
    uc.reg_write(UC_X86_REG_ESI,PLAYER)
    uc.emu_start(START,END,count=80)
    return [ru16(uc,addr) for addr in FIELDS]
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--exe",required=True,type=Path)
    p.add_argument("--probe",required=True,type=Path)
    p.add_argument("-o","--output",required=True,type=Path)
    a=p.parse_args()
    assert hashlib.sha256(a.exe.read_bytes()).hexdigest()==REFERENCE_SHA256
    uc,_=build_uc(a.exe)
    source=[]
    for band in range(4):
        for mode in range(4,10):
            w=list(struct.unpack("<4H",bytes(uc.mem_read(entry(mode,band),8))))
            source.append({"mode":mode,"band":band,"original_word_values":w})
    cases=[(x["mode"],x["band"],x["original_word_values"],"authentic")
           for x in source if x["original_word_values"][0]!=0]
    rng=random.Random(1014)
    for _ in range(1250):
        mode=rng.randint(4,9)
        band=rng.randint(0,3)
        candidate=rng.choice([1,2,3,4,5,7,17,53,154,155,1024,65535])
        words=[candidate,rng.randint(0,65535),rng.randint(0,65535),rng.randint(0,65535)]
        cases.append((mode,band,words,"source-routine-synthetic-input"))
    native=subprocess.run([str(a.probe)],input="".join(
        f"{mode} {band} "+ " ".join(str(w) for w in words)+"\n"
        for mode,band,words,_ in cases),capture_output=True,text=True,check=True)
    lines=native.stdout.strip().splitlines()
    assert len(lines)==len(cases)
    bad=[];n=0
    for idx,((mode,band,words,tag),line) in enumerate(zip(cases,lines)):
        expected=original(uc,mode,band,words)
        observed=list(map(int,line.split()))
        if expected==observed:n+=1
        else:bad.append({"index":idx,"mode":mode,"band":band,"words":words,
                         "source":expected,"native":observed,"tag":tag})
    report={"schema":"pixtee.original-v1014-cpu-scan-grid-x86.v1",
       "sha256":REFERENCE_SHA256,
       "authentic_table_rows":source,
       "test_vectors":len(cases),"exact_matches":n,"mismatches":bad[:50],
       "scope":"original CPU scan table and 154-unit candidate stepping, not computer strategy or shot optimization",
       "commercial_use":"REVIEW ORIGINAL SOURCE-DERIVED NUMERICAL TABLE RIGHTS BEFORE DISTRIBUTING"}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print("Original CPU scan table 24 slots (research reference):")
    for x in source:print(f" band={x['band']} mode={x['mode']} values={x['original_word_values']}")
    print(f"Original computer scan config source x86 vs native: {n}/{len(cases)}")
    for x in bad[:10]:print("MISMATCH",x)
    if bad:raise SystemExit("CPU scan config original source machine-code mismatch")
    print("PASS original 154-step CPU scan arithmetic and source table index decoding")
if __name__=="__main__":main()
