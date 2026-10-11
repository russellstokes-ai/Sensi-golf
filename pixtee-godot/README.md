# Pixtee Golf — Godot 4.5 + recovered Sensible Golf C++ core

**Status: native-engine integration scaffold, NOT a finished hole or Android APK.**

The project invokes the existing recovered C++ gameplay implementation via a
Godot 4 GDExtension. Godot owns presentation, camera, HUD, animation, and device
input; **the recovered C++ engine remains authoritative for ball physics and
gameplay state**. There is no Kotlin, Canvas, or third independently invented
physics implementation in this project.

## Source provenance

- C++ native authority: `../engine/include/sensigolf`, `../engine/src`.
- New independent Pixtee Lakewood terrain adapter: `native/src/pixtee_course_adapter.cpp`.
- Native host wrapper: `native/src/pixtee_gameplay.cpp`.
- Godot-only integration diagnostic: `scenes/CoreSmoke.tscn`.
- Mandatory feature/parity register:
  `../spec/pixtee/GODOT_REUSE_AND_PARITY_MASTER_2026-10-11.md`.
- User-approved Pixtee sprites are **not** replaced by primitive programmer art.
  Original owner-approved ZIPs remain outside this scaffold until imported and
  checksum-verified into the approved asset pipeline.

## Build prerequisites

Godot **4.5**, matching `godot-cpp` **4.5 branch**, C++17 compiler,
Python/SCons, and Android NDK for Android. Use the official Godot 4.5
GDExtension build guide. On a computer with internet access:

```sh
cd pixtee-godot/native
git clone --branch 4.5 --depth 1 https://github.com/godotengine/godot-cpp.git
python -m pip install scons
scons platform=linux target=template_debug
# Windows: scons platform=windows target=template_debug
# Android arm64: scons platform=android arch=arm64 target=template_debug
```

The generated native library must match the `bin/pixtee.gdextension`
filenames/architecture and be placed under `pixtee-godot/bin/`. **Do not
claim the GDExtension is verified until this build actually succeeds.**

Then open `pixtee-godot/project.godot` in Godot 4.5 and run CoreSmoke. It
displays native state and can inject a documented *test-vector* shot. This
screen is intentionally not presented as production Pixtee artwork.

## Native smoke without Godot

Run the workflow `pixtee-godot-native-core-smoke.yml`: it executes all
existing native C++ engine tests and compiles the new Pixtee course adapter
against the recovered core. This test does not validate the GDExtension,
device rendering, or commercial-use readiness.

## Safety / parity contracts

1. Keep original Sensi restoration on `main` unchanged.
2. Keep prior Pixtee Kotlin branch unchanged; this branch is experimental.
3. **Do not** copy original EXE, EPF, course layouts, sprites, music or sound
   into the Pixtee commercial product without rights.
4. Keep the original recovered 936/65536-second fixed tick, 4096 heading
   units, 13 original clubs, raw 0..105 power and sweet spot 63.
5. Three-click power and accuracy sampling must come from a **verified meter
   scheduler**; current CoreSmoke's values are test vectors only.
6. The Lakewood collision grid is an independently constructed first pass
   and is too coarse for final player-facing terrain, slopes and cup capture.
7. No GUI feature or ambient animation may update the C++ ball state.
8. No fidelity declaration without original-trace comparison and tests on
   Android portrait and Fold closed/open.
