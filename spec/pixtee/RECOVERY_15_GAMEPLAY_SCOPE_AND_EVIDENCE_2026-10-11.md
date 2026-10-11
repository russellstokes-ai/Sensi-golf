# Pixtee — Fifteen gameplay requirements: original Sensible Golf recovery evidence
Date: **11 October 2026**. Original executable: **Windows Sensible Golf v1.014**, SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`.
Branch: `feat/pixtee-godot-recovered-core`. Source payloads and unlicensed artwork excluded.

**Critical acceptance principle:** "Machine-verified subroutine" is NOT "full gameplay requirement closed". The tracked 15 gameplay requirements below still require end-to-end original game/state/round parity; the additional eight graphics/audio/presentation requirements remain deferred.

## New executable-to-C++ original behavioural findings

- **Code-9 special green collision decision**: exact original instruction branch at `0x40A374..0x40A3E0`, three choices (ignore, random deflection, 100-tick special stop): **4,390/4,390 exact**. [Run 38165847870](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38165847870). Native source `engine/src/recovered_interactions.cpp`.
- **Near-cup-lip contact decision**: source `0x40A456..0x40A4A8`, **3,172/3,172 exact**. [Run 38165941630](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38165941630). Fixed two prior inaccuracies: there is NO lower bound on signed intermediate height, and original mode/player word uses a SIGNED 16-bit comparison, so `0xFFFF` is treated as -1.
- **Flagpole contact branch**: source `0x40A501..0x40A53A`, **2,712/2,712 exact**. [Run 38166139060](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38166139060). Crucially the source uses the high 16-bit word of raw **ball X** for both comparisons: flag X = ball X high; flag Y = ball X high minus the arithmetic-right-shifted ball-height high word. The inferred BALL Y comparison was wrong. No nonnegative height restriction. Native source and regression tests corrected. **Do not enable in actual courses until verified object existence and coordinate spaces are proved.**
- **Code-8/9/10 club12 terminal state**: actual original descriptor indices {code8:7, code9:23, code10:15}, **2,160/2,160 exact original-versus-native** tests for event ID, counters, pause=100, flags, terminal gate and signed 16-bit mode behavior. [Run 38166483327](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38166483327). Native source `engine/src/recovered_putter_terminal.cpp`. This does not prove original *post-pause game continuation* or modes.
- All original launch/meter/256-putt and original multiplayer/CPU subroutine proofs recorded in [original 23 recovery report](ORIGINAL_23_RECOVERY_REPORT_2026-10-11.md) and [nine gameplay recovery checkpoint](REMAINING_NINE_ORIGINAL_GAMEPLAY_RECOVERY_2026-10-11.md). Source-derived PRNG and physics regression tests preserved.

## Fifteen original gameplay acceptance items

| ID | What is recovered and executable | Remaining full-system gate |
|---|---|---|
| 1 `welly_accurate_meter_timing` | x86 per-callback meter source 1,986/1,986 and native bridge/rejection handling | Original outer input scheduler, debounce and whole timing sequence |
| 2 `welly_colour_boundaries` | Original raw meter values and accuracy windows | Original display-to-input matching; visual art itself deferred |
| 3 `swing_accuracy_extremes` | Original 990 x86 launch cases: 668 no-flight attempts AND 322 launches **all charge +1 hole stroke/+1 total stroke**; native hole/round controller corrected | Full UI retry/attempt ownership and all mode-specific penalties |
| 4 `tree_collision_activation` | Original event address map and course collision lookup framework | Exact tree object identity/mask and 3D impact activation |
| 5 `tree_height_deflection` | Source collision PRNG math for some events | Exact tree impact height and direction changes with original-machine dynamic proof |
| 6 `flag_collision` | Original contact branch **2,712/2,712**, seeded deflection helper | Original course flag object existence and full collision path in live gameplay |
| 7 `code9_special_green` | Original activation **4,390/4,390**; putter terminal shared code-9 behaviour **2,160/2,160** | Full original post-event no-flight/scoring state and 100-tick continuation |
| 8 `code10_special_green` | Code-10 putter terminal part of **2,160/2,160** original checks | Complete original code-10 continuation and unusual-mode transitions |
| 9 `near_cup_lip` | Full *isolated contact decision* **3,172/3,172**, PRNG impact helper | Real player/mode/distance gate, live ball event sequence and scoring |
| 10 `misc_course_event_dispatch` | Real original x86 CFG, source-gated code9/lip/flag branches, and terminal code35/code50 route **2,730/2,730 original-machine exact** | All other event codes, trigger conditions and full dynamic coverage |
| 11 `multi_player_turn_order` | Original chooser **1,564/1,564**, mode1 score adjust **3,192/3,192**, mode4/6 side/slot **1,728/1,728** | Complete independent per-player live round/turn and result state machine |
| 12 `original_game_modes` | Original mode1/4/6 branch semantics and source menu strings | Complete all original practice, match, skins, tournament, season gameflows |
| 13 `computer_opponents` | CPU skill parameters **24/24**, candidate selection **378/378**, shot adjustment **1,620/1,620** | Whole original CPU candidate generation, club/aim/power selection and shot planner |
| 14 `match_tournament_rules` | Original competition-side turn primitive recovered | Whole original tie, leaderboard, cuts, match resolution, season and mode rewards |
| 15 `putts_strokes_penalties` | 650 original launches, 256 original green putts, tested cup/hazard counters, mode1 score branch, code8/9/10 terminal | Every special end-state penalty, multiplayer scoring and original transition |

**No 100%-clone-engine claim. No fabricated gameplay rule as a substitute for a missing original routine.** The live native gameplay and Godot smoke suites passed on the latest tested native engine commit. A production-ready original-equivalent Android playable build is not signed off.

## Authenticated original executable numerical test suites introduced here

- `tools/verify_original_code9_gate.py`
- `tools/verify_original_cup_lip_activation.py`
- `tools/verify_original_flag_activation.py`
- `tools/verify_original_special_green_terminal.py`
- `.github/workflows/pixtee-original-code9-activation.yml`
- `.github/workflows/pixtee-original-cup-lip-activation.yml`
- `.github/workflows/pixtee-original-flag-activation.yml`
- `.github/workflows/pixtee-original-special-terminal-matrix.yml`

All compare executed Windows source-machine code with C++ under zero tolerance. Branch-specific evidence does not license the original game or its resources for commercial reuse. Review rights before shipping source-derived logic, data and media.

## Continued engine-only recovery — source-backed progress

**UI/design ownership:** Astra handles the basic HTML style-approval prototype. The engine recovery work here does not create or modify HTML, graphics, sprites or their assets.

### Original CPU shot search reconstruction

The original machine-code source at `0x40C6B2..0x40C77F` has yielded:
- **24 verified source table records** for original player modes 4–9 and bands 0–3 (four WORD values per entry), stored as research data at `pixtee-godot/reference/ORIGINAL_CPU_SCAN_TABLE_2026-10-11.json`.
- **1,274/1,274 exact** authentic Windows executable-versus-native scan setup comparisons, including source count, two angular extents, fourth word, 154-unit divider and remainder. [Evidence](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38169533045)
- Original *entire per-heading candidate sweep* over 24 table records and five starting directions: **120/120 exact**, **4,605 individual candidate headings identical**. [Evidence](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38170143652)
- C++ source: `pixtee-godot/native/src/original_cpu_search_grid.{hpp,cpp}`, `original_cpu_heading_sweep.{hpp,cpp}`. Exposed to Godot through the `PixteeRoundBridge.original_cpu_scan_grid` and `original_cpu_heading_sweep` interfaces.
- **Remaining**: original terrain-aware candidate construction, the original shot evaluator at `0x40C93E`, CPU club selection and overall play strategy, independent per-player round engine and end-to-end original AI matches. Do not sign off `computer_opponents` yet.

### Original code9/code10 special green timer and state recovery

- Original Windows source `0x40A7E0..0x40A7F1`: **1,230/1,230 exact** source-versus-C++ comparison of signed 16-bit pause decrement, expiry and UI latch. [Evidence](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38169920597)
- Critical detail: a 100-count timer waits at `remaining=0` and only enters the original recovery branch on the **101st logical update**, when `0` wraps to `0xFFFF`.
- Source `engine/src/recovered_putter_terminal.cpp`, now used by `ClassicHoleSession::step`; native regression forbids premature acknowledgement on the 100th tick.
- After signed timer expiry the original executes `0x40A7F1..0x40A8D6`, the *same already recovered post-stop ball/player relocation* as the code-35 hazard path. The C++ hole session now calls a shared `apply_original_post_stop_recovery()` after expiry for both conditions, avoiding the former unsupported fixed-position resume.
- **Remaining**: full original outer presentation/UI player-turn scheduler and 60-count post-recovery event flow, plus complete original code9/code10 round replay parity. Native round/regression CI for the newly connected relocation is a separate mandatory acceptance gate; do not claim all-session success until it passes.

**All remaining 15 full gameplay systems retain their original-machine end-to-end signoff requirements. The eight visual/audio requirements remain deferred.**

### Replay regression: paid no-flight original swings (native integration)

- The original Windows v1.014 **668/668** out-of-profile attempts charge a stroke but leave the ball stationary (machine proof above). This pass fixes only deterministic platform replay of that already-recovered rule; it does **not** assert the original Windows game contained the same mobile save format.
- `PixteeRoundRuntime` now appends completed no-flight inputs as ordinary version-1 replay frames, and `ClassicReplayJournal::replay_into` independently re-executes and validates phase, coordinates, distance, stroke count and green mode.
- Manual three-click and original Welly 2-press paths each have round-trip save/reload tests. Godot adapter CI: [run 38174394126](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38174394126); main native tests: [run 38174398909](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38174398909). Both succeeded.
- **Conservative gate remains:** no-flight attempts made with already-zero distance to the hole cannot yet be replay-signed-off; code-9/10 continuations, unsupported event branches and full original turn scheduler remain unrecovered. None of the fifteen complete original-gameplay requirements is closed by this platform persistence fix.

### More authenticated terminal-dispatch source routing

- Real Windows v1.014 x86 **0x40A686..0x40A6AB** routes terrain code 35 through a signed ball-height high-WORD threshold of **200**, code 50 through a separate branch, and all others to the default route. C++ isolated predicate `original_terminal_stop_contact` matches **2,730/2,730** original source-machine samples across the three routes: [CI 38174782111](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38174782111). [Main native CI 38174783233](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38174783233) passed.
- Original 77-entry terrain table uses code 50 for **GREEN D2, GREEN D3, GREEN D4, and MUD D1** descriptors. **Code 50 is not evidence of a tree object.** This routine remains isolated from live terrain collision until activation conditions and complete ball impact effects are proven.
- This is a new source-verified *subroutine*, **not** sign-off of tree collision, code-50 dynamics, multiplayer, or any of the 15 entire gameplay systems. No art or HTML has been changed.


### Continued original x86 terminal rebound and event-latch recovery

- **Full force rebound segment** from original Windows v1.014 x86 `0x40A7BD..0x40A7E0`, including wrapping 32-bit NEG, two signed SAR operations, ADD, and signed JG continue/stop decision: **4,524/4,524 exact native-versus-original** vectors (including signed 32-bit extremes). [Machine CI 38178442764](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38178442764).
- **One-shot event latch plus rebound** from original `0x40A79D..0x40A7E0`: bit 0 of original player state, event-zero request, horizontal/vertical forces and continuation route: **3,912/3,912 exact** vectors. The original nonphysics audio CALL is bypassed **only in Unicorn emulator memory** while its call invocation is counted; no original executable file or audio payload modified or bundled. [Verified CI 38178607009](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38178607009).
- Newly implemented portable routines `original_terminal_rebound` and `original_terminal_rebound_with_latch` in `engine/src/recovered_interactions.cpp`; exact executable emulator and C++ comparison scripts in `tools/verify_original_terminal_rebound*.py`. The additional native CI also passed: [CI 38178609751](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38178609751).
- **Not signed off:** original object/terrain activation, actual tree identity and height geometry, other event codes, complete audio callback/event timing, full collision integration, AI/multiplayer/game mode logic and whole-game original parity. The isolated predicates and effects are deliberately not turned on for unverified courses.


### Authenticated numeric special-terrain termination: codes 35 and 50

- Newly recovered original **Windows v1.014 x86 `0x40A686..0x40A7E0`**, beyond the earlier route-only 2,730 comparisons:
  - **Code 35** when signed high WORD of ball height is <=200: source writes horizontal force = 0, vertical force = 0, ball height = 0, sets player status bit 15, and issues event-call argument **11**.
  - **Code 50** at every height: issues event-call argument **1**. Original club **12** then sets player status bit 15 and exits **without changing ball forces**; other clubs execute previously recovered exact signed force rebound and source continuation/no-continuation.
- **8,336/8,336 exact native-versus-original cases**, including 3,916 code-35 and 4,420 code-50 vectors; first source x86 exit destinations `0x40A7E0` (6,570 cases) and `0x40A626` (1,766 cases). Verified ball height, both forces, original player status, event-call code, and exit destination. [Original machine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38179160345).
- Native implementation: `original_special_terminal_terrain` in `engine/src/recovered_interactions.cpp`; reproducible x86 comparison `tools/verify_original_special_terminal_terrain.py` and the corresponding C++ oracle probe. Sprite effects and host audio/event subroutine CALLs were replaced with equal-sized NOPs **only inside Unicorn emulator memory** to isolate ball-state parity; the authentic CALL sites and event arguments were recorded. The source EXE was SHA-256 checked and never modified or committed.
- The terrain database describes numeric code 35 (including water/hazard surfaces) and numeric code 50 (including alternative greens/mud). **This is NOT evidence either code is a tree**, and object/class lookup and original live activation remain open. We have not connected this isolated routine to live collision, rendered source assets, or claimed full event-scheduler parity.


### Original airborne high terrain source entry is distinct from terminal contact

- Identified and recovered the **separate original Windows v1.014 x86 entry `0x40A5D3..0x40A5F2`** (after prior original branch exclusions for numeric codes 0–10). This route chooses event-11 hard stop on **code 35 with signed height-high <=200**, but enters event-1 rebound on **code 35 above 200** and other numeric codes >10; the grounded terminal entry `0x40A686` instead uses an event-0 latch for some of those cases. The game must NOT reuse only one predicate for both entry points.
- Recovered native isolated helper `original_airborne_high_code_contact`; tested **11,112/11,112 exact original-machine vs C++** cases across numeric codes 11, 35, 36, 49, 50, 51, 60, 65535, club and player state flags, signed height threshold cases and varied 32-bit forces. Original machine reached `0x40A626` in 4,099 cases and `0x40A7E0` in 7,013; observed event-call arguments 1 (10,087) and 11 (1,025). [Original x86 workflow run 38179474254](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38179474254); [full native CI 38179477201](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38179477201).
- Research-only oracle `tools/verify_original_airborne_high_code_contact.py` imports authenticated setup and emulator-only CALL stubbing, compares exact original player flags, ball height, signed force results and branch. **This does not validate real-world tree object geometry, classify actual course object presence, or execute original rendering/sound/host callbacks**; no new live-source collision hook or source art has been introduced.


### Grounded course lookup → original terminal effects: synthetic and genuine first-hole MAPM

- Added **native engine module** `engine/src/recovered_mapped_terminal.cpp` plus header, independent regression test, CMake integration and reproducible original x86 comparison workflows. Reuses already proven native `lookup_course_subcell` → `classic_terrain_descriptor` → the recovered code-35, code-50, or generic event-0 contact-latch and rebound routines. Excludes code-8 cup handling, zero vertical-force idle handling and the unverified upstream shot-collision activation path.
- Original authentic Windows v1.014 machine route: **`0x40A65E` → actual source `0x409535` map lookup → original `0x41DFF7` terrain table → `0x40A686..0x40A7E0`**. **5,312 / 5,312** synthetic MAPI descriptor/selector examples matched exactly, across 12 terrain codes, original event id 0/1/11 or suppressed repeat, signed forces, ball height, two independent player flags, continuation decision and original descriptor index. [Source-machine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38180002367).
- More importantly, executed that original source routine with **genuine, private, extracted `MAPM01.MAP` + `MAPI01.RAW` + `MAPI02.RAW` first-hole data** from the SHA-256-authenticated v1.014 archive; native C++ decoded the same real cell descriptor and reproduced original game state: **2,414 / 2,414 exact**. Original first-hole selection had **5,693 eligible numeric code-35 subcells** and **14 code-50 subcells** (overlap-free classification in the authentic first-hole map). Verified all available code-50 subcells, sampled 1,200 code-35 contacts and representative ordinary terrain; source event-count and x86 exit targets agree with C++. [Real-course x86 CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38180193973); [native CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38180197297).
- **7,726 / 7,726 new exact original x86 map-to-terminal comparisons.** Only source EXE and map/MAPI hashes, case counts and summary outcomes are retained in CI artifacts; **no original executable, unlicensed map raster, sprite or audio bytes** are committed. The original course data remains in Actions' transient private workspace.
- **Still blocked:** upstream world/object collision scheduling, actual tree-object detection and height geometry, 0x40A65E admission from a genuine fully simulated shot, original code-8 cup route through this adapter, and host event/sprite/audio callback side effects. Isolated **source-scoped** collision adapter is not automatically activated on unverified Pixtee courses; **all 15 end-to-end gameplay gates remain open**.
