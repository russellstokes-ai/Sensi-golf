#!/usr/bin/env python3
"""Validate real transparent Pixtee golfer frames before any Android preview.

All eight full-swing frames plus putter must be identical 128x64 RGBA PNGs.
A common feet pivot and contact point protect the exact game-world geometry.
This checks exported pixels, not aesthetic quality, swing continuity, or
copyright/owner approval. Those still require human review and sign-off.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import struct
import zlib

GOLFER_IDS = (
    "golfer_idle", "golfer_takeaway", "golfer_backswing", "golfer_top",
    "golfer_downswing", "golfer_impact", "golfer_follow",
    "golfer_finish", "golfer_putt",
)
WIDTH, HEIGHT = 128, 64
FEET = (40, 60)
CONTACT = (92, 60)
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def decode_alpha(p: Path) -> list[bytes]:
    data = p.read_bytes()
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("not a PNG")
    pos = 8
    image_data = bytearray()
    dimensions = None
    while pos + 12 <= len(data):
        length = struct.unpack_from(">I", data, pos)[0]
        label = data[pos + 4:pos + 8]
        end = pos + 12 + length
        if end > len(data):
            raise ValueError("truncated PNG")
        payload = data[pos + 8:pos + 8 + length]
        if label == b"IHDR":
            width, height, depth, mode, compression, filter_kind, interlace = (
                struct.unpack(">IIBBBBB", payload))
            if ((width, height) != (WIDTH, HEIGHT) or depth != 8 or
                mode != 6 or compression or filter_kind or interlace):
                raise ValueError("golfer frames must be 128x64 RGBA8 noninterlaced PNGs")
            dimensions = True
        elif label == b"IDAT":
            image_data.extend(payload)
        elif label == b"IEND":
            break
        pos = end
    if not dimensions or not image_data:
        raise ValueError("missing PNG image data")
    payload = zlib.decompress(image_data)
    row_size = WIDTH * 4
    offset = 0
    previous = bytearray(row_size)
    alpha = []
    for _ in range(HEIGHT):
        if offset + row_size + 1 > len(payload):
            raise ValueError("truncated decoded PNG pixels")
        mode = payload[offset]
        offset += 1
        raw = payload[offset:offset + row_size]
        offset += row_size
        if mode not in (0, 1, 2, 3, 4):
            raise ValueError("invalid PNG filter")
        row = bytearray(row_size)
        for x, value in enumerate(raw):
            a = row[x-4] if x >= 4 else 0
            b = previous[x]
            c = previous[x-4] if x >= 4 else 0
            if mode == 0: predictor = 0
            elif mode == 1: predictor = a
            elif mode == 2: predictor = b
            elif mode == 3: predictor = (a+b)//2
            else:
                p0 = a+b-c
                da, db, dc = abs(p0-a), abs(p0-b), abs(p0-c)
                predictor = a if da<=db and da<=dc else b if db<=dc else c
            row[x] = (value+predictor)&255
        alpha.append(bytes(row[3::4]))
        previous = row
    return alpha


def has_solid_near(rows: list[bytes], x: int, y: int, radius: int) -> bool:
    return any(rows[yy][xx]>=128
               for yy in range(max(0,y-radius),min(HEIGHT,y+radius+1))
               for xx in range(max(0,x-radius),min(WIDTH,x+radius+1)))


def validate_folder(folder: Path) -> list[str]:
    errors = []
    for id in GOLFER_IDS:
        p = folder / (id + ".png")
        if not p.exists():
            continue   # Release gate independently requires all nine
        try:
            rows = decode_alpha(p)
            if not has_solid_near(rows,*FEET,4):
                errors.append(f"{id}: registered feet pivot has no opaque pixels")
            if id == "golfer_impact" and not has_solid_near(rows,*CONTACT,3):
                errors.append("golfer_impact: clubface does not meet physical ball contact anchor")
        except (ValueError, OSError, struct.error, zlib.error) as exc:
            errors.append(f"{id}: {exc}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path,
                        default=Path(__file__).resolve().parents[1])
    args=parser.parse_args()
    sources=(
        args.root/"pixtee-android/app/src/debug/assets/art/review",
        args.root/"pixtee-android/app/src/main/assets/art/production",
    )
    errs=[]
    for source in sources:
        errs.extend(validate_folder(source))
    if errs:
        for e in errs: print("ART FRAME ERROR:", e)
        return 1
    print("Pixtee golfer registration check passed (does not approve artwork).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
