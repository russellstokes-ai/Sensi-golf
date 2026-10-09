#!/usr/bin/env python3
"""Detect original Sensible Golf DOS main-menu graphics in fixed emulator frames.

Structural brown-button/green-turf signature only; no OCR or display-time
assumption. CI-only gate for the 2400x1080 emulator, not a general vision model.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image


def fractions(path: Path, normalize_dim_fade: bool = False) -> tuple[float, float]:
    with Image.open(path) as source:
        im = source.convert("RGB")
        w, h = im.size
        if w < 400 or h < 250:
            raise ValueError("unexpectedly small screenshot")
        left, right = round(w * 0.38), round(w * 0.688)
        top, bottom = round(h * 0.324), round(h * 0.806)
        step = max(2, w // 480)
        pixels = im.load()
        sample = [pixels[x, y] for y in range(top, bottom, step)
                  for x in range(left, right, step)]
        if not sample:
            raise ValueError("empty screenshot sampling region")

        gain = 1.0
        if normalize_dim_fade:
            intensities = sorted(max(rgb) for rgb in sample)
            p95 = intensities[int(len(intensities) * 0.95)]
            # The actual original menu was photographed in a dark fade at
            # probe-30 of run 37996364852 (p95 ~= 18). Normalising its
            # colour brightness reveals the correct brown-button signature;
            # demo course footage remains overwhelmingly green.
            if not 7 <= p95 < 60:
                return 0.0, 1.0
            gain = min(12.0, 160.0 / p95)

        brown = green = 0
        for r, g, b in sample:
            if gain != 1.0:
                r = min(255, r * gain)
                g = min(255, g * gain)
                b = min(255, b * gain)
            brown += bool(r > 40 and r > g * 1.4 and r > b * 1.75 and 4 < g < 130)
            green += bool(g > r * 1.35 and g > b * 1.24 and g > 65)
        n = len(sample)
        return brown / n, green / n


def is_main_menu(path: Path) -> bool:
    brown, green = fractions(path)
    if brown >= 0.18 and green <= 0.15:
        return True
    # Do not confuse a fade-in with a failed menu launch. This branch is
    # brightness-normalised only for genuinely dark frames; it doesn't
    # turn normal brightly-coloured demo footage into a false menu match.
    brown, green = fractions(path, normalize_dim_fade=True)
    return brown >= 0.18 and green <= 0.15


def main() -> int:
    parser = argparse.ArgumentParser(description="Original main-menu screenshot signature")
    parser.add_argument("image", type=Path)
    args = parser.parse_args()
    brown, green = fractions(args.image)
    ok = is_main_menu(args.image)
    print(f"original-menu-signature={ok} brown={brown:.3f} green={green:.3f} file={args.image.name}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
