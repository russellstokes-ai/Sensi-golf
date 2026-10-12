#!/usr/bin/env python3
"""Hard fail-closed Pixtee commercial release certificate verifier.

Default mode confirms all 31 required delivery gates are present and that
currently OPEN work is not misrepresented as a finished commercial product.
--require-release requires fully proved/signed, source-verified deliverables.
A PASS in default mode is documentation completeness only, NOT release QA.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PATH = ROOT / "spec/pixtee/PIXTEE_COMMERCIAL_RELEASE_MASTER_GATE_2026-10-12.json"
EXISTING = ROOT / "spec/pixtee/MILESTONE_ONE_ENGINE_DATA_INVENTORY_2026-10-12.json"
EXPECTED = {
    "GAME": 9, "VIS": 7, "SOUND": 4, "UI": 4,
    "CONTENT": 2, "ANDROID": 3, "LEGAL": 1, "RELEASE": 1,
}
TOTAL = sum(EXPECTED.values())
VALID_STATES = {"OPEN", "PASS"}


def validate(require_release: bool) -> tuple[list[str], dict]:
    errors: list[str] = []
    if not PATH.is_file() or not EXISTING.is_file():
        return ["Missing master release document or original native baseline"], {}
    try:
        doc = json.loads(PATH.read_text(encoding="utf-8"))
        old = json.loads(EXISTING.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        return [f"Unreadable source: {exc}"], {}
    if doc.get("schema") != "pixtee.commercial-release-gates.v1":
        errors.append("Unknown product release schema")
    if doc.get("gameplay_authority") != ("Recovered Sensible Golf C++ only; fixed 936/65536 tick; "
                                       "Godot cosmetic FX cannot affect ball state or gameplay RNG."):
        errors.append("Recovered native C++ gameplay contract was changed")
    refs = doc.get("authority", {})
    if "pixtee-lakewood-open.zip SHA256 82c0df6831241e31ad0c83a4302df145349afb9b63503487a914b964d05c7ff3" != refs.get("hole1"):
        errors.append("Frozen source course hash missing/changed")
    if "2043e9c678b9904311778d17a1f341d577344e6d49a3a9d48ce6c3aef3b5239e" not in refs.get("art", ""):
        errors.append("Owner-approved art SHA missing/changed")
    if "1544ac4782b64d2a2b968a581f31b9cf7b37b6e0cf4583dd665e21d1d81ffe17" not in refs.get("standard", ""):
        errors.append("Separate Work production-standard v1.1 hash missing/changed")
    requirements = doc.get("requirements", [])
    ids = [x.get("id") for x in requirements]
    if len(requirements) != TOTAL or len(ids) != len(set(ids)):
        errors.append(f"Commercial release requires {TOTAL} distinct tracked gates")
    from collections import Counter
    counts = Counter(str(x).split("-", 1)[0] for x in ids)
    if dict(counts) != EXPECTED:
        errors.append(f"Commercial coverage changed {dict(counts)} != {EXPECTED}")
    for category, count in EXPECTED.items():
        for number in range(1, count + 1):
            if f"{category}-{number:02d}" not in ids:
                errors.append(f"Mandatory release area missing {category}-{number:02d}")
    for gate in requirements:
        name, state = gate.get("id", "UNKNOWN"), gate.get("status")
        if state not in VALID_STATES:
            errors.append(f"{name}: status must be OPEN or PASS")
        if not gate.get("title") or not gate.get("requirements") or not gate.get("proof_required"):
            errors.append(f"{name}: missing requirement or acceptance proof description")
        if state == "PASS":
            if not isinstance(gate.get("reviewer"), str) or not gate["reviewer"].strip():
                errors.append(f"{name}: PASS without named reviewer/signoff")
            proof = gate.get("evidence_links", [])
            if not proof or not all(isinstance(link, str) and
                                    (link.startswith("https://") or
                                     (ROOT / link).is_file() and
                                     (ROOT / link).resolve().is_relative_to(ROOT.resolve()))
                                    for link in proof):
                errors.append(f"{name}: PASS without file/https evidence")
    approved = len(requirements) == TOTAL and all(g.get("status") == "PASS" for g in requirements)
    old_m1 = old.get("milestone_one_acceptance", [])
    old_accepted = (len(old_m1) == 14 and
                    all(x.get("status") == "PASS" for x in old_m1) and
                    old.get("commercial_engine_ready") is True and
                    old.get("rights_clearance") == "APPROVED" and
                    old.get("playable_course_status", {}).get("verified_production_courses") == 3)
    ready = (approved and old_accepted and doc.get("godot_playable_three_holes") is True
             and doc.get("copyright_clearance") is True)
    if doc.get("commercial_release_ready") is not ready:
        errors.append("COMMERCIAL READY CLAIM CONTRADICTS GATES/ORIGINAL CORE/Licences")
    if require_release and not ready:
        errors.append(
            "COMMERCIAL SHIP BLOCKED: native TREE+OOB+GREEN, full HOLE1-3 gameplay, "
            "music/SFX, particles/shaders, UI/intro, real Android device QA, "
            "independent evidence and RIGHTS must ALL pass"
        )
    return errors, {
        "tracked_release_gates": len(requirements), "required_release_gates": TOTAL,
        "gates_passed": sum(x.get("status") == "PASS" for x in requirements),
        "core_release_gates_passed": sum(x.get("status") == "PASS" for x in old_m1),
        "audio_gates": counts.get("SOUND", 0),
        "effects_green_arrow_gates": counts.get("VIS", 0),
        "commercial_release_ready": bool(doc.get("commercial_release_ready")),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--require-release", action="store_true")
    args = parser.parse_args()
    errors, summary = validate(args.require_release)
    print(json.dumps(summary, indent=2, sort_keys=True))
    for error in errors:
        print("FAIL", error, file=sys.stderr)
    if errors:
        return 1
    print("PASS: release checklist is complete as a tracker")
    print("NOT AN APK OR COMMERCIAL CERTIFICATE" if not args.require_release else "COMMERCIAL EVIDENCE GATE GREEN")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
