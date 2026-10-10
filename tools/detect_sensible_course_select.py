#!/usr/bin/env python3
"""Evidence-based state detector for ORIGINAL 'Select Golf Course' menu.

Never classify a blue background and brown buttons alone: those colours
also occur in Player Select and falsely passed earlier CI gates. The
original course list is NARROW, centered, and leaves the LEFT of the
screen blue, unlike Player Select's full-width player rows.

Calibrated against actual 2400x1080 Android screenshots in GitHub run
38038987483; deliberately only an emulator-test detector.
"""
from pathlib import Path
import argparse
from PIL import Image


def signature(path: Path) -> tuple[float,float,float]:
    with Image.open(path) as image:
        im=image.convert("RGB")
        if im.size != (2400,1080):
            raise ValueError(f"Detector calibrated only for 2400x1080, got {im.size}")

        def brown_fraction(x0,x1,y0,y1):
            samples=(im.getpixel((x,y)) for y in range(y0,y1,10)
                     for x in range(x0,x1,10))
            flags=[r>45 and r>g*1.45 and r>b*1.75 for r,g,b in samples]
            return sum(flags)/len(flags)

        def blue_fraction(x0,x1,y0,y1):
            samples=(im.getpixel((x,y)) for y in range(y0,y1,10)
                     for x in range(x0,x1,10))
            flags=[b>40 and b>r*1.7 and b>g*1.4 for r,g,b in samples]
            return sum(flags)/len(flags)
        return (brown_fraction(750,880,350,690),
                blue_fraction(750,880,350,690),
                brown_fraction(1040,1420,280,730))


def is_course_select(path: Path) -> bool:
    left_brown,left_blue,center_brown=signature(path)
    return left_brown < .25 and left_blue > .70 and center_brown > .55


def main() -> int:
    parser=argparse.ArgumentParser()
    parser.add_argument("image",type=Path)
    args=parser.parse_args()
    fractions=signature(args.image)
    valid=is_course_select(args.image)
    print("original-course-select={} left_brown={:.3f} left_blue={:.3f} center_brown={:.3f} file={}".format(valid,*fractions,args.image.name))
    return 0 if valid else 1


if __name__=="__main__":
    raise SystemExit(main())
