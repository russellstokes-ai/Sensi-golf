#!/usr/bin/env python3
"""Fail closed on lost features and impossible 100%-parity claims."""
import json, pathlib, sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=ROOT/"spec/pixtee/ENGINE_COMPLETENESS_GATE_2026-10-11.json"
j=json.loads(p.read_text())
assert j["schema"]=="pixtee.original-gameplay-coverage.v1"
assert j["parity_claim"]=="UNPROVEN"
assert j["engine_completion"]=="INCOMPLETE"
assert len(j["features"])>=65, "Sensible Golf exhaustive mechanics register has lost entries"
ids=[f["id"] for f in j["features"]]
assert len(ids)==len(set(ids)), "Duplicated feature IDs"
for f in j["features"]:
    assert (ROOT/f["source"]).is_file(), f["id"]+" references missing original module"
    assert (ROOT/f["evidence_reference"]).is_file()
    assert f["status"] in ("native_reference_available","unresolved_or_not_integrated")
    assert f["release_gate"]=="BLOCKED_UNTIL_ORIGINAL_PARITY_VERIFIED"
critical=("welly_accurate_meter_timing","tree_collision_activation",
          "code9_special_green","code10_special_green",
          "swing_accuracy_extremes","multi_player_turn_order")
for k in critical:
    assert k in ids and next(x for x in j["features"] if x["id"]==k)["status"]=="unresolved_or_not_integrated", k
blocked=sum(x["status"]=="unresolved_or_not_integrated" for x in j["features"])
print(f"PASS inventory: {len(ids)} distinct Sensible Golf requirements preserved; {blocked} not fully integrated. This is NOT 100% parity.")
if "--release-gate" in sys.argv:
    if blocked or any(x["godot_engine_proof"]!="original_parity_verified" for x in j["features"]):
        sys.exit("RELEASE GATE BLOCKED: original-game parity remains unproven")
