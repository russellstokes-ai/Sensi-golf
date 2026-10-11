#!/usr/bin/env python3
"""Read actual fingerprint-verified v1.014 exe / EPF for Pixtee research.
Output only opcode summaries, primitive geometry/metadata, original resource
names and status; never original graphics/sound/course bitmaps or raw binaries.
No claim that disassembly alone proves full game rules or event dispatch.
"""
from __future__ import annotations
import argparse,collections,hashlib,json,re,struct,sys
from pathlib import Path
from pe_target_xrefs import parse_pe
from pe_disasm_range import rva_to_off

EXE_SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
EPF_SHA="58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e"
WINDOWS={
 "welly_original_tick_and_display":(0x407680,0x407990),
 "welly_dispatcher":(0x407970,0x407A70),
 "ball_special_interaction_dispatch":(0x40A240,0x40A510),
 "ball_terrain_terminal":(0x40A570,0x40A850),
 "timer_callback":(0x401C00,0x401E20)
}
def disassemble(source:bytes,sections,image_base,start,end):
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    start_off,sec=rva_to_off(start-image_base,sections)
    end_off,sec2=rva_to_off(end-image_base-1,sections)
    if start_off is None or end_off is None or sec["name"]!=sec2["name"]:
        raise RuntimeError("requested VAs do not lie inside the original .text bytes")
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    ins=list(md.disasm(source[start_off:end_off+1],start))
    return [{"va":hex(x.address),"op":x.mnemonic,"args":x.op_str}
            for x in ins]
def hit_strings(payload):
    decoded=[]
    # This is a strict ASCII presence inventory, NOT a diagnosis of game
    # modes/AI implemented by the matching binary.
    for m in re.finditer(rb"[\x20-\x7e]{5,}",payload):
        val=m.group().decode("ascii","replace")
        if re.search(r"stroke|skin|match|computer|difficulty|tournament|season|practice|birdie|eagle|putt|welly|meter|player|hole|sound|slope|penalt|water|tree|arrow",val,re.I):
            decoded.append({"offset":hex(m.start()),"value":val[:130]})
    return decoded[:130]
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",required=True,type=Path)
    ap.add_argument("--epf",required=True,type=Path)
    ap.add_argument("--epf-root",required=True,type=Path)
    ap.add_argument("-o","--output",required=True,type=Path)
    a=ap.parse_args()
    exe=a.exe.read_bytes()
    if hashlib.sha256(exe).hexdigest()!=EXE_SHA or hashlib.sha256(a.epf.read_bytes()).hexdigest()!=EPF_SHA:
        raise RuntimeError("Not the previously verified original Windows Sensible Golf v1.014")
    image_base,_,sections=parse_pe(exe)
    blocks={k:disassemble(exe,sections,image_base,*v) for k,v in WINDOWS.items()}
    epf_names=sorted(p.name for p in a.epf_root.iterdir() if p.is_file())
    assert len(epf_names)==277, "Original EPF not fully extracted"
    by_extension=collections.Counter(Path(n).suffix.upper() for n in epf_names)
    warnings=[
      "Opcode listing does NOT prove which path runs in actual gameplay.",
      "The Welly visible pixel boundaries and real-time meter scheduler require call-state emulation.",
      "CPU player policy, tourney/mode menus, special impact object activation and event continuation require runtime traces.",
      "Original source game binaries, graphics, audio and proprietary course layouts are intentionally excluded.",
      "Any exact recovered numeric tables and code require rights review before commercial distribution."
    ]
    report={"schema":"pixtee.research.original-v1014-unresolved-investigation.v1",
            "source":{"exe_sha256":EXE_SHA,"epf_sha256":EPF_SHA,
                      "original_resources":len(epf_names)},
            "resource_extension_counts":dict(sorted(by_extension.items())),
            "original_resource_names":epf_names,
            "menu_and_gameplay_ascii_string_matches":hit_strings(exe),
            "disassembly_windows":blocks,
            "limitations":warnings}
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(report,indent=2)+"\n")
    print("PASS exact original hashes, 277 EPF entries and disassembly windows")
    print("Original resource extensions:",dict(by_extension))
    print("Identified ASCII strings:",len(report["menu_and_gameplay_ascii_string_matches"]))
    for row in report["menu_and_gameplay_ascii_string_matches"][:28]:
        print(" string",row["offset"],repr(row["value"]))
    for name,ins in blocks.items():
        print(f"\n=== {name}: {len(ins)} original x86 instructions ===")
        if name.startswith("welly"):
            for x in ins:
                print(x["va"],x["op"],x["args"])
        else:
            for x in ins[:42]:
                print(x["va"],x["op"],x["args"])
            print("... complete disassembly in JSON artifact; not authoritative dynamic trace")
    print("\nThese are observations, not proof of fully recovered game mechanics.")
if __name__=="__main__": main()
