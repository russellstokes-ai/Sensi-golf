#!/usr/bin/env python3
"""Pixtee visual-asset intake gate.

An owner-approved, content-hashed entry is required for EVERY file in the
production art folder. No default sprite packs, downloaded artwork, temporary
Canvas glyphs or self-certified approvals. This checks the evidence register;
actual owner approval must be independently obtained in conversation/review.
Use --release to additionally require all visual categories to be signed off.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

CATEGORY_IDS = {
    "golfer_and_ball", "terrain_and_scenery", "course_compositions",
    "menus_and_hud", "animations_and_effects", "branding_and_sponsorship",
}
REQUIRED_FRAMES = {
    "golfer_idle.png", "golfer_takeaway.png", "golfer_backswing.png",
    "golfer_top.png", "golfer_downswing.png", "golfer_impact.png",
    "golfer_follow.png", "golfer_finish.png", "golfer_putt.png",
    "tree_round.png", "tree_pine.png",
    "spectator_idle.png", "spectator_wave.png",
    "spectator_photographer.png", "camera_flash.png",
    "bird_wings_up.png", "bird_wings_down.png",
    "flower_yellow.png", "flower_pink.png",
    "terrain_rough.png", "terrain_fairway.png", "terrain_green.png",
    "terrain_sand.png", "terrain_water.png", "ui_pixtee_logo.png",
    "ui_wood_button.png", "ui_hud_panel.png", "ui_sponsor_board.png",
}
ASSET_PATH = Path("pixtee-android/app/src/main/assets/art/production")
LEDGER_PATH = Path("spec/pixtee/ART_REVIEW_REGISTER.json")
ALLOWED_SUFFIXES = {".png", ".webp", ".jpg", ".jpeg", ".svg"}
IGNORED_FILES = {"APPROVED_v1.txt"}
SHA_PATTERN = re.compile(r"[a-f0-9]{64}")
BAD_REFERENCE = {"", "pending", "todo", "tbd", "test", "placeholder", "unapproved"}


def validate(root: Path, release: bool = False) -> list[str]:
    errors = []
    ledger_file = root / LEDGER_PATH
    try:
        data = json.loads(ledger_file.read_text(encoding="utf-8"))
    except (OSError, ValueError) as exc:
        return [f"Art approval ledger not readable: {exc}"]
    if data.get("schema") != "pixtee.art-review.v1":
        errors.append("Art ledger schema is incorrect")
    if data.get("ownerApprovalRequired") is not True:
        errors.append("Owner approval must remain mandatory")
    categories = data.get("categories", [])
    if not isinstance(categories, list):
        return ["Art categories must be a list"]
    category_map = {row.get("id"): row.get("status")
                    for row in categories if isinstance(row, dict)}
    if set(category_map) != CATEGORY_IDS or len(categories) != len(CATEGORY_IDS):
        errors.append("All six visual categories must be registered exactly once")
    if release and any(category_map.get(k) != "approved" for k in CATEGORY_IDS):
        errors.append("Release blocked: not all visual categories have owner approval")
    assets = data.get("approvedAssets", [])
    if not isinstance(assets, list):
        return ["Approved assets must be a list"]
    art_root = root / ASSET_PATH
    found = {p.name: p for p in art_root.rglob("*") if p.is_file()} if art_root.exists() else {}
    # No nested folder is needed for the contract's single flat production pack.
    names = []
    for row in assets:
        if not isinstance(row, dict):
            errors.append("Malformed approved asset entry")
            continue
        filename = row.get("file", "")
        sha = row.get("sha256", "")
        approval = row.get("ownerApprovalReference", "")
        revision = row.get("revision", "")
        category = row.get("category")
        if not isinstance(filename, str) or not re.fullmatch(r"[a-z0-9][a-z0-9_-]*\.(?:png|webp|jpe?g|svg)", filename):
            errors.append(f"Unsafe artwork filename: {filename!r}")
            continue
        names.append(filename)
        if category not in CATEGORY_IDS:
            errors.append(f"Unknown category for {filename}")
        if not isinstance(revision, str) or not revision.strip():
            errors.append(f"Missing immutable revision for {filename}")
        if not isinstance(approval, str) or approval.strip().lower() in BAD_REFERENCE:
            errors.append(f"Missing explicit owner-approval reference for {filename}")
        if not isinstance(sha, str) or not SHA_PATTERN.fullmatch(sha):
            errors.append(f"Missing valid SHA-256 for {filename}")
        p = art_root / filename
        if not p.is_file():
            errors.append(f"Approved artwork missing: {filename}")
        elif SHA_PATTERN.fullmatch(sha or ""):
            actual = hashlib.sha256(p.read_bytes()).hexdigest()
            if actual != sha:
                errors.append(f"Production artwork changed without reapproval: {filename}")
    if len(names) != len(set(names)):
        errors.append("Duplicate artwork approval entries")
    for filename, path in found.items():
        if filename in IGNORED_FILES:
            continue
        if path.parent != art_root or filename not in names:
            errors.append(f"Unapproved artwork exists in production pack: {path.relative_to(root)}")
        elif path.suffix.lower() not in ALLOWED_SUFFIXES:
            errors.append(f"Unsupported production artwork type: {filename}")
    if release:
        if not REQUIRED_FRAMES.issubset(set(names)):
            errors.append("Release blocked: required golfer/scenery frames have not been individually approved")
        marker = art_root / "APPROVED_v1.txt"
        if not marker.is_file() or marker.read_text(encoding="utf-8").strip() != "PIXTEE_PRODUCTION_ART_APPROVED_V1":
            errors.append("Release blocked: production art marker absent or incorrect")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--release", action="store_true")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    errors = validate(args.root, args.release)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1
    print("Pixtee visual asset check passed. No art has been auto-approved.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
