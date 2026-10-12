# PIXTEE GOLF — MILESTONE ONE ENGINE DATA & COMMERCIAL HANDOVER
**12 October 2026 · authoritative engine-readiness checkpoint · owner directive: finish a commercial-quality playable engine, not endless original routine research**

**Development branch:** `feat/pixtee-godot-recovered-core` · [Draft PR #12](https://github.com/russellstokes-ai/Sensi-golf/pull/12) · **do not merge until release gates pass**.

> **CURRENT VERDICT — NOT YET COMMERCIAL-READY.** This repository already contains substantial measured Sensible Golf numerical data and an executable recovered native C++ engine, but has **only one independently authored test-hole adapter**, unresolved tree activation/height behaviour, incomplete end-to-end shot/event ownership, unapproved/not yet integrated product UI and assets, and no verified Android product build or rights clearance. Component parity and thousands of matching x86 probes are NOT a playable/commercial engine certificate. Do not describe three finished playable holes, full original parity, or cleared commercial licensing.

## 0. One-click canonical data and source links

| What | Canonical link | Use |
|---|---|---|
| **This handover** | [Milestone One commercial engine handover](MILESTONE_ONE_COMMERCIAL_ENGINE_HANDOVER_2026-10-12.md) | Single implementation and acceptance contract |
| **Machine-readable COMPLETE inventory** | [Milestone One source/status/gate register](MILESTONE_ONE_ENGINE_DATA_INVENTORY_2026-10-12.json) | 27 source/data areas with paths, status, precise remaining gate; audit with script/CI |
| Raw numeric evidence bank | [Original physics research](ORIGINAL_PHYSICS_RESEARCH.json) / [Godot reference copy](../../pixtee-godot/reference/ORIGINAL_PHYSICS_RESEARCH.json) | Source measurements: 13 clubs, 13×10 lie profiles, accuracy/curvature, 77 terrain descriptors, timing and related constants; **engineering evidence, not automatically licensed commercial data** |
| Original 18-hole numerical reference | [Original round fixture](../../pixtee-godot/reference/ORIGINAL_ROUND_REFERENCE.txt) | Test-only captured trajectories/round metadata, not approved Pixtee golf-course geometry |
| Full engine feature gate | [73-item completeness register](ENGINE_COMPLETENESS_GATE_2026-10-11.json) | Existing baseline: 50 native references, 23 unresolved/not integrated; engine `INCOMPLETE`, parity `UNPROVEN` |
| Mechanical original-source evidence | [15 game parity gates](RECOVERY_15_GAMEPLAY_SCOPE_AND_EVIDENCE_2026-10-11.md) · [Nine-area checkpoint](NINE_AREA_EXECUTION_CHECKPOINT_2026-10-12.md) · [23 recovery report](ORIGINAL_23_RECOVERY_REPORT_2026-10-11.md) | Source-machine tests, explicit missing behaviour and proof links |
| Native gameplay authority | [Engine C++](../../engine/src) · [public API](../../engine/include/sensigolf) · [engine test build](../../engine/CMakeLists.txt) | The **only** code allowed to advance physical ball/scores/PRNG/shot state |
| Godot 4.5 integration | [Godot README](../../pixtee-godot/README.md) · [native GDExtension](../../pixtee-godot/native/SConstruct) · [Godot headless binding test](../../pixtee-godot/scripts/EngineBindingCI.gd) | Presentation host; it must never introduce a second ball physics engine |
| Course geometry | [Lakewood Hole 1 adapter](../../pixtee-godot/native/src/pixtee_course_adapter.cpp) · [original course resources](../../engine/src/classic_course_resources.cpp) | **One** independently authored research/test hole; *not* three approved final holes |
| Reference-data integrity | [Data verifier](../../pixtee-godot/tools/verify_reference_data.py) · [reference workflow](../../.github/workflows/pixtee-godot-reference-data-gate.yml) | Guard table provenance and input fixtures |
| **Living course effects** | [Godot World FX and Game Feel Standard](PIXTEE_MILESTONE_ONE_WORLD_FX_AND_FEEL_STANDARD_2026-10-12.md) | Wind-coherent vegetation, subtle water movement, ball and terrain reactions, bird/crowd ambience, visual-only shader/particle system, 10 FX-specific tests |\n| Owner design/scaling | [Canonical gameplay contract](CANONICAL_GAMEPLAY_MECHANICS_LOCK_2026-10-10.md) · [World-scale/camera research](SCREEN_RESOLUTION_AND_WORLD_SCALE_2026-10-10.md) | Straight top-down portrait, ball-following camera, horizontal meter; Fold closed/open |
| Art source (protected) | [Approved production art brief](ASTRA_FINAL_ART_ASSET_BIBLE_2026-10-10.md) | Integrate only owner-approved Pixtee art as a **separate** production task; never touch Astra's HTML style preview |

**Original reference:** Sensible Golf Windows v1.014, EXE SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`. Original EXE, EPF, SPT/MAPM/MAPI/MAPS commercial course payloads, sprites, music, audio and original asset bytes are **not** commercial build inputs; original program/resources may be transiently examined in rights-limited original-source CI research only.

## 1. Exact Milestone One definition

Deliver a **production-grade native Pixtee game engine** able to complete **three distinct independently authored holes**, tee to cup, deterministically and without assisted/test-vector shortcuts, in a **real Godot 4.5 mobile-facing build**. The native C++ engine uses source-verified recovered golf behaviour or carefully specified independently developed legally usable equivalents; Pixtee owns its courses, branding, objects, approved sprites, new sounds and display code.

A successful hole must permit a real player to **aim → choose one of 13 clubs → press the same horizontal Whack-o-meter area through start/power/accuracy → hit → camera follow → bounce/roll → interact with all authored hazards and collidable objects → resolve pause/event/penalty → take another shot → putt/hole out → record score → advance to next hole**. Finishing all three must yield an accurate scorecard and stable saved/replayed state, on Android with no extra or missing strokes.

**Milestone One is NOT:** an 18-hole art/content expansion, career/rewards, CPU opponents, multiplayer matches, tournaments or season mode. Those are Milestone Two. Their pre-existing verified data and working source must be preserved; do not remove or replace them.

### Mechanical invariants (verified reference contract)
- Fixed simulation step `936/65536 seconds` (~70.017 Hz), independent of 60/90/120 Hz rendering. Ball positions/forces remain original signed fixed-point state; raw heading is `0..4095`.
- **13 clubs** (`0..12`, putter `12`); **13×10** club/lie selectors, original 11 accuracy/curvature profiles; ordinary power `0..105`, centre accuracy `63`.
- MAPI: original fine lookup is **8×4 subcells per 16×8 world-unit MAPM tile**, **2×2** units per subcell. Green geometry has independent MAPS resource and recovered source slope encoding **16 directions × 16 magnitudes**.
- Original source semantics take priority for gravity, flight, spin, repeated bounces, friction, stopping, cup, flag, hazards, world OOB and lie change. Never use viewport animation, a fake bounce, inferred sprite rectangle, or generic circular object collider to determine the simulation.
- Original Welly is not a normalised sine/power slider; first click starts, second captures power, third captures accuracy, with original counters and charge/no-flight rules. The approved horizontal visual presentation may map these raw states but **cannot** change their sample semantics. No invented red/yellow boundary values.
- Original source **code 8** means cup terminal; **code 9/10** have distinct special-green behaviour/counters; **code 35** involves hazards but not all named OOB sources; **out-of-world** is a separate source route. Code labels must not be conflated.
- Terrain/green/physical tree/flag/object presence comes from distinct authored course data, **never** the artwork bitmap, advertising board or random graphic placement.
- User-approved assets maintain top-down view, golfer scale, course-perspective crop, tee/green adverts oriented toward the tee, birds, crowd, grass/flower breeze and subtle water movement. They are **render-only** unless an explicit authored physical object record exists.

## 2. DATA STATUS — what is truly banked and what is not

The [machine-readable inventory](MILESTONE_ONE_ENGINE_DATA_INVENTORY_2026-10-12.json) is the authoritative 27-item coverage map. At this checkpoint:
- **6 areas** have scoped verified numerical/source evidence; this says nothing about integration, legal release or full-session parity.
- **13 areas** have a native integration but still lack full Milestone One acceptance.
- **7 areas** remain blocked by unresolved original/gameplay/product release behaviour.
- **1 area** lacks production content: **two more independently authored playable holes** plus final approval of all three.
- Existing old register tracks **73 original mechanics**, **23 unresolved/not integrated** at the original baseline; it is broader than the Milestone One 27-area grouping. All **15 original full-gameplay end-to-end parity gates remain open**, not passed.

### Already available: DO NOT extract again
| Original mechanics and data | Main source or test |
|---|---|
| Numeric clock, coordinate precision, aim and fixed-point handling | `engine/src/classic_shot_model.cpp`, `recovered_flight.cpp` |
| Original clubs, lie and accuracy source bank | `spec/pixtee/ORIGINAL_PHYSICS_RESEARCH.json`, `pixtee-godot/reference/` |
| Flight force, trajectory, multi-bounce, ground drag/roll, putting and PRNG | `engine/src/recovered_{flight,ground,prng}.cpp`; `engine/tests/` |
| Tee/cup SPT, MAPM, MAPI, MAPS, terrain and slope nibble decoding | `engine/src/classic_course_resources.cpp`, `recovered_course_lookup.cpp` |
| Original Welly numerical producer, first start and native three-press data | `pixtee-godot/native/src/recovered_welly_tick.cpp`, `engine/src/classic_mobile_controls.cpp` |
| Source-proven world OOB, hazard recovery, no-flight strokes, code8/9/10, flag/cup-lip fragments | `engine/src/classic_hole_session.cpp`, `recovered_{world_boundary,hazard_recovery,putter_terminal,interactions,flag_setup}.cpp` |
| Ten-slot event countdown, release/wait and safely gated callback handoff | `engine/src/recovered_animation_events.cpp`; [original callback x86 test](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38235564193) |
| Round scoring, replay/journal and Godot bridge test scaffolding | `engine/src/classic_round_session.cpp`, `classic_replay_journal.cpp`, `pixtee-godot/native/src/pixtee_round_runtime.cpp` |
| Additional CPU/multiplayer/competition source routines | `pixtee-godot/native/src/original_cpu_*.cpp`, `original_turn_*.cpp`; preserve for Milestone Two |

### Only four original-game questions merit further FIRST-MILESTONE investigation
1. **Physical tree/object collision:** exact original object identity, world placement model, physical masks/footprints, height, impact and deflection. MAPM high bits and visual frame dimensions are **not** adequate evidence. Do not enable fake circular tree collisions.
2. **Complete shot and event lifecycle:** all original source ordering from live ball movement to terrain/object/flag/cup or hazard, event queue, acknowledgement, penalty and the next usable shot. Distinguish authentic x86 subroutine tests from 3-hole playback.
3. **Meter display/input timing:** authoritative logical producer has been recovered; exact host polling, input debounce and mapping of red/yellow zones from authentic source inputs into Pixtee's horizontal view still need whole-interaction tests.
4. **Original callback/event side effects:** dispatch `0x8000` original handler token is known, **not** the actual original callback content. Map to independently authored safe Pixtee events and verify each event resolves. Preserve unknown-code rejection; no raw address execution.

Flagpole, cup lip, OOB, code9/10, terrain and hazards already have authentic component evidence; their **actual three-hole scenarios** still need integration. Research is not a reason to reopen entire original binary analysis.

## 3. THREE REQUIRED PRODUCTION HOLE MANIFESTS

**Current reality:** `make_lakewood_hole_1()` exists, with independently authored green, sand, water, rough, fairway and original-format MAPM/MAPI/MAPS geometry. It is an **engine fixture**, not accepted as production artwork/collision completion. The multi-hole test uses artificial original-rule fixtures; those are NOT Holes 2/3.

Create exactly three **separate authored** hole packages (no original copyrighted course geometry). Each must expose:
- Stable `hole_id`, `resource_id`, independently approved `par`, `world_width`, `world_height`, 4 tee slots, cup/flag and source-consistent logical origin, protected object manifest and terrain bank/checksums.
- Distinct hazard routing: at minimum, put an actual fairway/rough/green approach, sand/bunker, water or no-go, and world OOB across the **three-hole suite**; document hazards absent from a particular hole.
- At least one intentionally collidable **tree** with authored 3D footprint/height and tested varying ball heights; separately declared decorative trees never collide.
- At least one **full signed-off green** per hole: slopes (including flat where authored), code8 cup, code9/10 transitions only where legitimately authored, near-cup lip and reachable putting positions.
- Tee-to-green routing at original physical world scale so a normal mobile frame shows **only a portion of the hole**, not the whole course. Wind as approved per hole is an explicitly authored gameplay/environment property; visual breeze alone must not create hidden force changes.
- Deterministic playable completion vector: one launch, one terrain interaction, at least one green approach, one putt, valid final score. Also dedicated regression shots for each collision and special.
- Visual manifest hooks for approved sprites/animation pivots, sponsor boards around tee/green off walkways, and scene decor **without influencing core physics**.

**No “three holes completed” claim** until all three native resource packages are authored, registered in the Godot bridge, and each can be finished by gameplay controls and by deterministic CI fixture on native and mobile device.

## 4. Commercial-ready engineering handover: mandatory acceptance matrix

| ID | Required PASS, no substitute | How to verify | Current |
|---|---|---|---|
| M1-01 | 27-area inventory paths and evidence still present; source/reference integrity verified | [Inventory audit script](../../tools/verify_pixtee_milestone_one_inventory.py), [existing bank verifier](../../pixtee-godot/tools/verify_reference_data.py), [54 paths and source bank CI PASS](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38246626705) | **SOURCE AUDIT PASSED; commercial gate RED pending full signoff** |
| M1-02 | Three independent, complete and owner-approved collision/green course resource packages | Three registered IDs, dimension/tee/cup/object assertions, test actual collisions | **RED** (1 adapter) |
| M1-03 | Original-informed trees: object identity, altitude, deflection and solid-vs-decorative | Source machine study + native high/low tree trajectory parity | **RED** |
| M1-04 | Fixed-step flight/roll on real three holes; no 60/90/120Hz changes | Headless trajectory golden files + real Godot snapshots + ball follow | **RED** |
| M1-05 | Three-click Whack-o-meter, colors/pointer, retry and touch debounce | Frame/input timestamp capture 0..105, aiming and swing lifecycle, no doubled stroke | **RED** |
| M1-06 | Hazard/water, sand, OOB, flag and cup physically correct | Ball-through-trigger full-game tests, verified source state after event | **RED** |
| M1-07 | Special-green events, cup-lip and all pause/recovery transitions never freeze | Each code8/9/10/recovery test reaches ready/hole scored via public bridge | **RED** |
| M1-08 | Three consecutive player-completed holes with accurate par/strokes and no manual scoring | Real input-to-hole scored-to-next-hole-to-final scorecard replay | **RED** |
| M1-09 | Save/resume, repeatable PRNG, stable shot log, interruptions mid-round | Persist/reload after each shot and app pause, checksum/tamper test | **RED** |
| M1-10 | Working Godot 4.5 visual host and **authoritative C++ only** | Real gameplay scene; no GDScript/Kotlin physics; actual game pad/phone input | **RED** |
| M1-11 | Frozen-approved Pixtee art, legible HUD, sponsor boards and living course shaders/particles implemented without altering physics | Reference screenshots, asset hashes and [all ten FX checks](PIXTEE_MILESTONE_ONE_WORLD_FX_AND_FEEL_STANDARD_2026-10-12.md) across three holes | **RED** |
| M1-12 | Android arm64 build signed & tested on ordinary portrait and Fold closed/open; resize-safe | Native extension build, Android app export, device smoke and orientation test | **RED** |
| M1-13 | No original protected content; commercial licence/clean-room review cleared | Asset/source provenance review + APK content audit + legal approval | **RED** |
| M1-14 | Performance, restart/crash handling and final release QA | Long soak at 60/90/120Hz, accessibility/touch, battery/memory/ANR and no event deadlock | **RED** |

**The evidence columns must contain links to passing commits/workflows; turning a RED cell green requires a test, not a document edit.** The audit script checks manifest/provenance consistency, not these gameplay/device/legal gates. A passing inventory audit is **NOT** product signoff.

## 5. Delivery order and strict scope freeze

1. **Freeze original evidence bank.** Preserve reference hashes and native tests. Do not re-recover already verified club/lie/slope/PRNG data. Finish only source-original tree activation/height, meter input-to-sample and unsupported host event sequence **if the first three-hole game cannot function accurately without them**.
2. **Build native 3-hole package** without new physics. Supply Hole 2/3 unique Pixtee-authoring data, verified tee/cup/flag/tree/green/hazard manifests; hook each to `ClassicCourseResources` and a real `PixteeRoundRuntime` resource map.
3. **Unify the public shot API.** No desktop/sample test-vector input in user gameplay. Route touches→meter tick→`ClassicHoleSession`→pending event→host acknowledgement→new shot/score→next hole. Add deterministic three-hole full trajectory and state fixtures.
4. **Bind production Godot host and living effects.** Apply [Godot World FX standard](PIXTEE_MILESTONE_ONE_WORLD_FX_AND_FEEL_STANDARD_2026-10-12.md) to frozen Hole 1 **without changing approved asset bytes or geometry**; align with the separate Work-owned course-production standard before implementation. Live vegetation, subtle water, ball shadow and surface reactions are visual listeners to authoritative C++ native physics, with cosmetic-only visual RNG and Android quality settings. Load canonical approved Pixtee graphics unchanged; implement top-down ball-follow, 360-logical-width world-adaptive camera, X left/right aim, club cycling, horizontal Welly display, touch target, green slope arrows, wind indication, mini-map, scorecard and non-colliding advert boards. **No screen reflow can change world units or the ball's result**. Astra's separately owned 3-hole HTML style-approval preview is *not* the engine build.
5. **Android/device and commercial release gate.** Build arm64 APK and run closed/open Fold and normal phone gameplay; exact replay and event/score check across FPS. Independently review distribution rights: original numeric data and reverse-engineered code need clearance or lawful reimplementation before public commercial shipping.
6. **Freeze evidence.** Deliver one source commit/PR, complete binaries/export config, 3 verified hole manifests, build instructions, regression reports, device matrix, art/object manifest and a legal decision record. Never claim success from Linux-only CTest or x86 slice verification.

## 6. Existing test/run links and commands

- [Godot native GDExtension Linux build workflow](../../.github/workflows/pixtee-godot-gdextension-build.yml): genuine headless module bind, **not** Android, actual art or full game.
- [Native C++ engine/round/geometry smoke](../../.github/workflows/pixtee-godot-native-core-smoke.yml): compiled course adapter, 256 slope combinations, native 18-hole test fixture, and original meter producer.
- [Reference data integrity workflow](../../.github/workflows/pixtee-godot-reference-data-gate.yml).
- [Original meter outer/UI source vectors](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38206789435), [source flag loader](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38207010331), [source OOB](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38181667415).
- [Original event callback dispatch 3,576/3,576](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38235564193); [original animation queue reset 1,300/1,300](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38236194547).
- [Original CPU score selection 2,900/2,900](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38237924678) — **Milestone Two**, not permission to declare a complete computer player.

```sh
# Verify the reference-data files have not changed
python3 pixtee-godot/tools/verify_reference_data.py
# Validate this inventory, present sources, honest status and original data bank
python3 tools/verify_pixtee_milestone_one_inventory.py
# Build native C++ core and run ALL native engine tests
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 2
ctest --test-dir build --output-on-failure
# Run actual Godot GDExtension workflow for Linux class ABI.
# Android export, actual 3 holes and commercial rights still require independent gates.
```

## 7. Owner acceptance and no-regression rules

- Preserve original Sensible Golf restoration on `main`, approved final Pixtee artwork ZIP, approved brand, bird side-view animation, game-viewpoint and Astra HTML prototype unchanged.
- World view **straight down**, same apparent Sensible Golf scale, partial-hole crop and ball-follow camera. Portrait handset is default; Fold open/closed uses adapted HUD, not distorted ball physics.
- Keep purposeful environmental elements: bushes not gratuitously crowded; breeze moves grass/flowers, still water subtle flow; birds region-specific; golfer/crowd subtle breathing; sponsor boards off pathways, facing the player camera near tees/greens. These are **presentation** and do not add physical contact unless explicitly manifested.
- Never silently fill missing original behaviour with guessed values; if a commercially acceptable independent design deviates, write a separate proposed exception that states the actual behavioural difference, how it is tested and approval status.
- Document original-source code/table/art licence concerns. Reverse engineering and a passing source-comparison test alone **do not confer commercial rights**.
- **STOP CONDITION:** Milestone One is finished only when M1-01 through M1-14 have verifiable evidence and commercial clearance. Until then use `NOT_COMMERCIALLY_READY`. Full 15-point original-game acceptance remains separate; do not misuse the number of successful numeric probes to imply those gameplay systems are finished.

**Next concrete engineering task:** write/test the original tree collision descriptor/object geometry interface **and** create the two missing Pixtee-authored hole resource manifests; then wire actual 3-hole end-to-end gameplay instead of another unrelated source disassembly.
