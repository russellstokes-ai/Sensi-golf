# PIXTEE GODOT — SENSIBLE GOLF REUSE & PARITY MASTER
Date: 2026-10-11 | Owner direction: **Do not lose ANY useful recovered
Sensible Golf work.** Independently authored visuals/courses, but gameplay
should behave as closely as verified evidence permits.

## Project state

- [Recovered reference C++ engine](../../engine) remains the source of truth.
- [Godot project](../../pixtee-godot) is a separate, early-stage native wrapper.
- `main` original restoration stays protected; Pixtee old Kotlin remains
  available but **must not** supply fallback physics.
- This document is the **mandatory no-regression index** for every feature.
  An unverified subsystem remains RED, never replaced by unexplained "arcade"
  approximations just to make a screenshot work.
- This integration is NOT an assertion of byte-identical physics, complete
  original mechanics, correct rendering, licensability, or a playable APK.

## Inventory: recover, preserve, connect and verify

| Mechanic/system | Recovered reference / evidence | Godot target | Gate |
|---|---|---|---|
| Fixed timebase | `classic_shot_model.hpp`, timer `0x3A8` = 936/65536 s; original ~70.017Hz | C++ one tick per sample; render independently | INTEGRATED WRAPPER; runtime pending |
| 13 original club rows | `ORIGINAL_PHYSICS_RESEARCH.json`, `recovered_flight.cpp` | Keep original index 0..12; putter=12; no guessed yards/loft | PENDING INPUT / parity |
| 13×10 club/lie table | `ORIGINAL_PHYSICS_RESEARCH.json`, original binary extraction checks | Always use authoritative C++ launch/surface profiles | PRESERVED IN CORE |
| 11 accuracy/curvature profiles | `recovered_flight.cpp`, research JSON | Keep signed 12-bit heading, in-flight curves | PRESERVED IN CORE; extreme cases RED |
| WHACK-O-METER motion | `classic_mobile_controls.cpp`; classic scheduler recovered incompletely | Horizontal display only; same authoritative raw power/accuracy/tick | RED: scheduler / red-yellow boundaries |
| Three input clicks | `classic_mobile_controls.*` | Start → lock power → lock accuracy; no extra shot | NATIVE CONNECTED; GUI RED |
| Aiming | original 0..4095 heading, aim input adapter | Touch left/right of X; lock on swing | NATIVE CONNECTED; GUI RED |
| Flight gravity/drag | `recovered_flight.*`; `PHYSICS_BEHAVIOUR_BASELINES.md` | Exact core simulation; no sine trajectory | C++ AVAILABLE; parity check needed |
| Repeated bounces | `recovered_ground.*` | No single artificial bounce limit | C++ AVAILABLE |
| Lie friction and stopping | `classic_terrain.*`, `recovered_ground.*` | Surface-specific physics, no roll timeout | C++ AVAILABLE; course conversion RED |
| 77 terrain descriptors | `classic_terrain.cpp`, `recovered_course_lookup.cpp` | Authored Pixtee tile bank → SAME resolver | COARSE ADAPTER BUILT |
| MAPM/MAPI/MAPS geometry | `classic_course_resources.*` and native fixtures | Independently authored Pixtee equivalent, including greens/slope | RED: fine mask & MAPS |
| Green slope | `recovered_ground.cpp`, MAPI descriptor nibble decode | Keep original slope direction/magnitude vectors | RED: authored green map |
| Sand/bunkers | descriptor code 7 and original lie profile | Not same as water; correct spin/roll | COARSE ADAPTER BUILT; compare |
| Water/OOB | terminal code 35, original immediate stop, pause 100 ticks | Exact original hazard state/recovery and penalty | PARTIAL REFERENCE; full rules RED |
| Safe-anchor relief | `recovered_hazard_recovery.*`, session counters | Preserve original recovery choices | CORE AVAILABLE; edge cases RED |
| Putting | club 12, `recovered_putter_terminal.*` | Native low-energy roll, cup interaction | CORE AVAILABLE; fine green RED |
| Cup / hole completion | `classic_hole_session.*`, code 8, zero distance terminal | Do not invent radius-5 auto score or 12-shot cap | CORE AVAILABLE; new-hole tests RED |
| Specials 9/10 | original green/stop branch | No unsupported fabricated behavior | RED |
| Tree strikes and collisions | `recovered_interactions.*`, original renderer research | Distinguish decoration vs true original-style interactions | RED: activation/height evidence |
| PRNG | `recovered_prng.*` | Deterministic seed and reproducible state | CORE AVAILABLE |
| Replay/resume | `classic_replay_journal.*` | Stable-shot replay with crash safe checkpoints | CORE AVAILABLE; Godot binding RED |
| Hole/round progression | `classic_round_session.*`, `classic_game_session.*` | Genuine multi-hole results; modes 3/9/18 | CORE AVAILABLE; Godot binding RED |
| Original eighteen-hole traces | `analysis/evidence/gate2_full_original_eighteen_hole_checkpoint.json` | Gold reference for state/score transitions | FIXTURES EXISTS; Pixtee parity RED |
| Baseline physics traces | `engine/tests/*`, `PHYSICS_BEHAVIOUR_BASELINES.md` | Keep CI gates and add Godot-side comparison | CI ADAPTER TARGETED |
| Camera and world scale | `CAMERA_SCALE_REFERENCE.md`, `SCREEN_RESOLUTION_AND_WORLD_SCALE_2026-10-10.md` | True straight overhead, partial hole, uniform XY camera zoom | RED: measured view parity |
| Tee-to-green authored course | `PixteeCourses.kt` Lakewood design | Preserve original Pixtee authored boundaries, not Sensi bitmaps | COARSE MAP ONLY |
| Original art proportions | `CANONICAL_GAMEPLAY_MECHANICS_LOCK_2026-10-10.md` | Golfer/ball/green/flag at calibrated mobile scale | RED: approved image import |
| Golfer animation timing | `PixteeSwingRig.kt`, approved uploaded sprite sheets | Bind native shot events to approved frame/pivot IDs | RED |
| Balls and lie graphics | `art/source/ball/` research + approved art | Distinct tee/rough/sand/green/water visualization | RED |
| Flag, bunker, water effects | User art pack + original timing research | Native shot event → independent animation only | RED |
| Ambient trees, flowers, birds | Approved original Pixtee art and new bird references | Render-only state (not physics RNG) | RED |
| Crowd/spectator movement | Approved art + Pixtee ambient contract | Small idles, reactions, no physics effects | RED |
| Sponsorship | `ON_COURSE_SPONSORSHIP.md` | 4 tee/green world-space noncolliding slots | RED |
| Audio, music, sound effects | Original resource inventory (MCH/LBM + effect lookup research) | Match event timing with original Pixtee-original audio | RED; do not redistribute original payloads |
| Difficulty, computer/AI players | Original mode/gameplay research (if recovered) | Document mechanics and verified test cases before use | RED |
| Golf rules, UI, scoring, rounds | Recovered session, original manuals/source research | Integrate native state into UI; do not re-model scores | RED |
| Controller, gamepad and mobile | `classic_mobile_controls.*`, Android legacy controls | Simple touchscreen buttons, folds, orientation | RED |
| Career/trophies/stats | Current Pixtee Kotlin UI/progression reference | Port additive features, NEVER touch ball physics | RED |
| Original golf course variations | Recovered resource metadata for research | New Pixtee-authored 35×18 course data | RED; original geometry excluded |

## Verified reference source map

- Canonical [gameplay contract](CANONICAL_GAMEPLAY_MECHANICS_LOCK_2026-10-10.md)
- [Original binary hard-data master](ORIGINAL_BINARY_HARD_DATA_MASTER_2026-10-10.md)
- [Original physics reference](ORIGINAL_PHYSICS_REFERENCE.md)
- [Research table](ORIGINAL_PHYSICS_RESEARCH.json)
- [Physics behaviour baselines](PHYSICS_BEHAVIOUR_BASELINES.md)
- [Camera baseline](CAMERA_SCALE_REFERENCE.md)
- [Portrait parity contract](PORTRAIT_PARITY_CONTRACT.md)
- [Asset production Bible](ASTRA_FINAL_ART_ASSET_BIBLE_2026-10-10.md)
- [Approved visual](PIXTEE_CANONICAL_VISUAL_2026-10-10.md)
- [17+ reference golden-master workflows](../../.github/workflows)
- Source code: `engine/src/recovered_{flight,ground,putter_terminal,hazard_recovery,distance,prng,interactions,course_lookup}.cpp`, `classic_{shot_model,course_resources,terrain,hole_session,round_session,game_session,mobile_controls,replay_journal,hole_plan}.cpp`.

## Blocking acceptance gates (do not call it "ready" before all pass)

1. Prove native C++ build and full engine regression suite green on branch.
2. Prove Godot GDExtension actually builds and loads on desktop and Android.
3. Verify exported Android arm64 and x86_64 artifacts.
4. Replace coarse Lakewood adapter with Pixtee-authored fine subcell and
   dedicated green/slope maps; compare render/collision coordinates.
5. Recover the original meter scheduler; **horizontal bar is visual only**.
   No normalized sine substitute. 3-click one-shot atomic dispatch.
6. Validate original-derived fixed point positions, shot traces for all 13
   clubs and lie surfaces, bounce counts and phase transitions.
7. Validate water, sand, green, code8 cup, specials 9/10, hazard penalty and
   safe recovery, green slopes, tree interactions; track unverified separately.
8. Bind the approved artwork and animation timing; no placeholder game
   presented as production-quality.
9. Verify input/hitboxes, camera scaling, UI on actual 19.5:9, 20:9 portrait
   phones and Fold closed/open; fixed world physics must be screen independent.
10. Independently play entire first authored hole, scoring and putting,
    with physical device/emulator evidence; no scripted test passing as play.
11. Extend 3/9/18 rounds and progression only after baseline mechanics pass.
12. Check IP rights before any commercial release or incorporation of original
    game binaries/data/assets or source-derived data whose rights are unclear.

## Deliberately NOT carried from the abandoned HTML demo

No arcade sine trajectory, manually chosen arbitrary club yards, fake
12-stroke auto hole, 5-unit auto cup, "one-hop" ball, or mini hole squeezed
into one portrait frame. It is not a reference implementation.
