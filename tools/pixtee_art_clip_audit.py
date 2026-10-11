#!/usr/bin/env python3
"""Pixtee art QA: fail on clipped RGBA artwork, wrong frame sizes or missing assets.
Never promotes review assets to production or changes approval status.
"""
import argparse
import json
from pathlib import Path
from PIL import Image

GOLFER_ACTION_FRAMES = {
    "address": 4, "swing": 24, "putt": 12, "walk": 8,
    "idle": 4, "watch_shot": 6, "celebrate": 8, "disappointed": 6,
}
DIRECTIONS = ("n", "ne", "e", "se", "s", "sw", "w", "nw")

def inspect_png(path, size=None, transparent_margin=2):
    problems = []
    try:
        with Image.open(path) as source:
            im = source.convert("RGBA")
    except Exception as exc:
        return [f"invalid PNG: {exc}"]
    if size and im.size != size:
        problems.append(f"size {im.size} != {size}")
    alpha = im.getchannel("A")
    bbox = alpha.getbbox()
    if bbox:
        left, top, right, bottom = bbox
        w, h = im.size
        if min(left, top, w-right, h-bottom) < transparent_margin:
            problems.append(f"art touches padding: bbox={bbox}, canvas={im.size}")
    else:
        problems.append("fully transparent image")
    return problems

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("root", type=Path, help="Repository root")
    ap.add_argument("--require-all-golfer-frames", action="store_true")
    ap.add_argument("--json", type=Path)
    args = ap.parse_args()
    root = args.root
    source = root / "pixtee-android/app/src/main/assets/art/review"
    result = {"status": "unapproved", "checked": 0, "missing": [], "problems": {}}
    for png in sorted(source.glob("*.png")):
        result["checked"] += 1
        issues = inspect_png(png)
        if issues:
            result["problems"][png.name] = issues
    if args.require_all_golfer_frames:
        for action, count in GOLFER_ACTION_FRAMES.items():
            for direction in DIRECTIONS:
                for index in range(count):
                    name = f"golfer_{action}_{direction}_f{index:02d}.png"
                    path = source / name
                    if not path.is_file():
                        result["missing"].append(name)
                    else:
                        issues = inspect_png(path, (256, 128))
                        if issues:
                            result["problems"][name] = issues
    result["passed"] = not result["missing"] and not result["problems"]
    print(json.dumps(result, indent=2))
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    raise SystemExit(0 if result["passed"] else 1)

if __name__ == "__main__":
    main()
