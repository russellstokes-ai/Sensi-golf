#!/usr/bin/env python3
"""Detect original Sensible Golf DOS main-menu graphics in fixed emulator frames.

Structural brown-button/green-turf signature only; no OCR or display-time
assumption. CI-only gate for the 2400x1080 emulator, not a general vision model.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image


def fractions(path: Path) -> tuple[float, float]:
    with Image.open(path) as source:
        im = source.convert("RGB")
        w, h = im.size
        if w < 400 or h < 250:
            raise ValueError("unexpectedly small screenshot")
        left, right = round(w * 0.38), round(w * 0.688)
        top, bottom = round(h * 0.324), round(h * 0.806)
        step = max(2, w // 480)
        n = brown = green = 0
        pixels = im.load()
        for y in range(top, bottom, step):
            for x in range(left, right, step):
                r, g, b = pixels[x, y]
                n += 1
                brown += bool(r > 40 and r > g * 1.4 and r > b * 1.75 and 4 < g < 130)
                green += bool(g > r * 1.35 and g > b * 1.24 and g > 65)
        return brown / n, green / n


def is_main_menu(path: Path) -> bool:
    brown, green = fractions(path)
    return brown >= 0.18 and green <= 0.15


def main() -> int:
    parser = argparse.ArgumentParser(description="Original main-menu screenshot signature")
    parser.add_argument("image", type=Path)
    args = parser.parse_args()
    brown, green = fractions(args.image)
    ok = brown >= 0.18 and green <= 0.15
    print(f"original-menu-signature={ok} brown={brown:.3f} green={green:.3f} file={args.image.name}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
