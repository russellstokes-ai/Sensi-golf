# Pixtee Golf — Godot 4.5 + recovered Sensible Golf C++ core

> **Milestone One authoritative handover (12 October):**
> [Commercial-quality three-hole native engine data and acceptance plan](../spec/pixtee/MILESTONE_ONE_COMMERCIAL_ENGINE_HANDOVER_2026-10-12.md),
> [complete 27-area source inventory](../spec/pixtee/MILESTONE_ONE_ENGINE_DATA_INVENTORY_2026-10-12.json),
> [World FX/shader/particle standard](../spec/pixtee/PIXTEE_MILESTONE_ONE_WORLD_FX_AND_FEEL_STANDARD_2026-10-12.md),\n> and [automated audit](../tools/verify_pixtee_milestone_one_inventory.py).
> The original physics research bank is retained; 3-hole gameplay, commercial
> rights, Android production and all 14 release gates are **NOT YET COMPLETE**.
> Passing original x86 comparisons and headless bindings do not mean ready-to-sell.

**Status: native-engine integration scaffold, NOT a finished hole or Android APK.**

The project invokes the existing recovered C++ gameplay implementation via a
Godot 4 GDExtension. Godot owns presentation, camera, HUD, animation, and device
input; **the recovered C++ engine remains authoritative for ball physics and
gameplay state**. There is no Kotlin, Canvas, or third independently invented
physics implementation in this project.

## Original executable verified milestones (11 October 2026)

- 650/650 club × lie × power/accuracy original machine-code launch vectors
  match native recovered C++ **exactly**.
- 1,986/1,986 original Welly counter and capture callback states match
  `recovered_welly_tick.cpp` **exactly**; native portable test is in CI.
  Actual game-loop input timing / colour-zone pixels remain OPEN.
- 256/256 original Windows putting vectors (16 arrow directions × 16 slope
  magnitudes, controlled putt) match native C++ final X/Y/ticks **exactly**.
- Unbounded extreme-accuracy original launcher experiment: 668/668 original
  out-of-profile samples **did not launch**, 322/322 supported samples did.
  Whole user-input/penalty behaviour remains OPEN.
- Original visual measurement: 874 MCH frames, 19 LBM resources,
  72 course/green geometry sets and original sound/media name hashes.
- Original AI and tourney source VAs identified, but not reconstructed.
- Actual Godot 4.5 Linux engine-only GDExtension class load and C++
  original-rule shot/round smoke **PASS** in GitHub Actions; Android not tested.
- Remaining original-game parity requirements and original-code rights review:
  [ORIGINAL_23_RECOVERY_REPORT_2026-10-11.md](../spec/pixtee/ORIGINAL_23_RECOVERY_REPORT_2026-10-11.md).

**No production sprites, menus, playable Android APK, or commercial licence
are delivered by this engine-only research milestone.**

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

## Engine-only progress (recovered original mechanics)

- `PixteeGameplay`: one recovered classic hole, original raw aim, 13 clubs,
  three-click input capture, complete C++ position/velocity/terrain/slope state.
- `PixteeRoundBridge`: Godot API for the existing recovered
  `ClassicGameSession`, 18 original-rule hole requests, scoring counters,
  course resource injection (MAPM/SPT/MAPI/MAPS byte buffers), original
  hazard and special-green acknowledgements, checked stable-shot replay.
- `PixteeRoundRuntime`: host-independent C++ round adapter validated with
  an original-rule 18-hole round and save corruption test.
- `pixtee_course_adapter.cpp`: independent Pixtee Lakewood geometry is
  encoded using the real original 8×4 MAPI fine subcells, three green
  transition markers and a separate MAPS green; all 16x16 original slope
  nibble combinations are tested. A flat authored green is the default
  until approved slope geometry is provided—no invented hill values.
- `reference/`: authenticated Sensible Golf research tables and numeric
  18-hole original gameplay evidence, integrity checked in CI.
- `spec/pixtee/ENGINE_COMPLETENESS_GATE_2026-10-11.json`: exhaustive known
  original-mechanics inventory and strict no-100%-parity release gate.
- `scripts/EngineBindingCI.gd`: headless Godot engine-only binding test,
  independent of approved art.

No original commercial course map or binary is part of the playable asset
pipeline. Recovered numerical evidence/engine redistribution must undergo
rights clearance before shipping.

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
6. The Lakewood collision grid now uses recovered-format original 8×4 fine
   subcells and a dedicated green map, but final Pixtee course geometry,
   authored slope fields and original parity for all terminal cases remain gated.
7. No GUI feature or ambient animation may update the C++ ball state.
8. No fidelity declaration without original-trace comparison and tests on
   Android portrait and Fold closed/open.
