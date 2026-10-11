# Pixtee / Original Sensible Golf — nine-area recovery checkpoint
Date: 2026-10-11
Branch: `feat/pixtee-godot-recovered-core`
**Status: INCOMPLETE. Engine/research only; this does NOT mean all original mechanics are recovered.**

User instruction: complete the *actual original Sensible Golf Windows v1.014 gameplay engine*, with graphics postponed, and never invent substitutes for missing mechanics.

## Verified original source and integration work

### 1. Welly-o-Meter — advanced, not fully closed
- Already proven **1,986/1,986** original binary vs C++ per-callback Welly states. Source: `tools/trace_original_welly_machine_code.py`; original-source [CI 38159584005](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38159584005).
- This pass **connected** `OriginalWellyTick` to `PixteeRoundRuntime` with `start_original_welly` and `tick_original_welly`, and exposed both through the Godot `PixteeRoundBridge` along with original power, accuracy, angle and step data.
- The original two-press sequence is adapted to the core's atomic three-phase shot request. It prevents aim/club changes during the source-authenticated sweep and keeps timeout and out-of-profile no-launch from creating fake strokes. Native regression includes a no-input timeout, valid capture and rejected-then-valid retry. [Native CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38161291827).
- **Missing:** verified game-level callback scheduling, click debounce, exact graphical colour zones and original outer retry/penalty UI. Our safe no-stroke fallback is not asserted to be the complete original rejection flow.

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
- This pass adds original per-meter invalid-profile no-launch handling without falsely incrementing a stroke.
- **Missing:** edge cases around cup lip bounce, code 9/10 resume, per-player scoring, all original penalties/modes and original shot-attempt/accounting semantics.

## Preservation, gates and release
- `main` original Sensible Golf restoration and prior Pixtee branch untouched.
- The native and Godot code lives solely on `feat/pixtee-godot-recovered-core`; draft PR #12 remains unmerged.
- The full requirements inventory `ENGINE_COMPLETENESS_GATE_2026-10-11.json` remains intentionally **red** until complete original-machine end-to-end comparisons.
- Artwork/audio/courses from the original are not shipped in Pixtee. Recovered source-derived numerical data and code require an IP/right-to-distribute review before commercial release.
- **No production playable Android APK has been made or claimed.**

## Collision activation safety correction

The original flag branch compares ball/flag coordinates, but source course collision-object existence is not yet classified. Enabling coordinate collisions for *every* course with a cup would consume extra PRNG values even on synthetic fixture courses and invalidate known tests. The shot API therefore defaults flag-object-verified to **false**; the exact native routine is ready but unverified real-course activation stays gated. The original code-9 conditional branch can execute using a source-confirmed terrain code and original seed. This distinction deliberately avoids claiming full object-collision parity.
