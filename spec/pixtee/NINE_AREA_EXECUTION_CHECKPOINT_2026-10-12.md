# Pixtee – 9-area consecutive recovery execution checkpoint
Date: 2026-10-12
Branch: `feat/pixtee-godot-recovered-core`; draft PR #12.
**Status: IMPORTANT NEW CODE AND PROOFS, BUT NINE AREAS NOT ALL COMPLETE.**
User directive: continue all nine workstreams in chat; do not change the canonical approved art or Astra's three-hole HTML preview.

| Original gameplay area | Work implemented this pass | Authenticated original proof | Full parity still missing |
|---|---|---|---|
| 1. Welly meter timing/input | Original source first-press `0x407620` startup (accumulator **8**, saved-angle **0x40**) used by live `PixteeRoundRuntime`; native meter keeps earlier isolated producer conditions for regression | **3,324/3,324** outer original stage dispatcher x86 comparisons: [run 38206789435](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38206789435) | Real OS input polling/wall-clock cadence, display colours, outer click ownership across original UI |
| 2. Trees and object collisions | Traced MAPM upper-bit distribution, source xrefs, and flag/object globals without guessing tree geometry; no generic circular colliders enabled | Authentic first-hole MAPM has **4/3,876** upper-bit cells, source scanner found **39** resource xrefs and seven MAPI lookup callers ([run 38183207814](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38183207814)). Additional original flag/tree xrefs [run 38206541819](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38206541819) | Actual trees' course placement, mask and 3D height/deflection physics |
| 3. Flagpole collision | Recovered original unconditional flag coordinate loader from fifth SPT record `0x409F39..0x409F64` (globals `0x41F3B2/B4`, marker `0x41F3B0=3`). Added source flag setup module. Native hole and Godot now accept explicitly **verified authored flag object** presence, enabling previously recovered source flag deflection. Prevented replay claims until object manifest becomes part of journal | **3,112/3,112** original Windows source SPT setup matches: [run 38207010331](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38207010331); existing `0x40A50B` collision predicate 2,712/2,712 original | Original sprite/object presence map on imported real courses and complete full-shot flag deflection parity |
| 4. Code8 cup and code9/10 greens | Exposed source code8 cup event `0x40CD25`; code9 early-stop adds one stroke to both original 16-bit counters, direct code10 doesn't; special green event explicitly differentiated at host | **1,280/1,280** original code8 cup event [run 38206131628](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38206131628); **4,860/4,860** early code9 vs late code10 states [run 38183067005](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38183067005) | Complete original post-pause transition and cup lip/flag interaction in full shots |
| 5. Misc event dispatcher | Native Godot snapshot carries original OOB request, cup code8 display message `0x1000F`, special green event request, with event cause and safe off-map surface handling | Original OOB display source `0x40CD00`: **1,024/1,024** [run 38182823777](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38182823777); cup above | Audio/sprite event scheduling, exact original event completion & host UI dismissal |
| 6. Multiplayer | Added `OriginalTurnOrchestrator` combining authentic source original ordinary next-player and mode4/6 side selection; explicit handoff/ack and rejection of impossible source owner. Added `PixteeMultiRoundRuntime` with distinct native shot, round, PRNG, replay state for 1–4 participants, and tests for isolated scoring | Both underlying source routines have prior original x86 machine proofs; turn + multi-round integration [run 38205921357](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38205921357) | Original shared hole/turn scheduler, exact multi-player match lifecycle, scoreboards and rule controllers |
| 7. Computer opponents | `OriginalCpuScanPipeline` composes source difficulty budget, original 24-entry angular scan table, heading schedule, candidate selection and sampled swing variation. Cannot invent missing candidate metrics; source retry must be explicit. Native pipeline tests green | Original source-verified constituent original CPU oracles preserved; [integration run 38205921357](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38205921357) | Actual candidate trajectory evaluation and generation, RNG ownership, club/aim/power selection and full CPU shots |
| 8. Game modes and competitions | Source original mode4/6 side-turn controller wired through turn orchestration; per-player independent native sessions usable. Nonverified season/tournament outcomes deliberately not faked | Original competition selection prior proof 1,728/1,728 and turn selector 1,564/1,564; new integration run above | Season/tournament bracket, qualifying rounds, ties, cuts, awards and persistence |
| 9. Putts, strokes, penalties | World OOB detection and safe anchor/source event (separate from named OOB terrain); source extra +1 counters, corrected code9 vs code10 penalty; independent player score/journal integrity in multi-session tests | World OOB **9,426/9,426** original-machine state cases across 2 source stages [run 38181667415](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38181667415), [run 38182134331](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38182134331); code9 above | Full mode-specific penalties, all original replay/event settlement rules, complete scoring under tournament and multiplayer |

## Acceptance and protected scope

- **All original full-gameplay acceptance gates remain blocked/red** until end-to-end source and actual Godot player-session parity is proven. Unit or original-machine subroutine parity does NOT close a feature. Some newly integrated branch changes await latest full CI/Godot run; report failures and fix rather than claim release.
- No copyrighted original executable, raw course, visual asset or audio is committed. Original SHA-256-verified x86 and course payloads are loaded only inside transient GitHub Actions environments.
- Do not alter original `main`, canonical approved Pixtee sprites/asset pack, or the independent Astra HTML design preview.
- Real blockers are source-original **tree identity and height physics**, complete shot/object scheduler, original CPU candidate evaluation, exact input/visual timer, original competition/game-mode policy and end-to-end game-state/event lifecycle.
- Next actual recovery stage should start from original source tree data and original second CPU simulation caller `0x40CB00..`, then integrate course flag/collision data with the authored Pixtee object map and replay journal.

## Newly implemented entry points

`pixtee-godot/native/src/original_turn_orchestrator.hpp`, `original_cpu_scan_pipeline.hpp`, `pixtee_multi_round_runtime.hpp`;
`engine/src/recovered_flag_setup.cpp`, `engine/src/classic_hole_session.cpp`, `engine/src/recovered_world_boundary.cpp`;
`pixtee-godot/native/src/recovered_welly_tick.cpp`, `pixtee_round_runtime.cpp`, `pixtee_round_bridge.cpp`.

Original-machine workflows and native nine-area test suite live in `.github/workflows/`.


## 12 October — original CPU preflight, green multi-object admission and animation-script progression

- **Source multi-object GREEN map rectangle gate `0x40C35B..`:** **4,740/4,740 exact original Windows x86 comparisons** of all-in/all-out routes `0x40C3F0` vs `0x40C40D` on 1–6 source scene objects, ordinary world and expanded green coordinates, signed 16-bit edges and wrapping origins. [Original x86 evidence](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38226901152). Source logic now drives actual `ClassicGreenRegion::contains_course` / `contains_green` helpers, rather than an approximated 32-bit coordinate expression. [Native engine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38227042294), [Godot smoke](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38227036816) passed. **Full source all-object scene-loop activation is still not integrated** because the original per-frame object catalog is not yet supplied to the host. This discovery is a *green-scene region gate*, **not a recovered tree hitbox**.
- **Original CPU candidate preflight `0x40CB40..0x40CB6C`:** **9,390/9,390 exact source x86 comparisons** across original descending/low-height code9 termination, source club-12 putter zero/positive momentum, and nonputter entry into gravity. Source route targets `0x40CB6C`, `0x40CBCD`, `0x40CC71`, `0x40CC49`. [Machine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38227518922). Native `original_cpu_trial_route_after_lookup` composes prior original CPU power/force setup `0x40CCA4`, original heading table, this source gate and previous original gravity/lip contact. Explicit input `source_terrain_lookup_complete` is required; **no guessed landing-code, trajectory scoring, candidate selection or CPU full-shot claim**. [Integration CI passed](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38227746507).
- **Original ten-slot animation dispatcher:** Source `0x40C539..0x40C559` ordinary nonnegative script frame decoder **5,852/5,852** exact original x86 comparisons: frame word, delay, source frame-offset `+2` and preservation of other slot bytes. [Machine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38228126497). Combined **10-slot dispatcher `0x40C50B..0x40C52C`** correctly performs countdown or script decode on each active slot: **1,500/1,500** mixed original-machine source cases, **75,000 checked original slot fields**. [Mixed machine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38228843162). `ClassicHoleSession::step_original_animation_plain_scripts` now advances real queued code9 reactions with explicitly authored asset script bytes. [Engine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38229076312), [Godot smoke](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38229074417) green.
- **Original animation negative script opcodes `0x40C55A` remain unsupported.** They dynamically jump through `0x41F89C`; a distinct numeric command-target research workflow `.github/workflows/pixtee-original-animation-opcodes.yml` has been started. Native deliberately throws on negative opcode rather than hallucinating callbacks, SFX or asset rendering.
- Original source executable/course resources remain ephemeral CI-only. No accepted Pixtee art or Astra HTML previews changed. All **15 high-level gameplay signoff gates remain blocked/red** until authentic end-to-end source and Godot gameplay evidence proves them.

## Follow-on original animation opcode investigation — 12 October 2026

The original animation event dispatcher is now an explicit source-recovery
workstream. Earlier code includes the x86-authenticated 10-slot enqueue,
timer countdown and ordinary nonnegative script-frame reader.
These do **not** prove signed/negative script commands, actual sound playback,
sprite rendering, host callback dismissal or whole gameplay event completion.

Added research-only source tooling:
- `tools/trace_original_animation_negative_commands.py`: original
  SHA-verified signed-command branch disassembly/control-flow summary in
  the range `0x40C55A..0x40C5F0`; no original binary/graphics committed.
- `tools/probe_original_animation_negative_routes.py`: original Unicorn
  x86 source execution for 15 representative negative script WORDs, stops
  before invoking unmodeled host effects, captures first exit, event-slot
  mutations and executed source addresses as **observations**, not claims
  about native C++ parity.
- `.github/workflows/pixtee-original-animation-negative-research.yml`:
  transient source-executable ingestion and private downloadable
  source-analysis JSON artifacts. The job must be reviewed before its
  results count as verified. Research alone does not close any of the
  15 engine gameplay gates.

**Next functional acceptance:** classify exact signed opcode operands,
side effects and source callback ordering; implement only independently
verified native command routes with event/replay state tests. Continue
actual tree-geometry discovery, original CPU shot evaluation, host
acknowledgement and multiplayer turn settlement separately. Keep the PR
DRAFT and canonical Pixtee sprites/Astra HTML untouched.


### Original animation source command table and lifecycle completion (12 October)

- Authenticated original source Windows v1.014 command table at `0x41F89C`: opcode `0x8000` points at `0x40C56A` external callback; opcode `0x8004` points at `0x40C573` slot release; opcode `0x8008` points at `0x40C58D` wait-duration loader. Source jump-table audit [CI 38229176857](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38229176857). Arbitrary table offsets after these entries are **not established command IDs**.
- Original x86 **5,650/5,650 exact comparisons** for source 0x8004 release (status 0xFFFF, pointer/offset/ticks reset) and 0x8008 wait (reads next WORD delay and advances frame offset a total of 4 bytes), preserving other animation slot fields. [Source command CI 38229701187](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38229701187). Native `original_animation_decode_supported_step` supports only nonnegative frames, 0x8004 and 0x8008. Unrecovered 0x8000 dynamic callbacks and unknown script words **still throw without modifying slot state**; nothing fabricated.
- The original `ClassicHoleSession` code9 reaction now has a native integration regression that executes an authored source-format **frame → wait → release** script through original ten-slot slot/timer machinery; original sprite/audio rendering and real scene animation side effects are not yet implemented. Follow-up native/Godot CI was queued as of this documentation write.
- All full-gameplay acceptance gates remain BLOCKED. The recovered source opcode interpreter is only a **three-action whitelisted subset**, not a full animation event system.

### Host multi-player ownership safety (not original scheduler parity)

`PixteeMultiRoundRuntime::propose_original_turn` now refuses to transfer
ownership while the current original hole session is in `ShotActive`,
`HazardStopped`, `HazardRecovered`, `SpecialGreenStopped`,
`CupTerminal` or `UnsupportedTerrain`. The small constexpr helper
`host_turn_handoff_phase_allowed` and exhaustive phase-table assertions
are tested in `pixtee_multi_round_runtime_tests.cpp`. The host may
request a new original turn only when `ReadyForShot` or `HoleScored`.
This prevents bypassing an unsettled OOB/hazard/special event or live ball
flight; it is a protective integration rule **not** an assertion that
the original Windows UI used precisely this predicate. Preserve the
source-original next-player chooser, and leave the full original shared
event/turn scheduler parity release gate OPEN. Native/Godot CI on this
new commit must pass before this guard is called verified.

### Source x86 original script callback 0x8000 — recovered host handoff

The Windows v1.014 script command `0x8000` dispatches through the original
jump table `0x41F89C` to `0x40C56A`, where it preserves CPU registers,
calls the DWORD function pointer *embedded in the animation script*, restores
registers, and advances the script cursor by four more bytes after the
original shared two-byte command increment. The slot countdown is the
source script-header delay. The actual callback functions and their game
side-effects have **not** been recovered.

- Added `original_animation_prepare_callback` and
  `original_animation_finalize_callback` to the portable recovered event
  module. C++ reads an **opaque token**, never executes a serialized machine
  address, and applies the source post-callback state only after an
  independently authored host action acknowledges it.
- Native tests ensure stale acknowledgements cannot advance a slot twice.
  The live script tick intentionally still refuses opcode `0x8000` until
  an actual host callback route is implemented and the real effects are
  independently verified.
- A transient Unicorn x86 source test invokes a harmless synthetic RET
  function pointer rather than original sound/UI/game callbacks, then
  compares exact event slot fields to the native handoff for **3,576/3,576**
  cases. [Original machine CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38235564193).
  All original executable bytes remain CI-private and are never committed.

**Status:** isolated command-dispatch/slot parity verified; callback side
effects, whole event scheduler, scene/object activation, original AI,
multiplayer and the 15 end-to-end gameplay signoffs remain BLOCKED. Preserve
draft PR and approved art/HTML.

### Original CPU evaluator's first two authenticated source stages

The original Windows 1.014 CPU evaluation machine function
`0x40C93E..` has now been isolated beyond the existing shot-heading,
trial force, contact and candidate-rank components:

1. `0x40C93E..0x40C968`: source initializes consecutive **14-byte**
   candidate score records at `0x41FE89`, increments the record pointer,
   and repeats according to wrapping signed 16-bit DX/JNS logic using
   original globals `0x41FA0F` and `0x41FAD3`; it then resets the
   original swing-adjuster DWORD. C++ module
   `original_cpu_score_scratch.{hpp,cpp}` matches **1,750/1,750**
   authentic x86 source-machine cases.
2. `0x40CA5A..0x40CA8E/0x40CAC4`: original CPU evaluator scans
   consecutive 14-byte score records and finds the first with BOTH signed
   status WORDs `[record+0]` and `[record+2]` nonnegative. Rejected
   entries advance 16-bit BX, DX, CX and signed `JL` compares CX to
   the source span. Portable C++ matches **2,700/2,700** authentic x86
   cases including first-valid and no-valid exits.

[Original source and native CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38237284636).
The original executable and game resources stay transient and absent
from the repository. Both native helpers fail explicitly for unsupported
unbounded scratch loops or truncated buffers. No fabricated candidate
quality input, winner choice, original shot execution or strategy is
introduced. The original evaluator's intervening calls
`0x40CCA4`, `0x40CAE1`, `0x40CB00`, `0x4094F0`, `0x40B998`
and the downstream actual score-selection rules remain independently
gated. **No full CPU-opponent or game-mode signoff yet.**

### Verified source animation-command table and full ten-slot reset

SHA-authenticated original source pointer table `0x41F89C` has three
valid event script entries: `0x8000` callback at `0x40C56A`,
`0x8004` release at `0x40C573`, and `0x8008` wait at
`0x40C58D`. The following DWORD entry is `0x00000004`, not a
valid executable command target. Unknown signed words remain blocked;
do **not** infer further animation commands. [Original table research](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38235821419).

The source-original global ten-event queue reset `0x40C5C0..0x40C5E8`
fills **10×20-byte slots** with `status=0xFFFF` and zeros every
remaining field. New native `original_animation_reset_all` is used by
`ClassicHoleSession::reset`. Exact original x86 versus native parity
is **1,300/1,300 cases**, **104,000 individual slot field
comparisons**. [Original reset CI](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38236194547).

These are authentic source subroutines, **not** proof of host callback
effects, presentation timing, complete shared shot/event dispatcher or
tree collision. Keep every full-gameplay release gate OPEN.

### CPU evaluator score winner (third authenticated stage)

Original Windows `0x40CA8E..0x40CAE0`, after the preceding
`0x40CA5A` eligibility gate, uses the first record's signed
`[record+0xC]` score as incumbent. It scans remaining original
14-byte records, excludes either-negative-status entries, and
replaces the winner **only** when a later record's signed score is
strictly lower. Equal scores preserve the earlier winner. A fully
ineligible collection returns the base record pointer without a valid
winner; new mobile API represents this as `found=false`.

- Native `original_cpu_select_best_scored_record` requires caller-
  supplied source-format score data; it never generates fabricated
  candidate quality values and rejects unsafe input sizes.
- **2,900 / 2,900 exact authentic source x86 comparisons** covering
  signed negative/wrapping scores, noneligible records, first-vs-
  subsequent record outcomes, stable ties and no-winner returns.
  [Original-source verification](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38237924678).
- All three newly recovered CPU evaluator stages from this
  continuation total **7,350 / 7,350** independently compared x86
  cases. These are source-scoped test vectors, not a complete AI
  opponent match.

This evaluator selection at `0x40CA5A..` is not conflated with
the separately authenticated CPU candidate chooser at
`0x40C78B..`, which has different original first-candidate rules.
The original score-creation subprocess and completed CPU shot
planning/execution remain blocked. Do not activate a presumed score
source or close full-player/game-mode parity.


### Further 12 October continuation — authentic computer-opponent scoring now chained

Original Windows v1.014 evaluator source `0x40C9FD..0x40CA1E` was executed against recovered native C++ with **8,324/8,324** exact independent comparisons across original contact-code indexing, 16-bit distance, signed overflow, and the **green mode exactly 1** branch. Source proof: [GitHub CI 38240947386](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38240947386). The original 32-WORD static penalty table in unmodified source executable data at `0x41FADF` was also recovered, and both its contents and original x86 scoring were proved in **5,228/5,228** additional comparisons: [GitHub CI 38241812948](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38241812948). **Combined new actual original-machine scoring parity: 13,552/13,552.**

Native modules `pixtee-godot/native/src/original_cpu_quality_score.cpp`, `original_cpu_score_scratch.cpp`, `original_cpu_scan_pipeline.cpp` now form the source-correct chain from **caller-provided authentic outcome/distance** -> real reference terrain penalty score -> unchanged 14-byte record status/other fields -> signed best-score scan -> exact original candidate heading. An explicit original evaluation guard rejects invented scores; invalid source indices or incomplete buffers throw *before mutation*. Original negative scores and equal-score earliest-candidate retention are unit tested; latest native nine-area integration [CI 38241158149](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38241158149) is green. Godot native interface now exposes both `original_cpu_quality_score`, built-in `original_cpu_reference_penalties` and evidence-gated `original_cpu_scored_record_choice`; extension ABI build and headless tests are separate final checks (do not assert green without CI).

**Important remaining AI blocker:** The game still needs exact original trajectory/candidate outcome simulation, source mode-specific table initialization/possible table rewrites, club and power selection, source RNG ownership, source object/tree impacts and full match scheduling. A statically authenticated original numeric penalty table is not proof it is unchanged throughout all game modes. **Do not mark the computer opponents or any original full-gameplay acceptance gate green.** No copyrighted original source binary, course map, art or audio was committed.
