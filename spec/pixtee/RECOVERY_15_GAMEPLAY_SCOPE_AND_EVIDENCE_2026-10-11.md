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
| 3 `swing_accuracy_extremes` | Original 990 launch cases including 668 no-flight out-of-profile results | Complete game-level invalid swing/retry/penalty flow |
| 4 `tree_collision_activation` | Original event address map and course collision lookup framework | Exact tree object identity/mask and 3D impact activation |
| 5 `tree_height_deflection` | Source collision PRNG math for some events | Exact tree impact height and direction changes with original-machine dynamic proof |
| 6 `flag_collision` | Original contact branch **2,712/2,712**, seeded deflection helper | Original course flag object existence and full collision path in live gameplay |
| 7 `code9_special_green` | Original activation **4,390/4,390**; putter terminal shared code-9 behaviour **2,160/2,160** | Full original post-event no-flight/scoring state and 100-tick continuation |
| 8 `code10_special_green` | Code-10 putter terminal part of **2,160/2,160** original checks | Complete original code-10 continuation and unusual-mode transitions |
| 9 `near_cup_lip` | Full *isolated contact decision* **3,172/3,172**, PRNG impact helper | Real player/mode/distance gate, live ball event sequence and scoring |
| 10 `misc_course_event_dispatch` | Real original x86 CFG + source-gated code9/lip/flag branches | All other event codes, trigger conditions and full dynamic coverage |
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
