#!/usr/bin/env python3
"""Ensure Astra's expanded art coverage stays complete (not a claim of approved artwork)."""
import json, re, sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
def ensure(ok,msg):
    if not ok: raise AssertionError(msg)

def main():
    spec=json.loads((ROOT/"spec/pixtee/ASTRA_AMBIENCE_AND_IMPACT_MANIFEST_v2.json").read_text())
    bible=(ROOT/"spec/pixtee/ASTRA_FINAL_ART_ASSET_BIBLE_2026-10-10.md").read_text()
    courses=(ROOT/"pixtee-android/app/src/main/java/com/pixtee/golf/PixteeCourses.kt").read_text()
    ensure(spec["schema"]=="pixtee.art.complete-ambience-impact-production-v2","wrong schema")
    ensure(spec["authored_art_status"]=="PENDING_ALL_OWNER_APPROVALS","art approval misrepresented")
    ensure("PENDING" in bible and "## 11. CANONICAL EXTENSION" in bible, "brief does not include expanded full coverage")
    match=re.search(r"val courses: List<PixteeCourse> = listOf\((.*?)\)\.mapIndexed", courses, re.S)
    ensure(match is not None,"failed to find source catalog")
    names=re.findall(r'"([^"]+)"', match.group(1))
    names=[s.lower().replace(" ","-") for s in names]
    original=[c["id"] for c in spec["courses"]]
    ensure(names==original and len(names)==35,"35 authored course biome index drifted from code")
    species=spec["species_dimensions"]
    ensure(len(species)==spec["species_count"]==21,"wildlife species count invalid")
    ensure(len(spec["courses"])==spec["course_count"]==35,"course count invalid")
    for c in spec["courses"]:
        ensure(c["wildlifeBiome"] in spec["biome_species"],c["id"]+" missing biome")
        ensure(set(c["species"])==set(spec["biome_species"][c["wildlifeBiome"]]),c["id"]+" species mismatch")
        ensure(set(c["species"]).issubset(species),c["id"]+" has unexported species")
    g=spec["groups"]
    ensure(len(g)==len(set(x["id_template"] for x in g)),"duplicate asset pattern")
    for row in g:
        x=row["export_px"]; w=row["world_wh"]
        ensure(x["w"] >0 and x["h"]>0,"invalid export dimensions")
        ensure(x["w"]==round(w[0]*8) and x["h"]==round(w[1]*8),
               row["id_template"]+" does not map 8 px/world")
        ensure(row["total_frames"]==row["frames_per_variant"]*row["variants"],
               row["id_template"]+" wrong frame count")
        if row["asset_class"] in ("bird","golfer","impact","advertising"):
            pattern = row["id_template"]
            if row["asset_class"] == "bird":
                pattern = re.sub(r"^bird_.*_(fly|hit|recover)_", r"bird_<species>_\\1_", pattern)
            ensure(pattern in bible,"MD misses generic asset family "+pattern)
    totals={}
    for row in g: totals[row["asset_class"]]=totals.get(row["asset_class"],0)+row["total_frames"]
    ensure(totals==spec["totals_frames_by_asset_class"],"manifest total frames drifted")
    ensure(sum(totals.values())==spec["total_animation_or_art_frames"]==1655,"wrong complete frame total")
    ensure(totals["golfer"]==576 and totals["bird"]==21*26 and totals["impact"]>=140,"required actions missing")
    for species_name in species:
        for action in ("fly","hit","recover"):
            ensure(any(r["id_template"].startswith(f"bird_{species_name}_{action}_")
                for r in g), f"missing {species_name} {action}")
    for prefix in ("fx_bunker_ball_impact","fx_bunker_explosion","fx_water_entry",
                   "fx_water_ripple_ring","fx_water_sink","fx_tree_trunk_hit",
                   "fx_tree_canopy_hit","fx_tree_leaf_scatter",
                   "green_slope_arrow","spectator_photographer_raise",
                   "spectator_photographer_shoot","camera_flash"):
        ensure(any(x["id_template"].startswith(prefix) for x in g),"missing "+prefix)
    for part in ("tee_left_front","tee_right_front","green_left_front","green_right_front"):
        ensure(any("board_"+part==x["id_template"] for x in g),"missing board facing "+part)
    boards=spec["billboards"]
    ensure(boards["per_hole"]==4 and boards["tee"]==2 and boards["green"]==2
           and boards["noncolliding"] is True and boards["export_canvas_px"]==[352,160],
           "advertising board geometry/face contract drift")
    ensure("normalize(tee_xy-sign_xy)" in boards["facing_normal"],"tee faces wrong direction")
    ensure("cup_or_approach_xy-sign_xy" in boards["facing_normal"],"green faces wrong direction")
    game=spec["bird_collision_special"]
    ensure(game["status"]=="PIXTEE_NEW_OPTIONAL_NONPHYSICS_VISUAL"
           and game["physics_rng"]=="separate cosmetic RNG"
           and game["ball_after_event"]=="identical world trajectory unchanged",
           "comic bird hit changed original-game physics contract")
    print(f"PASS: 35 original Pixtee courses have biome assignments; 21 bird species with fly/hit/recover; "
          f"{totals['golfer']} directional golfer frames; {totals['impact']} sandboxed impact effect frames; "
          f"4 tee/green-facing boards per hole; {sum(totals.values())} proposed art-frame outputs covered.")
    print("NOTE: This checks SPEC COVERAGE only; no images, collision response or animation timing are marked approved.")

if __name__=="__main__":
    try: main()
    except (OSError,KeyError,ValueError,AssertionError) as e: sys.exit("FAIL expanded Astra art coverage: "+str(e))
