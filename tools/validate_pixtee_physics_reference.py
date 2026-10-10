#!/usr/bin/env python3
"""Audit factual Pixtee research data against the verified Sensi-golf recovery.

Only checks provenance and correspondence. Does NOT generate a Pixtee runtime
table and does NOT grant original-game source or asset redistribution rights.
"""
from __future__ import annotations
import json
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[1]
REF = ROOT / "spec/pixtee/ORIGINAL_PHYSICS_RESEARCH.json"
SRC = ROOT / "engine/src/recovered_flight.cpp"
TERRAIN = ROOT / "engine/src/classic_terrain.cpp"

def block(src: str, pattern: str) -> str:
    found = re.search(pattern, src, re.S)
    if not found:
        raise AssertionError(f"original source block missing: {pattern}")
    return found.group(1)

def main() -> None:
    data = json.loads(REF.read_text(encoding="utf-8"))
    flight = SRC.read_text(encoding="utf-8")
    terrain = TERRAIN.read_text(encoding="utf-8")

    club = block(flight, r"kClubTable\{\{([\s\S]*?)\}\};")
    row = re.findall(r"\{\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(true|false)\s*\}", club)
    got_clubs = [
        dict(index=i, vertical_base=int(v), horizontal_base=int(h),
             power_scale=int(s), special_putter_path=(putter=="true"))
        for i,(v,h,s,putter) in enumerate(row)
    ]
    assert len(got_clubs)==13
    assert data["recovered_club_launch_research"]==got_clubs

    selectors = block(flight, r"kProfileSelector\{\{([\s\S]*?)\}\};")
    got_selectors = [
        list(map(int, re.findall(r"\d+", row)))
        for row in re.findall(r"\{\{([\d,\s]+)\}\}", selectors)
    ]
    assert len(got_selectors)==13 and all(len(r)==10 for r in got_selectors)
    assert data["recovered_club_lie_profile_selectors"]==got_selectors

    bounds = block(flight, r"kProfileBounds\{\{([\s\S]*?)\}\};")
    got_bounds = [
        {"min":int(a),"max":int(b)}
        for a,b in re.findall(r"\{\s*(\d+),\s*(\d+)\s*\}",bounds)
    ]
    assert len(got_bounds)==11
    assert data["recovered_swing_profile_accuracy_bounds"]==got_bounds

    samples = block(flight, r"kProfileSamples\{\{([\s\S]*?)\}\};")
    got_samples = [
        list(map(int, re.findall(r"-?\d+", row)))
        for row in re.findall(r"\{\{([\d,\s-]+)\}\}",samples)
    ]
    assert len(got_samples)==11 and all(len(r)==8 for r in got_samples)
    assert data["recovered_swing_profile_samples"]==got_samples

    terrain_block = block(terrain, r"kTerrain\{\{([\s\S]*?)\}\};")
    entries = re.findall(
        r'\{\s*(\d+),\s*(-?\d+),\s*(\d+),\s*"([^"]+)",supported\(\d+\)\}',
        terrain_block,
    )
    got_terrain = [
        dict(index=i, landing_code=int(landing), variant=int(variant),
             lie_profile_slot=int(slot), name=name)
        for i,(landing,variant,slot,name) in enumerate(entries)
    ]
    assert len(got_terrain)==77
    assert data["recovered_terrain_descriptor_research"]==got_terrain

    assert data["schema"]=="pixtee-research.original-physics.v1"
    assert "UNREVIEWED" in data["legal_reuse_status"]
    assert data["established"]["directions_per_circle"]==4096
    assert data["established"]["raw_power_max"]==105
    assert data["established"]["accuracy_center"]==63
    assert data["established"]["gravity_per_tick"]==0x2100
    assert data["established"]["normal_drag"]==0x0F00
    assert data["established"]["putter_green_drag"]==0x0780
    assert data["established"]["max_holes"]==18
    print("PASS original physics research source audit: 13 clubs,"
          " 13x10 selectors, 11 bounds/profiles, 77 terrains, key constants")

if __name__ == "__main__":
    main()
