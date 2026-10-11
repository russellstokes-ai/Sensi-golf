#!/usr/bin/env python3
"""Verified original v1.014 PE machine-code control flow atlas.

Disassemble every original instruction in the known ball and scoring,
player selection and AI routine windows. Captures ALL conditional and
unconditional flow transfers, direct calls, immediate terrain code
compares, global-state writes, and out-of-window calls. This is a
*static exhaustive range inventory* but not full game-wide semantic
or dynamically tested branch coverage. No protected resource bytes
are included.
"""
from __future__ import annotations
import argparse,hashlib,json,re
from pathlib import Path
from collections import Counter
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from pe_target_xrefs import parse_pe
from pe_disasm_range import rva_to_off

ORIGINAL="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
WINDOWS={
  "ball_interaction_dispatch":(0x40A250,0x40A581),
  "putter_terminal_and_green_events":(0x40A581,0x40AA78),
  "putt_slope_vector":(0x40B965,0x40BA60),
  "multiplayer_next_player":(0x40C180,0x40C260),
  "multiplayer_turn_scoring_adjust":(0x40C005,0x40C100),
  "computer_shot_planner":(0x40C600,0x40C8CB),
  "computer_quick_swing":(0x40C8CB,0x40CAE1),
  "water_hazard_player_event":(0x40CD00,0x40CD80),
  "original_welly_rise":(0x40768B,0x407787),
  "original_welly_fall":(0x407787,0x407960),
}
COND={"ja","jae","jb","jbe","jc","je","jg","jge","jl","jle","jna",
      "jnae","jnb","jnbe","jnc","jne","jng","jnge","jnl","jnle",
      "jno","jnp","jns","jnz","jo","jp","jpe","jpo","js","jz",
      "jcxz","jecxz","loop","loope","loopne"}
def main():
 p=argparse.ArgumentParser()
 p.add_argument("--exe",required=True,type=Path)
 p.add_argument("-o","--output",required=True,type=Path)
 a=p.parse_args(); data=a.exe.read_bytes()
 if hashlib.sha256(data).hexdigest()!=ORIGINAL:
     raise SystemExit("not the authenticated original Windows v1.014")
 base,_,secs=parse_pe(data);md=Cs(CS_ARCH_X86,CS_MODE_32)
 atlas={}
 for name,(start,end) in WINDOWS.items():
     aoff,sec=rva_to_off(start-base,secs)
     boff,sec2=rva_to_off(end-base-1,secs)
     if aoff is None or boff is None or sec["name"]!=sec2["name"]:
         raise ValueError("invalid recovered-original PE function window "+name)
     ins=list(md.disasm(data[aoff:boff+1],start))
     if not ins or ins[0].address!=start or len(ins)<10:
         raise RuntimeError("failed original x86 disassembly for "+name)
     records=[{"va":hex(i.address),"mnemonic":i.mnemonic,
               "args":i.op_str} for i in ins]
     cond=[r for r in records if r["mnemonic"] in COND]
     calls=[r for r in records if r["mnemonic"]=="call"]
     jumps=[r for r in records if r["mnemonic"]=="jmp"]
     compares=[r for r in records if r["mnemonic"] in ("cmp","test")]
     # Count every branch and call. Opaque/indirect cases are marked.
     outgoing=[]
     for record in cond+calls+jumps:
         val=re.search(r"0x[0-9a-fA-F]+",record["args"])
         target=int(val.group(),16) if val else None
         outgoing.append({**record,"target":hex(target) if target else None,
                          "internal":start<=target<end if target else None})
     event_comps=[]
     for record in compares:
         if re.search(r", (0x[0-9a-fA-F]+|\d+)$",record["args"]):
            try:
                value=int(record["args"].split(", ")[-1],0)
                if 0<=value<=105:
                    event_comps.append({**record,"immediate":value})
            except ValueError:pass
     atlas[name]={"start_va":hex(start),"end_va":hex(end),
         "instruction_count":len(ins),
         "conditional_count":len(cond),"call_count":len(calls),
         "unconditional_jumps":len(jumps),
         "unknown_indirect_flows":sum(x["target"] is None for x in outgoing),
         "small_terrain_event_comparisons":event_comps,
         "all_flow_edges":outgoing,
         "full_instructions":records}
 report={"schema":"pixtee.v1014.exhaustive-known-dispatcher-ranges.v1",
         "original_sha256":ORIGINAL,
         "regions":atlas,
         "limitations":[
           "Range-exhaustive enumeration, not entire original executable.",
           "Disassembly of code/data mixed regions can produce false instruction edges.",
           "Static edges alone cannot classify object identity, collision height or CPU strategy.",
           "Full dynamic activation, replay parity and all original mode rules remain open.",
           "Original machine code and unlicensed course/audio/graphics are excluded."
         ]}
 a.output.parent.mkdir(parents=True,exist_ok=True)
 a.output.write_text(json.dumps(report,indent=2)+"\n")
 for name,b in atlas.items():
     print(f"{name}: {b['instruction_count']} instructions, "
           f"{b['conditional_count']} conditional branches, "
           f"{b['call_count']} calls, {b['unknown_indirect_flows']} indirect flows")
     print("  event/terrain compares:",[
           (x["va"],x["immediate"]) for x in b["small_terrain_event_comparisons"][:30]])
 for key in ("ball_interaction_dispatch","putter_terminal_and_green_events","multiplayer_next_player","multiplayer_turn_scoring_adjust","water_hazard_player_event","computer_shot_planner","computer_quick_swing"):
     print("\n=== ORIGINAL BINARY VERIFIED INSTRUCTIONS:",key,"===")
     for row in atlas[key]["full_instructions"]:
         print(row["va"],row["mnemonic"],row["args"])
 print("PASS exhaustive static original code range catalog; dynamic semantics not claimed")
if __name__=="__main__":main()
