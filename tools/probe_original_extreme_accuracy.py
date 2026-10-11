#!/usr/bin/env python3
"""Original Windows Sensible Golf v1.014: execute REAL original launch routine
at out-of-profile accuracy values. Existing in-bounds oracle intentionally
rejects these; this research probe does NOT apply that artificial guard.
Any strange result is machine observation, not permission to implement it
without dynamic original menu/input-range proof.
"""
from __future__ import annotations
import argparse,hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import original_v1014_oracle as src
from unicorn.x86_const import UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_ESP

def w16(uc,a,v): uc.mem_write(a,struct.pack("<H",int(v)&0xffff))
def w32(uc,a,v): uc.mem_write(a,struct.pack("<I",int(v)&0xffffffff))
def wi32(uc,a,v):uc.mem_write(a,struct.pack("<i",int(v)))
def test_one(exe,club,lie,accuracy):
  uc,_=src.build_uc(exe)
  basev,baseh,scale=src.club_values(uc,club)
  profile=src.profile_for(uc,club,lie)
  lo,hi=src.bounds_for(uc,profile)
  uc.mem_write(src.PLAYER,b"\0"*0x100)
  uc.mem_write(src.BALL,b"\0"*0x100)
  w32(uc,src.PLAYER+0x02,src.BALL)
  w32(uc,src.PLAYER+0x12,777)
  w16(uc,src.PLAYER+0x28,club)
  wi32(uc,src.PLAYER+0x2a,basev)
  wi32(uc,src.PLAYER+0x2e,baseh)
  wi32(uc,src.PLAYER+0x32,scale)
  w16(uc,src.PLAYER+0x5a,0)
  w16(uc,src.BALL+0x20,lie)
  w32(uc,src.CURRENT_PLAYER_VA,src.PLAYER)
  w32(uc,src.CURRENT_BALL_VA,src.BALL)
  w16(uc,src.DROP_POWER_VA,105)
  w16(uc,src.ACCURACY_TICK_VA,accuracy)
  w16(uc,src.ACCURACY_LOWER_VA,lo)
  w16(uc,src.ACCURACY_UPPER_VA,hi)
  wi32(uc,src.SWING_ADJ_VA,0)
  w16(uc,src.GREEN_MODE_VA,0)
  w16(uc,src.SPECIAL_MODE_VA,0)
  wi32(uc,src.ADJ_X_VA,0)
  wi32(uc,src.ADJ_Y_VA,0)
  uc.mem_write(0x42558E,b"\x00")
  sp=src.STACK+0xf000-4
  w32(uc,sp,src.SENTINEL)
  uc.reg_write(UC_X86_REG_ESP,sp)
  uc.reg_write(UC_X86_REG_ESI,src.PLAYER)
  uc.reg_write(UC_X86_REG_EDI,src.BALL)
  base={"club":club,"lie":lie,"accuracy":accuracy,"profile":profile,
       "original_profile_bounds":[lo,hi],
       "outside_profile":not(lo<=accuracy<=hi)}
  try:
    uc.emu_start(src.LIVE_LAUNCH_VA,src.SENTINEL,count=5000)
    outcome=src.sample(uc,0)
    base.update({"result":"executed_original_machine_code","state":outcome})
  except Exception as e:
    base.update({"result":"emulation_exception","exception":type(e).__name__,
                 "reason":str(e)[:140]})
  return base
def main():
  a=argparse.ArgumentParser()
  a.add_argument("--exe",type=Path,required=True)
  a.add_argument("-o","--output",type=Path,required=True)
  args=a.parse_args()
  assert hashlib.sha256(args.exe.read_bytes()).hexdigest()==src.REFERENCE_SHA256
  results=[]
  accuracies=(0,30,54,55,60,63,65,70,71,90,105)
  for club in range(13):
    for lie in (0,2,3,5,7,9):
      uc,_=src.build_uc(args.exe)
      prof=src.profile_for(uc,club,lie)
      lo,hi=src.bounds_for(uc,prof)
      for acc in sorted(set(accuracies+(lo-1,hi+1,lo,hi))):
        if 0<=acc<=105:
          results.append(test_one(args.exe,club,lie,acc))
  import collections
  summary=dict(collections.Counter(
    (row["outside_profile"],row["result"]) for row in results))
  output={"schema":"pixtee.research.original-v1014-extreme-accuracy-oracle.v1",
    "source_executable_sha256":src.REFERENCE_SHA256,
    "scope":"isolated original launcher at arbitrary raw accuracy registers; actual user input accessibility remains unverified",
    "samples":len(results),"cases":results,
    "results_by_in_bounds_and_execution":[
      {"outside_profile":k[0],"result":k[1],"count":v}
       for k,v in summary.items()],
    "commercial_reuse":"UNREVIEWED"}
  args.output.parent.mkdir(parents=True,exist_ok=True)
  args.output.write_text(json.dumps(output,indent=2)+"\n")
  print("Original extreme accuracy source launch:",len(results),"cases")
  for k,v in summary.items():print(" original execution",k,v)
  out=[x for x in results if x["outside_profile"] and x["result"]=="executed_original_machine_code"]
  for x in out[:24]:
    state=x["state"]
    print(" outside",x["club"],x["lie"],x["accuracy"],"profile",x["profile"],
          "adjusted",state["adjusted_power"],"direction",state["direction"],
          "swing",state["swing_adjuster"],"forces",state["vertical_force"],state["horizontal_force"])
  print("PASS source machine execution observations generated; inputs outside normal profile require separately proven gameplay access")
if __name__=="__main__":main()
