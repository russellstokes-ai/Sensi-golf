#!/usr/bin/env python3
"""Bridge Android screen touches to libretro mouse-left for original DOS games.

The pinned LibretroDroid GLRetroView only exposes touchscreen presses via
RETRO_DEVICE_POINTER. DOSBox Pure 'direct' mouse mode derives cursor position
from that pointer but consumes RETRO_DEVICE_MOUSE_LEFT for left clicks.
Unmodified upstream LibretroDroid does not implement RETRO_DEVICE_MOUSE input,
so absolute positioning alone NEVER selects a game menu item.

This narrowly scoped and idempotent GPL-source patch is applied to the pinned
local LibretroDroid checkout before NDK compilation. No game rules are changed.
"""
from __future__ import annotations

import argparse
from pathlib import Path

ANCHOR = "        case RETRO_DEVICE_POINTER: {"
MARKER = "        case RETRO_DEVICE_MOUSE: {"
BRIDGE = """        case RETRO_DEVICE_MOUSE: {
            // Original-game direct pointer: GLRetroView sends the touchscreen
            // normalized position and a negative out-of-screen value on UP.
            // Only the first finger's primary click is mapped; this leaves
            // all gamepad and keyboard bindings unchanged.
            if (port != 0 || id != RETRO_DEVICE_ID_MOUSE_LEFT) return 0;
            // Fix sporadic wrong-mode clicks on authentic menu. The original
            // cursor may still point at Play Season during the same libretro
            // frame that receives a finger DOWN on Play Round. Let the core
            // process the new absolute pointer position first, then deliver
            // the mouse-down on a subsequent frame, without game patching.
            const bool pressed=(pads[port].pointerScreenXAxis >= 0 &&
                                pads[port].pointerScreenYAxis >= 0);
            static bool lastPressed=false;
            static std::chrono::steady_clock::time_point firstPressAt{};
            if (!pressed) { lastPressed=false; return 0; }
            if (!lastPressed) {
                lastPressed=true;
                firstPressAt=std::chrono::steady_clock::now();
                return 0;
            }
            return (int16_t) ((std::chrono::steady_clock::now()-firstPressAt)
                              >= std::chrono::milliseconds(35));
        }

"""


def patch(content: str) -> tuple[str, bool]:
    if MARKER in content:
        if BRIDGE not in content:
            raise ValueError("Existing RETRO_DEVICE_MOUSE handler differs; inspect upstream")
        return content, False
    if content.count(ANCHOR) != 1:
        raise ValueError("Pinned input.cpp pointer handler not found exactly once")
    if "RETRO_DEVICE_ID_POINTER_PRESSED" not in content:
        raise ValueError("Pinned source no longer exposes the expected pointer")
    patched=content.replace(ANCHOR, BRIDGE + ANCHOR, 1)
    if "#include <cmath>" in patched and "#include <chrono>" not in patched:
        patched=patched.replace("#include <cmath>", "#include <cmath>\n#include <chrono>", 1)
    return patched, True


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", nargs="?", type=Path, default=Path(
        "android-full/vendor/LibretroDroid/libretrodroid/src/main/cpp/input.cpp"))
    args = parser.parse_args()
    original = args.source.read_text()
    result, changed = patch(original)
    if changed:
        args.source.write_text(result)
    print(f"LibretroDroid DOS touch/mouse-left bridge: {'patched' if changed else 'already patched'}")


if __name__ == "__main__":
    main()
