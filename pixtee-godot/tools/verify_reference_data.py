#!/usr/bin/env python3
"""Verify the Godot engineering bank is the real source-derived Sensi data.

No creative defaults: fail immediately on changed source data, missing rows,
unverifiable fixture, or mismatch with the recovered C++ physics tables.
"""
import json
import pathlib
import runpy

ROOT = pathlib.Path(__file__).resolve().parents[2]
BANK = ROOT / "pixtee-godot/reference"
original = ROOT / "spec/pixtee/ORIGINAL_PHYSICS_RESEARCH.json"
copy = BANK / "ORIGINAL_PHYSICS_RESEARCH.json"
original_round = ROOT / "engine/tests/fixtures/original_round_all_eighteen.txt"
copy_round = BANK / "ORIGINAL_ROUND_REFERENCE.txt"

def main():
    assert copy.read_bytes() == original.read_bytes(), "Recovered physics bank differs from verified canonical data"
    assert copy_round.read_bytes() == original_round.read_bytes(), "Original 18-hole round fixture was altered"
    data = json.loads(copy.read_text(encoding="utf-8"))
    assert data["source_executable_sha256"] == "3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
    assert len(data["recovered_club_launch_research"]) == 13
    assert len(data["recovered_club_lie_profile_selectors"]) == 13
    assert all(len(x) == 10 for x in data["recovered_club_lie_profile_selectors"])
    assert len(data["recovered_swing_profile_accuracy_bounds"]) == 11
    assert len(data["recovered_swing_profile_samples"]) == 11
    assert len(data["recovered_terrain_descriptor_research"]) == 77
    assert data["established"]["raw_power_max"] == 105
    assert data["established"]["accuracy_center"] == 63
    assert data["established"]["original_logical_timer_fixed_interval"] == 936
    assert data["established"]["max_holes"] == 18
    assert "UNREVIEWED" in data["legal_reuse_status"]
    fixture_lines = [line.split() for line in copy_round.read_text().splitlines()
                     if line.strip() and not line.startswith("#")]
    assert len(fixture_lines) == 18, "Expected 18 genuine fixture records"
    for row in fixture_lines:
        hole_id, par, tee_x, tee_y, cup_x, cup_y, n = map(int,row[:7])
        assert hole_id > 0 and 3 <= par <= 5 and n >= 1
        assert len(row) == 7 + n*4
        for off in range(7,len(row),4):
            heading,power,accuracy,club = map(int,row[off:off+4])
            assert 0 <= heading < 4096 and 0 <= power <= 105
            assert 0 <= accuracy <= 105 and 0 <= club < 13
    # The already-established validator compares exact C++ launch/lie/terrain
    # arrays against source-derived research. Execute, do not duplicate rules.
    runpy.run_path(str(ROOT/"tools/validate_pixtee_physics_reference.py"),
                   run_name="__main__")
    print("PASS Godot original-data evidence bank: 13 original club rows,"
          " 13x10 lies, 11 bounds, 11 profiles, 77 terrains, 18 real round traces")

if __name__ == "__main__":
    main()
