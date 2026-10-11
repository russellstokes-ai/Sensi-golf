#!/usr/bin/env python3
"""Original Sensible Golf v1.014 geometry/sprite/collision dimensions audit.

Reads actual hash-verified original GOLFWIN.EXE and GOLF.EPF via existing
game-file extractors; emits numerical/metadata facts, not copyrighted sprite
pixels, tile arrangements, original map topology, or audio payloads.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
EXE_SHA="3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
EPF_SHA="58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e"

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def run_tool(name,arguments):
    p=subprocess.run([sys.executable,str(ROOT/"tools"/name),*map(str,arguments)],
                     capture_output=True,text=True,check=True)
    print(p.stdout.strip())

def u16(b,pos):
    return struct.unpack_from(">H",b,pos)[0]

def metrics(items):
    if not items:return {"min":None,"max":None}
    return {"min":min(items),"max":max(items)}

def map_cells(folder,seq,green,banks):
    prefix=f"MAPS{seq:02d}.MAP" if green else f"MAPM{seq:02d}.MAP"
    data=(folder/prefix).read_bytes()
    w,h=u16(data,0x54),u16(data,0x56)
    assert len(data)>=96+2*w*h
    selector,descriptor=banks
    cache={}
    def shape(tile):
        if tile not in cache:
            start=tile*8
            if start+8>len(selector):raise AssertionError(f"bad MAPI tile {tile}")
            rr=[]
            for y in range(4):
                for x in range(8):
                    mask=128>>x
                    ofs=(2 if selector[start+y]&mask else 0)+(4 if selector[start+y+4]&mask else 0)
                    val=u16(descriptor,start+ofs)
                    did=val&255
                    if did>=77:did=4
                    rr.append((did,((val>>8)&15)*256,(val>>12)&15))
            cache[tile]=rr
        return cache[tile]
    desc=Counter();slope=Counter();slope_angles=Counter();tiles=Counter()
    for cell in range(w*h):
        index=u16(data,96+2*cell)&0x3ff
        tiles[index]+=1
    for index,num in tiles.items():
        for did,angle,magnitude in shape(index):
            desc[did]+=num
            slope[magnitude]+=num
            if magnitude:slope_angles[angle]+=num
    assert sum(desc.values())==w*h*32
    return {
        "resource":prefix,
        "map_tiles_wh":[w,h],
        "world_wh":[w*16,h*8],
        "fine_collision_cells_wh":[w*8,h*4],
        "fine_collision_cell_world_wh":[2,2],
        "fine_collision_cell_count":w*h*32,
        "distinct_source_mapi_tile_indexes":len(tiles),
        "collision_descriptor_histogram":dict(sorted(desc.items())),
        "slope_magnitude_histogram":dict(sorted(slope.items())),
        "slope_direction_nonzero_magnitude_histogram":dict(sorted(slope_angles.items())),
        "scope":"Aggregate category and slope counts only; full protected original map topology not exported."
    }

def distance(a,b):
    from math import isqrt
    d=isqrt((int(a[0])-int(b[0]))**2+(int(a[1])-int(b[1]))**2)
    return d*6//10

def source_assets(sprite):
    records=[]
    for r in sprite["resources"]:
        basic={"source_file":r["file"],"sha256":r["sha256"],"bytes":r["bytes"],
               "format_status":r["status"]}
        if r["file"].endswith(".MCH"):
            frames=r["frame_details"]
            bbox=[f["painted_bbox"] for f in frames if "painted_bbox" in f]
            # Some legitimate original MCH frames contain no painted pixels.
            # Preserve the frame/timing slot with a null painted rectangle.
            basic.update({
              "frames":len(frames),
              "original_frames":[{"frame":f["frame"],
                       "padded_rect_px":[0,0,f["width_px"],f["height_px"]],
                       "painted_rect_px":[f["painted_bbox"][k] for k in ("x","y","width_px","height_px")] if "painted_bbox" in f else None,
                       "painted_pixel_count":f["painted_pixels"]}
                       for f in frames],
              "frame_size_variants":r["unique_frame_sizes"],
              "painted_width_range":metrics([b["width_px"] for b in bbox]),
              "painted_height_range":metrics([b["height_px"] for b in bbox]),
              "blank_frame_count":len(frames)-len(bbox)
            })
        else:
            basic.update({"original_bitmap_px":[r.get("width_px"),r.get("height_px")],
                          "iff_variant":r.get("iff_form")})
        records.append(basic)
    return records

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--exe",required=True,type=Path)
    ap.add_argument("--epf",required=True,type=Path)
    ap.add_argument("--epf-root",required=True,type=Path)
    ap.add_argument("--output",required=True,type=Path)
    a=ap.parse_args()
    if sha(a.exe)!=EXE_SHA or sha(a.epf)!=EPF_SHA:
        raise SystemExit("REJECT: expected original v1.014 source game fingerprints")
    with tempfile.TemporaryDirectory() as d:
        path=Path(d)
        hard=path/"hard.json";sprite=path/"sprites.json"
        run_tool("extract_pixtee_original_binary_hard_data.py",
                 ["--exe",a.exe,"--epf",a.epf,"--epf-root",a.epf_root,"--output",hard])
        run_tool("extract_sensible_original_sprite_dimensions.py",
                 ["--epf",a.epf,"--epf-root",a.epf_root,"--output",sprite])
        hard=json.loads(hard.read_text());sprite=json.loads(sprite.read_text())
    folder=a.epf_root
    descriptor=(folder/"MAPI01.RAW").read_bytes()
    selector=(folder/"MAPI02.RAW").read_bytes()
    assert len(descriptor)==len(selector) and len(descriptor)%8==0
    assert len(hard["original_game_resource_metadata"]["course_source_map_metadata"])==72
    courses=[]
    for meta in hard["original_game_resource_metadata"]["course_source_map_metadata"]:
        i=meta["source_index"]
        main=map_cells(folder,i,False,(selector,descriptor))
        green=map_cells(folder,i,True,(selector,descriptor))
        spt=meta["spt"]
        tee=spt["player_tee_xy"];cup=spt["cup_xy"]
        courses.append({
          "source_game_file_index":i,
          "main":main,"detailed_green":green,
          "tee_positions_raw_game_world":tee,"cup_position_raw_game_world":cup,
          "tee_to_cup_game_distance_for_slots":[distance(t,cup) for t in tee],
          "spt_original_sha256":spt["source_sha256"],
          "green_transition_markers":meta["green_transition"]})
    assert len(courses)==72
    files=[]
    for p in sorted(folder.iterdir()):
        if p.is_file():files.append({"file":p.name,"bytes":p.stat().st_size,"sha256":sha(p)})
    assert len(files)==277
    asset=source_assets(sprite)
    assert len(asset)==34
    width_metrics=[x["main"]["world_wh"] for x in courses]
    green_metrics=[x["detailed_green"]["world_wh"] for x in courses]
    mtype=Counter(tuple(x["main"]["map_tiles_wh"]) for x in courses)
    gtype=Counter(tuple(x["detailed_green"]["map_tiles_wh"]) for x in courses)
    total_frames=sum(x.get("frames",0) for x in asset)
    master={
        "schema":"pixtee.reference.original-exe-epf-all-measured-geometry.v1",
        "source":{"title":"Sensible Golf Windows v1.014",
                  "original_exe_sha256":EXE_SHA,"original_epf_sha256":EPF_SHA,
                  "source_files_directly_read":True},
        "status":"authentic-file-verified numerical measurement; sprites collisions and renderer placement NOT conflated",
        "asset_inventory":{
            "original_asset_resources_measured":len(asset),
            "mch_sprite_files":sum(x["source_file"].endswith(".MCH") for x in asset),
            "lbm_bitmaps":sum(x["source_file"].endswith(".LBM") for x in asset),
            "mch_frames":total_frames,
            "original_source_asset_rectangles_and_painted_bounding_boxes":asset
        },
        "collision_contract":{
            "collision_shape_type":"MAPI terrain descriptor subcell lookup; NOT rectangle hitboxes per artwork frame",
            "main_course_tile_world_wh":[16,8],
            "fine_collision_cells_per_tile_wh":[8,4],
            "fine_collision_cell_world_wh":[2,2],
            "selector_source":"MAPI02.RAW",
            "descriptor_source":"MAPI01.RAW",
            "selector_record_bytes":8,
            "descriptor_record_bytes":8,
            "binary_geometry_routine":"engine/src/recovered_course_lookup.cpp",
            "separate_actual_object_hitboxes":"NOT VERIFIED; tree WOOD*.BIN artwork metadata cannot establish physical collision boxes",
            "ball_collision_shape_radius":"NOT VERIFIED; original course collision samples world coordinates; do not fabricate circular collider radius",
            "sprite_painted_rect_vs_hitbox":"painted bbox is NOT a collision box, and does not establish in-world render anchor",
            "terrain_derived_hazards":"descriptor-driven codes such as 35 for water/no-go/out-of-bounds",
            "all_course_surface_histograms":"found individually in each original_course_resource entry",
        },
        "course_inventory":{
            "source_course_resources":72,
            "all_main_map_tile_size_variants":[{"tiles_wh":list(k),"count":v} for k,v in sorted(mtype.items())],
            "all_green_map_tile_size_variants":[{"tiles_wh":list(k),"count":v} for k,v in sorted(gtype.items())],
            "main_world_bounds_ranges":{"width_world":metrics([v[0] for v in width_metrics]),
                                        "height_world":metrics([v[1] for v in width_metrics])},
            "detailed_green_world_bounds_ranges":{"width_world":metrics([v[0] for v in green_metrics]),
                                             "height_world":metrics([v[1] for v in green_metrics])},
            "original_course_resources":courses
        },
        "original_archive_all_277_file_inventory":files,
        "original_binary_mechanics":{
            "original_club_launch_parameters":hard["measured_binary_tables"]["clubs_raw_and_loaded"],
            "13_x_10_club_lie_selector":hard["measured_binary_tables"]["club_lie_selectors"],
            "11_profile_accuracy_bounds":hard["measured_binary_tables"]["accuracy_bounds"],
            "11_swing_curve_profiles":hard["measured_binary_tables"]["swing_profiles"],
            "77_terrain_descriptors":hard["measured_binary_tables"]["terrain_descriptors"],
        },
        "pixtee_scale_locks":{
            "source_art_density_multipliers_supported":[2,3,4],
            "example":"16x21 pixel source frame -> 48x63 pixel newly authored 3x sprite source; render to the SAME reference world/visible screen bounding box, not a 3x taller golfer",
            "source_frames_are_not_screen_scale_proof":True,
            "camera":"same true top-down ball-follow view; crop wider course to phone portrait rather than fitting whole original hole",
            "requirements_before_exact_game_view_claim":[
                "measure original sprite blit coordinates and logical game camera scale",
                "identify original frame indices for playable character/swing, ball and scenery",
                "prove original physical object collisions/height/tree reactions against executable",
                "read original dynamic terrain/slope/ball lie animation indices from render path"],
        },
        "unverified":[
            "pixel-to-game-world transform for every original MCH resource",
            "actual per-sprite collision rectangles or circles; original terrain uses MAPI subcells",
            "tree hitbox and vertical clearance, and exact response",
            "ball size/impact shape (different from visual icon)",
            "non-MCH sprite content in original level tile resources",
            "all animation tick timings and per-frame roles",
            "commercial rights to reuse original game data, art or maps"],
        "legal":"Restricted historical numerical research/reference; do not ship original game maps, graphics or audio in independent Pixtee. Build independently authored more detailed artwork & original course designs."
    }
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(master,indent=2)+"\n",encoding="utf-8")
    print(f"PASS: 72 source course resource dimension+collision histograms; {total_frames} measured MCH frames; 277 archive files; 13 source-game clubs; 77 terrains -> {a.output}")

if __name__=="__main__":
    main()
