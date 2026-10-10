#!/usr/bin/env python3
"""Reproduce numerical Sensible Golf v1.014 gameplay evidence DIRECTLY from the
verified executable and extracted GOLF.EPF; never copy original asset payloads.

Output is research metadata, NOT a Pixtee shipping table or licensed game file.
All original map cells/sprites/sounds stay in the ignored private workspace.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
EXPECTED_EXE = "3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
EXPECTED_EPF = "58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e"

def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()

def run_extractor(filename: str, exe: Path) -> dict:
    out = subprocess.check_output([sys.executable, str(ROOT / "tools" / filename),
                                   str(exe)], text=True)
    return json.loads(out)

def be16(buf: bytes, at: int) -> int:
    return struct.unpack_from(">H", buf, at)[0]

def checked_file(folder: Path, name: str) -> bytes:
    path = folder / name
    if not path.is_file():
        raise ValueError("Missing original EPF entry: " + name)
    return path.read_bytes()

def analyse_map(folder: Path, number: int) -> dict:
    code = f"{number:02d}"
    main = checked_file(folder, f"MAPM{code}.MAP")
    green = checked_file(folder, f"MAPS{code}.MAP")
    spt = checked_file(folder, f"MAPM{code}.SPT")
    if len(spt) != 50:
        raise ValueError("SPT record length must be 50: " + code)
    row = {"source_index": number}
    for label, raw in (("main", main), ("green", green)):
        if len(raw) < 96:
            raise ValueError("truncated " + label + " map: " + code)
        w, h = be16(raw, 0x54), be16(raw, 0x56)
        if not w or not h or len(raw) < 96 + w*h*2:
            raise ValueError("invalid " + label + " dimensions: " + code)
        row[label] = {"tile_width": w, "tile_height": h,
                      "file_sha256": hashlib.sha256(raw).hexdigest()}
        if label == "main":
            markers = [(x,y) for y in range(h) for x in range(w)
                       if (be16(raw, 96 + 2*(y*w+x)) & 0xE000) == 0xE000]
            row["green_transition"] = {
                "marker_count": len(markers),
                "first_origin_xy": [markers[0][0]*16,markers[0][1]*8] if markers else None,
            }
    words = [[be16(spt, n*10 + k*2) for k in range(5)] for n in range(5)]
    row["spt"] = {
        "source_sha256": hashlib.sha256(spt).hexdigest(),
        "player_tee_xy": [[words[n][2],words[n][3]] for n in range(4)],
        "cup_xy": [words[4][2],words[4][3]],
        "other_words": "kept private until individually proven (not assumed decor/collisions)",
    }
    return row

def slope_and_surface_histogram(folder: Path, green_maps: list[dict]) -> dict:
    descriptors = checked_file(folder, "MAPI01.RAW")
    selectors = checked_file(folder, "MAPI02.RAW")
    if len(descriptors) != len(selectors) or len(descriptors)%8:
        raise ValueError("MAPI banks are not equally sized, 8-byte aligned")
    # Original 8x4 selectors choose one of four big-endian descriptor words.
    tile_cache = {}
    def decode_tile(tile: int):
        if tile not in tile_cache:
            at=tile*8
            if at+8>len(descriptors):
                raise ValueError("Invalid MAPS MAPI tile index")
            readings=[]
            for sy in range(4):
                for sx in range(8):
                    mask=0x80>>sx
                    offset=(2 if selectors[at+sy]&mask else 0) + (
                        4 if selectors[at+sy+4]&mask else 0)
                    raw=be16(descriptors, at+offset)
                    desc=raw&0xff
                    readings.append((desc if desc<77 else 4,
                                     ((raw>>8)&15)*256, (raw>>12)&15))
            tile_cache[tile]=readings
        return tile_cache[tile]
    terrain=collections.Counter()
    slope=collections.Counter()
    angle=collections.Counter()
    for number in range(1,73):
        green=checked_file(folder,f"MAPS{number:02d}.MAP")
        w,h=be16(green,0x54),be16(green,0x56)
        for cell in range(w*h):
            tile=be16(green,96+cell*2)&0x3ff
            for desc,ang,mag in decode_tile(tile):
                terrain[desc]+=1
                slope[mag]+=1
                if mag: angle[ang]+=1
    return {
        "interpretation": "Counts across green-resource collision subcells; no positional map/graphics payload",
        "mapi_tile_count": len(descriptors)//8,
        "mapi_bank_sha256": {"descriptors": hashlib.sha256(descriptors).hexdigest(),
                             "selectors": hashlib.sha256(selectors).hexdigest()},
        "terrain_descriptor_counts": dict(sorted(terrain.items())),
        "slope_magnitude_counts": dict(sorted(slope.items())),
        "nonzero_slope_direction_counts": dict(sorted(angle.items())),
        "slope_direction_unit": "raw 12-bit angle (nibble * 256)",
    }

def main() -> None:
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",type=Path,required=True)
    ap.add_argument("--epf",type=Path,required=True)
    ap.add_argument("--epf-root",type=Path,required=True)
    ap.add_argument("-o","--output",type=Path,required=True)
    args=ap.parse_args()
    if sha(args.exe)!=EXPECTED_EXE or sha(args.epf)!=EXPECTED_EPF:
        raise SystemExit("REJECT: original source SHA-256 does not match verified v1.014")
    clubs=run_extractor("extract_club_table.py",args.exe)
    accuracy=run_extractor("extract_live_accuracy_tables.py",args.exe)
    terrain=run_extractor("terrain_descriptor_probe.py",args.exe)["rows"][:77]
    if len(clubs["clubs"])!=13 or len(accuracy["selectors"])!=13 or len(terrain)!=77:
        raise SystemExit("REJECT: source data shapes differ")
    if not all(row["mapped"] for row in terrain):
        raise SystemExit("REJECT: original terrain table does not map 77 entries")
    expected=json.loads((ROOT/"spec/pixtee/ORIGINAL_PHYSICS_RESEARCH.json").read_text())
    loaded=[dict(index=v["index"],vertical_base=v["loaded_vertical_base"],
                 horizontal_base=v["loaded_horizontal_base"],
                 power_scale=v["power_scale"],special_putter_path=v["special_putter_path"])
            for v in clubs["clubs"]]
    if loaded!=expected["recovered_club_launch_research"]:
        raise SystemExit("REJECT: direct original binary club values differ from frozen reference")
    if [x["values"] for x in clubs["lie_distance_rows"]] != expected["recovered_club_lie_profile_selectors"]:
        raise SystemExit("REJECT: direct binary club/lie selector differs")
    if accuracy["selectors"] != expected["recovered_club_lie_profile_selectors"]:
        raise SystemExit("REJECT: original live accuracy selector differs")
    bounds=[{"min":x["lower"],"max":x["upper"]} for x in accuracy["bounds"]]
    if bounds!=expected["recovered_swing_profile_accuracy_bounds"]:
        raise SystemExit("REJECT: binary accuracy bounds differ")
    for i,p in enumerate(accuracy["profiles"]):
        for j,error in enumerate(expected["recovered_swing_profile_evenized_error_columns"]):
            val=p["samples_by_even_error"].get(str(error))
            if val is not None and val != expected["recovered_swing_profile_samples"][i][j]:
                raise SystemExit("REJECT: original binary swing profile differs")
    binary_terrain=[{"index":v["index"],"landing_code":v["landing_code"],
                     "variant":v["variant_signed"],"lie_profile_slot":v["profile_slot"],
                     "name":v["name"]} for v in terrain]
    if binary_terrain!=expected["recovered_terrain_descriptor_research"]:
        raise SystemExit("REJECT: original binary terrain table differs")
    holes=[analyse_map(args.epf_root,n) for n in range(1,73)]
    playable=slope_and_surface_histogram(args.epf_root,holes)
    graphics=[]
    for ext in (".MCH",".LBM",".BIN",".RAW",".DAT"):
        for p in sorted(args.epf_root.glob("*"+ext)):
            graphics.append({"name":p.name,"bytes":p.stat().st_size,
                             "sha256":sha(p)})
    report={
        "schema":"pixtee.research.original-binary-hard-data.v1",
        "status":"verified-extraction-from-original-game-files",
        "source":{"game":"Sensible Golf Windows v1.014","exe_sha256":EXPECTED_EXE,
                  "epf_sha256":EXPECTED_EPF},
        "measured_binary_tables":{"clubs_raw_and_loaded":clubs["clubs"],
                                  "club_lie_selectors":accuracy["selectors"],
                                  "accuracy_bounds":accuracy["bounds"],
                                  "swing_profiles":accuracy["profiles"],
                                  "terrain_descriptors":binary_terrain},
        "original_game_resource_metadata":{"course_resource_count":72,
                                           "course_source_map_metadata":holes,
                                           "original_green_collision_slope_histogram":playable,
                                           "graphics_audio_asset_inventory":graphics},
        "unresolved_not_to_fake":[
            "Exact original Welly-o-meter cursor timing and red/yellow/black pixel regions",
            "Tree/object-hit conditions, response and any particle/impact sprite frame timing",
            "Water splash, sand puff, bounce, cup/flag, lie and putting arrow animation frame sequences",
            "Full extreme original accuracy handling and special green event-11 continuation",
            "Exact original drawn club labels and maximum display yards by club",
            "Licensing of protected original course maps, images, sound and binaries"],
        "legal_scope":"Research provenance and source-derived values only; do not ship original images, course grids, MAPI banks or commercial binaries in independent Pixtee.",
    }
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
    print(f"PASS: verified game EXE+EPF, 13 clubs, 13x10 lies, 11 profiles, "
          f"77 terrains, {len(holes)} main/green/SPT sets, "
          f"{len(graphics)} graphics/audio metadata entries")
    print("Original green slope distributions and 72 source tee/cup locations in",args.output)

if __name__=="__main__":
    main()
