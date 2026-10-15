# HUD V2 implementation evidence — 15 October 2026

Scope: animated top-left H1 card and unboxed bottom M1 rail from the owner's approved visual contract. The rest of the living-world rebuild remains outstanding. No commercial gate promoted; no APK produced.

## Changes

- Card: clipped layered pixel sky (drifting cloud layers, treeline, sun, rain, mist, golden-hour palette), original ball/club art, raised wooden tee, native rough/deep-rough grass occlusion and shallow sand burial. Ball proportions remain unchanged. Card taps cycle clubs; no popup. Wind uses native direction/speed.
- Max: cached separate probes use the recovered ClassicShotModel and original distance conversion at full power, on a calm neutral flat surface (putter on flat green). Driver gives 240. This is a nominal club range, not a wind/terrain-adjusted landing prediction; the live game and its PRNG are not mutated.
- Rail: no enclosing panel; rounded blue neon body, native yellow/red regions at the right, ready pulse/sparks, charge ribbons, luminous pointer/trail, separate captured-power marker, accuracy-window light, leaves, rain/splashes, mist and golden accents. Card and rail consume the same live weather and presentation clock. Pause freezes them; reduced motion removes travelling effects.
- Mobile meter: position increases toward visual left during charging, reverses without a jump on capture. Native steps are 6/7 units per fixed tick, removing the old tenth-tick skip: approximately 2.50 seconds to full power and 2.14 seconds back to centre. Left yellow maps to hook, right to slice; red submits recovered accuracy 63 without reducing captured power. Actual native lie bounds shrink difficulty windows.
- Session timing is versioned: native session format 2 and course compatibility identity prevent old command journals from being reinterpreted at the new speed. Old interrupted rounds require a fresh round; saved player settings remain intact.
- Fixed existing garbled status/pause text encountered in the integrated render.

## Verification observed locally

- Watched the new meter-side GDScript assertion fail against the old UI; watched the new cadence assertion fail against the old C++ meter.
- Windows native extension rebuilt successfully with SCons.
- CTest: 29/29 passed, including expanded native integration tests.
- 200 club/lie/power launch combinations matched recovered force/power formulas and hook/slice direction. Four representative clubs × five displayed powers matched direct recovered input for 256 ticks each (20 complete adapter comparisons).
- HUDLivingCI, WhackMeterCI, MobileHUDCI, NativeWindCI, VisualPolishCI, CommercialJourneyCI, ShotTransitionCI and LakewoodSceneCI passed with no script errors. Lakewood native scene completed in three strokes; pause/replay checks passed.
- Actual OpenGL rendered HUDLivingVisualCI: 72 frames, tee/rough/sand/fairway and four weather conditions, plus eight-state gallery. Animation frames differ as expected. Images inspected.
- Actual OpenGL HUDLivingGameVisualCI passed: native Driver Max 240, tee lie, equal card/rail clocks, frozen pause clock, portrait/wide layout separation. Integrated screenshots inspected.
- Independent scoped source review: no important defects reported; review did not claim device or visual acceptance.

Sandbox tests use child-process APPDATA under work/test-temp/hud-appdata and absolute log paths. Default user storage is restricted. Successful final visual logs have no shader/runtime errors. The rendered GIF is sampled at 12fps, not the game's simulation/render cadence.

## Evidence paths from task root

- outputs/Pixtee-HUD-V2/Preview.html
- outputs/Pixtee-HUD-V2/Animated-HUD.gif
- outputs/Pixtee-HUD-V2/eight-states.png
- outputs/Pixtee-HUD-V2/live-course.png and live-wide.png
- work/test-temp/*-final.log and *-hud-v2.log
- work/native-build/Testing/Temporary/LastTest.log
- work/hud-v2-before/SHA256.json (original source restoration copy)

## Still awaiting

Owner visual/feel acceptance of these controls; actual Android/Fold touch and performance testing. Whole-course art, character/world polish and commercial release acceptance remain separate open work. No claim that this increment finishes the game.
