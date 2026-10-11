# Sensible Golf Windows v1.014 — Original-source recovery checkpoint
**11 October 2026 | Engine first | Protected development branch `feat/pixtee-godot-recovered-core`.**
**This is a factual recovery report, NOT a 100%-parity signoff.**

## Direct original-source results proved this session
1. **Original launch parity:** Windows v1.014 original machine code executed via Unicorn, 650 distinct club/lie/power/accuracy test vectors, **650/650 exact native recovered C++ state matches**. All 13 clubs × all 10 lie selector slots; original legal min/center/max accuracy and powers 0, 50 and 105. [CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159132161), [machine-generated test tool](../../tools/verify_original_launch_matrix.py).
2. **Original Welly tick producer:** Actual Windows routines at VA `0x40768B` and `0x407787` executed for club 0 and putter 12 under original-mode press sequences. Reconstructed C++ `OriginalWellyTick` produced **1,986/1,986 exact comparisons** of phase, rising/falling counters, power capture, angle, captured angle, cursor step/putter handling and timeout. [CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159584005), [native model](../../pixtee-godot/native/src/recovered_welly_tick.cpp), [original emulator](../../tools/trace_original_welly_machine_code.py). The original meter counts to 105, falls beginning at 70, and uses the original `count + (count >> 1)` descending power capture. **Per-callback producer is validated, but actual UI scheduler, click debounce and drawn colour-zone positions are NOT.**
3. **Extreme accuracy source observation:** Direct unbounded machine-code calls: **668/668 out-of-profile launch inputs left force zero (no launch)**, while **322/322 sampled in-range combinations launched**. [CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159975858), [original emulator](../../tools/probe_original_extreme_accuracy.py). This proves the isolated launch routine's range gate in tested inputs, NOT the surrounding UI's penalty, shot-attempt, or retry transition. The C++ original launcher currently throws for out-of-range; avoid replacing this with imagined huge hooks or slices.
4. **Original real gameplay/resource audit:** Fingerprint-verified `GOLFWIN.EXE` (SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`) and `GOLF.EPF` (SHA-256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`). **277/277** resources, **72** main/green/SPT course sets, **15 MCH files / 874 measured sprite frames**, **19 LBM bitmaps**, 34 major visual resource dimensions, **77 terrain descriptors**, verified subcell/slope distributions. [Actual-source audit](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159683998), [metadata extractor](../../tools/extract_sensible_complete_dimensions_and_collision_reference.py). Original pixel/audio/course map payloads not bundled in Pixtee.
5. **AI, turn and game mode source locations:** Verified in original executable, computer-shot diagnostics at VAs `0x40C8BF`, `0x40CACA`; "pick next player - tee off" diagnostic at `0x40C1E5`; original strings establish menu references to Play/Continue Tournament and Season, two named Open Tournaments, and a Skins Game. [Verified original PE call-site audit](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159740001). These identify locations for reverse engineering; they **do not yet reconstruct CPU AI, turn rules or tournament scoring**.
6. **Original special-event route:** Original code at `0x40A250` contains collision lookup call `0x409535` and several branch gates; original PRNG mutation helpers for code-9 low-height, near-cup lip and flag coordinate deflections were previously recovered. [Disassembly and 277-resource verification](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159000805). Activation conditions and complete event-11 continuation remain open.
7. **Existing native engine parity:** Fine MAPI 8×4 subcells; all 256 raw green slope magnitude/direction combinations; slope actually alters putt endpoint; real native 18-hole scoring / replay and 18 native CTests. [Native green CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38157838455). These are test-fixture checks and recovered-core behaviour, not entire original gameplay.

## Requested 23 outstanding — source recovery vs implementation

| # | Original mechanic/information | **Original-game evidence now** | Remaining gate |
|---|---|---|---|
| 1 | Welly timing | **1,986 exact original meter producer tick states recovered** | End-to-end game UI callback timing and normal click sequence |
| 2 | Welly colour/power windows | Exact numeric 0..105 and 70 falling, original rendering code located | Visual pixel boundaries, field scaling/green-yellow-red semantics |
| 3 | Extreme accuracy | **668 original no-launch cases, 322 in-range launch cases observed** | Whole game swing retry/stroke/penalty lifecycle and inputs |
| 4 | Tree strike activation | Original collision/event dispatcher identified; WOOD assets are artwork, not masks | Object classification, 3D/height activation and exact impact coordinates |
| 5 | Tree height/deflection | Original interaction/helper numerical mutations available | Full player-facing tree hit path and per-tick event integration |
| 6 | Flagpole collision | Original recovered `apply_flag_coordinate_deflection` | Full activation test and Godot game dispatcher |
| 7 | Code-9 special green | Original code-9 helper tested; normal 100-tick stop | Original continuation after event-11 special stop |
| 8 | Code-10 special green | Original stop state recovered | Complete original continuation/counters |
| 9 | Cup-lip rebounds | Original `apply_near_hole_lip_deflection` function tested | Activation conditions, cup/flag location edge tests |
| 10 | Other interaction event branches | Original `0x40A250` dispatcher and related lookup identified | Enumerate/execute every original event gate |
| 11 | Multiplayer turn order | Actual original `pick next player` source VA `0x40C1E5` identified | Full turn sequencing and scorecards |
| 12 | Original modes | Original Tournament and Season menu strings recovered | Per-mode rules and complete menu/controller transitions |
| 13 | Computer opponents | Original CPU shot handling VAs `0x40C8BF`/`0x40CACA` identified | CPU club/aim/power/strategy routines and all difficulty variants |
| 14 | Tournament/match rules | Original Open / Skins references verified | Exact competition/seed/cut/leaderboard/award rules |
| 15 | Putts/strokes/penalties | Recovered hole/round counters tested | All terminal/penalty/turn edge cases against original machine code |
| 16 | Camera scale | Original course dimensional reference available | Actual camera/scroll/render projection parity deferred |
| 17 | Ball/lie overlays | Original sprite dimensions/frame metadata recovered | Drawing pivots/overlay switching deferred |
| 18 | Welly art | Numeric meter machine trace recovered and original graphics resources enumerated | Exact colour/pixel presentation deferred |
| 19 | Slope arrows | Original slope 16×16 value decoding verified, green collision read | Arrow sprite frame/rotation mapping deferred |
| 20 | Impact animation triggers | Original event routine entry points located | Timed sprites/events and effect mapping deferred |
| 21 | Audio event timings | Original sound resources inventoried | Exact original sound call sites/tick timing deferred |
| 22 | Sound effect catalogue | Original file names, extension counts and hashes extracted | Semantic named event-to-sound mapping deferred |
| 23 | Course artwork dimensions | **72 source course dimensions + 874 individual sprite frame measurements recovered** | Visual alignment/approved Pixtee artwork integration deferred |

## Highest-priority remaining work

**Gameplay original-source branch closure:** original meter-to-UI tick ownership and coloured zones; exact shot rejection/attempt accounting; original tree/flag/lip activation and code 9/10 continuation; complete event classification; multiplayer and computer strategies; all original game mode/competition rules. All require actual original machine-code tracing and novel gold tests; do NOT invent branch semantics from visual intuition.

**Godot integration gate:** the native GDExtension shared library compiles, but a prior full editor headless import crashed and independent direct class-load smoke must pass before using "Godot playable engine" terminology. Linux/Android/phone/Fold final tests remain unverified.

**Source rights:** original EXE/EPF, sound, sprites, course geometry and source-derived code/tables are subject to commercial licensing/rights clearance. Current data and emulator are research only; shipping Pixtee should not contain unlicensed original payloads.

### Acceptance rule
A feature is COMPLETE only after source provenance is captured, its original runtime behaviour is observed, independent native tests match actual original machine execution, and the Godot/Android host demonstrates the same behaviour. **The 23 entries remain in the release gate; recovered fragments are not silently counted as closed.**
