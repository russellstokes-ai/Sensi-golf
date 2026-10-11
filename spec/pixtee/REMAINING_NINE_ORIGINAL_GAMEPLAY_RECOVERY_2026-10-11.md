# Pixtee / Original Sensible Golf — nine-area recovery checkpoint
Date: 2026-10-11
Branch: `feat/pixtee-godot-recovered-core`
**Status: INCOMPLETE. Engine/research only; this does NOT mean all original mechanics are recovered.**

User instruction: complete the *actual original Sensible Golf Windows v1.014 gameplay engine*, with graphics postponed, and never invent substitutes for missing mechanics.

## Verified original source and integration work

### 1. Welly-o-Meter — advanced, not fully closed
- Already proven **1,986/1,986** original binary vs C++ per-callback Welly states. Source: `tools/trace_original_welly_machine_code.py`; original-source [CI 38159584005](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159584005).
- This pass **connected** `OriginalWellyTick` to `PixteeRoundRuntime` with `start_original_welly` and `tick_original_welly`, and exposed both through the Godot `PixteeRoundBridge` along with original power, accuracy, angle and step data.
- The original two-press sequence is adapted to the core's atomic three-phase shot request. It prevents aim/club changes during the source-authenticated sweep and prevents incomplete/no-input meter timeouts from creating strokes; **source-authenticated completed out-of-profile swings charge one stroke even with no ball flight**. Native regression includes a no-input timeout, valid capture and rejected-then-valid retry. [Native CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38161291827).
- **Missing:** verified game-level callback scheduling, click debounce, exact graphical colour zones and original outer retry/penalty UI. Completed out-of-profile source attempts now have the original +1/+1 counters; incomplete click sequences still are not counted. The full original outer retry/display lifecycle remains unverified.

### 2–5. Trees, flag, cup, green codes 9/10, event dispatcher — advanced, incomplete
- Captured an original binary instruction/control-flow atlas from actual SHA-verified v1.014: [CI 38161351577](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38161351577), [extractor](../../tools/trace_remaining_original_dispatchers.py).
- Ball pre-update dispatcher: **194 disassembled instructions / 28 conditional branches / 20 calls** in the studied 0x40A250..0x40A581 range.
- Putter/event dispatcher: **309 instructions / 44 conditions / 20 calls** in 0x40A581..0x40AA78.
- Recovered and unit-tested exact x86 branch predicates in `recovered_interactions`:
  - Source 0x40A374..0x40A3E0: landing code 9, club-12 path, descending vertical force, height <=0x140000 and 0x30000 deflect-vs-stop threshold.
  - Source 0x40A50B..0x40A575: exact flagpole integer X, adjusted Y = ball Y − (height integer >> 1), green-mode exclusion, max height 0xA0000; applies already-recovered seeded original PRNG deflection.
  - Source 0x40A456..0x40A4A8: cup distance zero, height <=0x32 raw, player status <4, horizontal force >=0xD000, original lip rebound mutation. **Predicate only**, player-status dependency must be mapped before enabling it in live physics.
- Source-gated code-9 airborne deflection/stop is now connected to the actual C++ shot loop. The flag impact is exposed through the same native loop **only when a separately verified flag-object presence marker is supplied**; ordinary course-hole coordinates alone cannot activate it. This protection preserves earlier original-PRNG golden-master expectations on synthetic course fixtures.
- Native tests assert threshold boundaries; a deliberately armed flag-object test checks PRNG consumption and path deflection versus a miss. This is not yet a real-course object classification proof.
- **Missing:** original dynamic-code parity proof for those activated branches, tree-object identity/type and ball-height gating, complete PRNG/audio/event control and collision against all original fixtures; complete code-9/code-10 post-terminal continuation; flag/cup corner-case original-state proof and player status; dispatch outside the studied windows.
- **Do not claim tree parity, complete code-9/10, flag/cup full parity, or exhaustive dispatcher parity yet.** Static exhaustive instruction windows are not game-wide semantic coverage.

### 6. Multiplayer — source code mapped, not implemented in parity engine
- Original next-player source 0x40C180..0x40C260: 60 x86 instructions, nine conditional branches. Original per-player structure stride **0x86** bytes; player slots indexed against 0x425FE2 and player-count at 0x429527; alternatives include special holder 0x41F885, qualifying state and comparative player metric.
- Actual Windows code evidence is in the dispatcher atlas. Current `ClassicGameSession` provides a verified **single-player** 18-hole round; it is not original turn switching.
- **Missing:** execute original player selection branch with multiple players and reconstruct actual state/counters, with original-machine golden masters.

### 7. Computer opponents — binary code mapped, not parity-complete
- Source-verified CPU shot planner: 189 instructions, 20 branches, nine calls over 0x40C600..0x40C8CB; original quick swing: 144 instructions, 15 branches, 22 calls over 0x40C8CB..0x40CAE1.
- Previously identified original CPU diagnostics at 0x40C8BF/0x40CACA. No guessed AI has been added.
- **Missing:** exact aim/club/power/accuracy policy, skill-dependent source state, RNG seeding, shot evaluation, course-awareness, CPU retry logic and independent emulator outputs.

### 8. Modes — original menu/resource evidence, not parity-complete
- Original executable contains Tournament, Season, Continue Tournament/Season, named Open/Skins tournament resources. These strings alone do not prove original matchplay, season/cut/tie/leaderboard formulae.
- **Missing:** machine-verified actual mode controllers, bracket/cut/handicap/award/competition state transitions, multiplayer and game-mode saved data.

### 9. Scoring/putts/penalties — native core validated, not fully closed
- Existing source-derived single-player scoring, putter/green slopes, original code-8 cup and water/hazard recovery, **18-hole score progression/replay**, and 256/256 original-green putt oracle comparisons remain preserved.
- Expanded 990-vector original-machine accounting proves **both counters increment once even for a no-flight, out-of-profile accuracy attempt**. The native hole and Godot round adapters now charge that attempt, preserve the stationary ball and re-enable the meter for a new paid stroke; completed no-flight attempts made away from zero cup distance now save and reload with independent core re-execution (manual and original Welly regression verified). The zero-distance case and unverified original special-event continuations remain replay-gated. [Godot bridge CI 38174394126](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38174394126).
- **Missing:** edge cases around cup lip bounce, code 9/10 resume, per-player scoring, all original penalties/modes and original shot-attempt/accounting semantics.


## Additional original-machine recovery in the current pass

These are **new working C++ native subroutines** verified by executing the genuine SHA-256-identified original Windows v1.014 x86:

| Source-original logic | Exact original/native match | C++ / Godot integration | Still needed |
|---|---:|---|---|
| Player selection, eligible/ineligible participants, ties, forced slot `0x40C1B7..0x40C22F` | **1,564 / 1,564** | `original_next_player.cpp`, `PixteeRoundBridge.original_next_player_choice` | Per-player lifecycle, turns, scoreboard, whole game mode |
| CPU difficulty search budget, source `0x40C665..0x40C6B2` | **24 / 24** | `original_cpu_difficulty.cpp`, `PixteeRoundBridge.original_cpu_search_parameters` | Original full CPU shot strategy |
| Computer candidate ranking, source `0x40C78B..0x40C7C4` | **378 / 378** | `original_cpu_candidate.cpp`, `PixteeRoundBridge.original_cpu_candidate_choice` | Candidate generation, score evaluation, original club/aim/power decisions |

**GitHub Actions evidence:** [Original next player](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38162829000), [Original CPU difficulty](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38163091444), [Original CPU candidate scan](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38163324373).

These are actual native C++ algorithms, **not only original addresses**. Neither multiplayer nor computer AI is closed as a complete system.  The full original competitor-turn transition and AI decision paths are still blocked until original-runtime evidence and end-to-end parity testing exist. Existing original art/courses/audio remain excluded.

## Preservation, gates and release
- `main` original Sensible Golf restoration and prior Pixtee branch untouched.
- The native and Godot code lives solely on `feat/pixtee-godot-recovered-core`; draft PR #12 remains unmerged.
- The full requirements inventory `ENGINE_COMPLETENESS_GATE_2026-10-11.json` remains intentionally **red** until complete original-machine end-to-end comparisons.
- Artwork/audio/courses from the original are not shipped in Pixtee. Recovered source-derived numerical data and code require an IP/right-to-distribute review before commercial release.
- **No production playable Android APK has been made or claimed.**

## Collision activation safety correction

The original flag branch compares ball/flag coordinates, but source course collision-object existence is not yet classified. Enabling coordinate collisions for *every* course with a cup would consume extra PRNG values even on synthetic fixture courses and invalidate known tests. The shot API therefore defaults flag-object-verified to **false**; the exact native routine is ready but unverified real-course activation stays gated. The original code-9 conditional branch can execute using a source-confirmed terrain code and original seed. This distinction deliberately avoids claiming full object-collision parity.


## Additional recovered original gameplay from continued source analysis (11 October 2026)

All tests below were performed against the **actual SHA-256-verified Windows v1.014 machine code** and compared against newly implemented C++, not inferred from screenshots or recreated physics.

| Original source routine | Zero-tolerance original-versus-C++ results | Committed code | Scope/remaining blockers |
|---|---:|---|---|
| Computer shot stochastic power/accuracy/curvature deviation: `0x40C7DD..0x40C84F` | **1,620/1,620 exact** | `original_cpu_shot_variation.cpp`, `PixteeRoundBridge.original_cpu_shot_variation` | RNG return values provided explicitly; complete candidate generation, live RNG distribution and shot planner remain OPEN |
| Original multiplayer mode-1 turn/stroke counter correction: `0x40C005..0x40C098` | **3,192/3,192 exact** | `original_turn_score_adjust.cpp`, `PixteeRoundBridge.original_turn_score_adjust` | The actual original branch decrements hole + total stroke counters in qualifying states; this is not full multiplayer or penalties |
| Original mode-4/6 two-side competition turn-slot selection: `0x40C098..0x40C16D` | **1,728/1,728 exact** | `original_competition_turn.cpp`, `PixteeRoundBridge.original_competition_turn_choice` | Correct side, player-slot and previous-slot selection; complete match/season results and copied player state are not yet integrated |
| Original **two-ball-distance-zero state** in the mode-4/6 controller | Observed source branch loops between `0x40C0B8` and `0x40C110` | Native model rejects this invalid/no-progress state explicitly | The original game's outer scheduler/origin of the state is not yet proven; we do not imitate a hang |

Actual machine execution evidence:
- [Computer shot error 1,620/1,620](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38164319868)
- [Multiplayer turn and score adjustment 3,192/3,192](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38164530583)
- [Competition mode4/6 turn-owner branch 1,728/1,728](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38164902939)

**No original gameplay requirement is silently marked COMPLETE solely from a verified subfunction.** The 15 gameplay and 8 deferred presentation full-system gates remain open until original round/game-mode/Android full-session parity passes. The original binary/media are not part of the shipped Pixtee project, and original-source-derived data needs rights review before commercial distribution.

### 11 October continuation — signed terminal terrain event route

Source x86 `0x40A686..0x40A6AB` compared with the new C++ branch classifier: **2,730/2,730 exact** original-machine inputs including signed height boundaries; [authenticated original CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38174782111). Code 50 occurs in GREEN D2/D3/D4 and MUD D1 terrain descriptors and is **not proof of a tree collision**. The branch is intentionally not connected to live course objects; actual tree activation remains a separate red requirement. [Full native CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38174783233) also passed. This work does not change original graphics or Astra's HTML prototype.


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
