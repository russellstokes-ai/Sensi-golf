#!/usr/bin/env python3
"""Detect original DOS Player Select game-mode page by its stable layout colours.

Evidence: original Android runtime run 38002246293 reached the genuine
Player Select menu, not Demo Mode, before the main-menu-only gate timed out.
Detection uses geometry/color fractions; NEVER asserts that golf is playable.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image


def signature(path: Path) -> tuple[float, float, float]:
    with Image.open(path) as src:
        img = src.convert("RGB")
        w, h = img.size
        if w < 400 or h < 250:
            raise ValueError("Screenshot unexpectedly small")
        sample = [img.getpixel((x,y)) for y in range(round(h*.18),round(h*.85),8)
                  for x in range(round(w*.22),round(w*.81),8)]
    n = len(sample)
    blue = sum(b > 40 and b > r*1.7 and b > g*1.4 for r,g,b in sample)/n
    brown = sum(r > 40 and r > g*1.4 and r > b*1.75 for r,g,b in sample)/n
    green = sum(g > 65 and g > r*1.35 and g > b*1.24 for r,g,b in sample)/n
    return blue,brown,green


def is_player_select(path: Path) -> bool:
    blue,brown,green = signature(path)
    # Two original variants confirmed in real Android screenshots:
    # single-player Select: blue=.727 brown=.236 (run 38002246293)
    # four-player Select:  blue=.554 brown=.393 (run 38038070178).
    # Early blue intro has brown=.038; main menu has blue=.005.
    return blue >= 0.50 and brown >= 0.15 and green < 0.08


def main() -> int:
    parser=argparse.ArgumentParser()
    parser.add_argument("image",type=Path)
    args=parser.parse_args()
    b,r,g=signature(args.image)
    ok=is_player_select(args.image)
    print(f"original-player-select={ok} blue={b:.3f} brown={r:.3f} green={g:.3f} file={args.image.name}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
