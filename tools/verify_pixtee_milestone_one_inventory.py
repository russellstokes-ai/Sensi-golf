#!/usr/bin/env python3
"""Fail-closed audit of Pixtee Milestone One engine data/provenance and release claims.

Default mode verifies the documented source bank and truthful OPEN release
gates. Success in default mode is ONLY a documentation/data-integrity result,
NOT a playable, licensed or commercially ready game engine certificate.

--require-commercial additionally REJECTS a milestone with any RED/unproved
gate, under-three completed authored holes or uncleared distribution rights.
This command is intended to fail until the game is genuinely finished.
"""
from __future__ import annotations
import argparse
import json
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INVENTORY = ROOT / "spec/pixtee/MILESTONE_ONE_ENGINE_DATA_INVENTORY_2026-10-12.json"
HANDOVER = ROOT / "spec/pixtee/MILESTONE_ONE_COMMERCIAL_ENGINE_HANDOVER_2026-10-12.md"
BASELINE = ROOT / "spec/pixtee/ENGINE_COMPLETENESS_GATE_2026-10-11.json"
SOURCE_BANK = ROOT / "spec/pixtee/ORIGINAL_PHYSICS_RESEARCH.json"
COPIED_BANK = ROOT / "pixtee-godot/reference/ORIGINAL_PHYSICS_RESEARCH.json"
SOURCE_ROUND = ROOT / "engine/tests/fixtures/original_round_all_eighteen.txt"
COPIED_ROUND = ROOT / "pixtee-godot/reference/ORIGINAL_ROUND_REFERENCE.txt"
ORIGINAL_SHA = "3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8"
ALLOWED_STATES = {
    "RESEARCH_PROVEN", "INTEGRATION_PARTIAL", "BLOCKED", "PRODUCT_CONTENT_MISSING"
}
GATES = {f"M1-{i:02d}" for i in range(1, 15)}

def readable_relative(relative: str) -> bool:
    if not isinstance(relative, str) or not relative or relative.startswith("/"):
        return False
    try:
        path = (ROOT / relative).resolve()
        path.relative_to(ROOT.resolve())
    except ValueError:
        return False
    return path.is_file()

def audit(require_commercial: bool) -> tuple[list[str], dict]:
    errors: list[str] = []
    if not INVENTORY.is_file() or not HANDOVER.is_file():
        return ["Milestone One source inventory or handover missing"], {}
    try:
        doc = json.loads(INVENTORY.read_text(encoding="utf-8"))
        baseline = json.loads(BASELINE.read_text(encoding="utf-8"))
        bank = json.loads(SOURCE_BANK.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        return [f"Could not parse source/reference: {exc}"], {}

    if doc.get("schema") != "pixtee.milestone-one-engine-data-gates.v1":
        errors.append("Milestone One schema is not recognized")
    if doc.get("reference_sha256") != ORIGINAL_SHA:
        errors.append("Original source binary hash drift")
    if bank.get("source_executable_sha256") != ORIGINAL_SHA:
        errors.append("Original research bank binary hash drift")
    if baseline.get("engine_completion") != "INCOMPLETE" and not require_commercial:
        errors.append("Baseline original-game parity status changed; review this audit")
    features = baseline.get("features", [])
    if len(features) != doc.get("baseline_known_features"):
        errors.append(f"Original 73-feature inventory changed ({len(features)}); reconcile records")
    if not features or len(features) != 73:
        errors.append("Incomplete canonical original-game baseline")
    if doc.get("source_data_bank") != "pixtee-godot/reference/ORIGINAL_PHYSICS_RESEARCH.json":
        errors.append("Unexpected source evidence bank location")
    if SOURCE_BANK.read_bytes() != COPIED_BANK.read_bytes():
        errors.append("Godot original research bank must exactly match canonical research")
    if SOURCE_ROUND.read_bytes() != COPIED_ROUND.read_bytes():
        errors.append("Godot original numeric round fixture differs from canonical fixture")

    records = doc.get("required_sources", [])
    ids = [r.get("id") for r in records]
    if len(records) != 27 or len(ids) != len(set(ids)):
        errors.append("Milestone One requires 27 unique audited data systems")
    counts = Counter()
    source_count = 0
    for rec in records:
        name = str(rec.get("id"))
        state = rec.get("state")
        counts[state] += 1
        if state not in ALLOWED_STATES:
            errors.append(f"{name}: unknown source state: {state}")
        if not isinstance(rec.get("evidence"), str) or not rec["evidence"]:
            errors.append(f"{name}: source evidence missing")
        if not isinstance(rec.get("release_gap"), str) or not rec["release_gap"]:
            errors.append(f"{name}: remaining acceptance step missing")
        if not isinstance(rec.get("paths"), list) or not rec["paths"]:
            errors.append(f"{name}: required file paths missing")
            continue
        for relative in rec["paths"]:
            source_count += 1
            if not readable_relative(relative):
                errors.append(f"{name}: tracked source path absent or inaccessible: {relative}")
    courses = doc.get("playable_course_status", {})
    if courses.get("required_distinct_complete_courses") != 3:
        errors.append("Milestone One requires exactly 3 complete independent courses")
    if not readable_relative("pixtee-godot/native/src/pixtee_course_adapter.cpp"):
        errors.append("Original course adaptation evidence missing")

    gates = doc.get("milestone_one_acceptance", [])
    if len(gates) != 14 or {g.get("id") for g in gates} != GATES:
        errors.append("Must retain all 14 separately signed Milestone One release gates")
    for gate in gates:
        gid = gate.get("id", "UNKNOWN")
        status = gate.get("status")
        if status not in {"RED", "PASS"}:
            errors.append(f"{gid}: invalid gate status")
        if not isinstance(gate.get("criterion"), str) or not gate["criterion"]:
            errors.append(f"{gid}: missing acceptance criterion")
        if status == "PASS":
            evidence = gate.get("evidence_links", [])
            if not evidence or not all(
                isinstance(x, str) and
                (x.startswith("https://") or readable_relative(x))
                for x in evidence
            ):
                errors.append(f"{gid}: PASS without a linked test/provenance evidence")
            if gate.get("signoff_owner") in ("", "PENDING", None):
                errors.append(f"{gid}: PASS without accountable named signoff")

    all_pass = all(g.get("status") == "PASS" for g in gates) and len(gates) == 14
    three_holes = courses.get("verified_production_courses") == 3
    rights = doc.get("rights_clearance") == "APPROVED"
    ready = bool(doc.get("commercial_engine_ready"))
    if ready != (
        all_pass and three_holes and rights and
        doc.get("full_three_hole_release_gate") == "GREEN"
    ):
        errors.append(
            "Commercial-ready claim disagrees with evidence/14 gates/3 holes/legal clearance"
        )
    if require_commercial and not ready:
        errors.append(
            "COMMERCIAL RELEASE BLOCKED: cannot substitute passing audit for "
            "3 human-playable holes, completed 14-gate QA and rights clearance"
        )
    return errors, {
        "required_data_systems": len(records),
        "source_references": source_count,
        "states": dict(counts),
        "original_baseline_features": len(features),
        "release_gates_passed": sum(g.get("status") == "PASS" for g in gates),
        "release_gates_total": len(gates),
        "approved_production_holes": courses.get("verified_production_courses"),
        "commercial_engine_ready": ready,
    }

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--require-commercial", action="store_true",
        help="Require 3 truly finished production holes and all release gates PASS",
    )
    args = parser.parse_args()
    errors, summary = audit(args.require_commercial)
    print(json.dumps(summary, indent=2, sort_keys=True))
    if errors:
        for error in errors:
            print("FAIL", error, file=sys.stderr)
        return 1
    print("PASS inventory and original research provenance audit ONLY")
    if not args.require_commercial:
        print("NOT A COMMERCIAL ENGINE CERTIFICATE: see RED Milestone One gates")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
