# Project Status

Last updated: **2026-10-09**

This is the canonical current-status document for the Sensible Golf Enhanced recovery/port project. Specialist documents contain deeper technical detail, but this page is the authoritative summary of what is complete, what is parity-proven, and what remains in the active mobile-first integration work.

## Executive status

- **Step 2 — original PC acquisition/inventory: COMPLETE**
- **Gate 1 — original gameplay/physics recovery: COMPLETE / GO**
- **Gate 2 — complete classic game/core integration: IN PROGRESS**
- Gate 1 is formally signed off. Gate 2 is building the platform-neutral classic game/session layer that the mobile product will host. Android remains the first actual playable product target; desktop executables are optional developer/test utilities only.

The project-killing feasibility question has been answered positively: the selected Windows v1.014 gameplay engine is recoverable and the portable core reproduces the original machine code at zero tolerance across the principal shot, terrain, hazard, putting, cup, PRNG and course-collision branches.

The current Gate-2 checkpoint now includes **scored-hole progression, hazard return-to-play, original next-hole resource selection, a platform-neutral game-session controller, and **five** original holes (resources 42, 50, 58, 38 and 70) scored consecutively in one game session with original resource 44 physically loaded next**. Cup capture, scoring, resource loading and next-hole/round state remain separate recovered stages, matching the original program rather than collapsing them into platform UI logic.

## Verified reference build

Windows reference:

- `GOLFWIN.EXE`
- version: **1.014**
- size: **239,616 bytes**
- CRC-32: `23c300b4`
- SHA-256: `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

Companion DOS executable:

- `GOLFDOS.EXE`
- size: **582,895 bytes**
- SHA-256: `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed`

Game archive:

- `GOLF.EPF`
- size: **833,273 bytes**
- SHA-256: `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`
- **277/277 entries extracted**
- original commercial payloads remain outside the public repository.

## Gate 1 parity matrix

| Area | Status | Evidence |
|---|---|---|
| Aim / heading / trig | Parity-verified | 4096-angle direction model and exact Q14 trig |
| Club launch physics | Parity-verified | 13-row original club table |
| Welly-o-meter power path | Recovered | raw 0..105 power path plus timer/dispatcher mapping |
| Accuracy / draw / fade | Parity-verified | live profile selection, power penalty, heading and per-tick curve |
| Airborne flight | Parity-verified | zero-tolerance original-machine-code traces |
| Landing / bounce / roll | Parity-verified | launch-to-rest golden masters |
| Ordinary surfaces | Parity-verified | skirt, fairway, semi rough, rough, very rough, sand |
| Hazards | Parity-verified | water, NO GO and out-of-bounds |
| Putter | Parity-verified | flat-green club-12 path |
| Green slope | Parity-verified | controlled slope vectors |
| Cup/hole capture | Parity-verified | code-8 terminal branch |
| PRNG | Parity-verified | original 16-bit ranged PRNG |
| PRNG shot interactions | Parity-verified fragments | code-9, near-hole lip and flag-coordinate mutations |
| Course collision lookup | Parity-verified | exhaustive MAPI lookup, 81,920 zero-tolerance cases |
| Logical timing source | Recovered | Windows wall clock -> 16.16 timer scheduler |
| Wall-clock cadence | Recovered | interval `0x3A8` 16.16 seconds, about 70.02 Hz |

## Key Gate-1 workflow evidence

Representative successful runs:

- launch-to-rest generic flat: `37523005402`
- ordinary surface parity: `37535123112`
- hazard parity: `37535123148`
- putter parity: `37535123117`
- green slope parity: `37535123104`
- hole capture parity: `37535155917`
- PRNG / interaction parity: `37545660767` and `37545660869`
- exhaustive course collision lookup: `37545810759`
- meter/tick closure: `37546007071`
- final timing/meter dispatch: `37546141205`
- timer source closure: `37546519169`
- end-to-end course interaction: `37603087138`
- consolidated Gate-1 sign-off: `37603222703`
- CI on the sign-off commit: `37603222791`

## Important recovered timing result

The Windows scheduler converts `GetTickCount` milliseconds into 16.16 seconds using:

```
elapsed_fixed = (elapsed_ms << 16) / 1000
```

The gameplay timer callback is registered with interval:

```
0x3A8 = 936
```

in those 16.16-second units, equivalent to roughly **70.02 logical callbacks per second**.

The recovered simulation should therefore preserve original logical update cadence while modern rendering may interpolate independently at 60/90/120 Hz.

## Important collision result

`WOOD1.BIN` through `WOOD4.BIN` were investigated and proved to feed graphics-copy/rendering paths, not the authoritative ball collision logic.

The course collision path is instead driven by the MAPI course lookup. The recovered portable lookup matches original v1.014 machine code over **81,920 cases at zero tolerance**.

Separate post-physics interaction branches use the recovered original PRNG. Their state mutations were parity-tested independently and then joined to the real course/object lookup in the final Gate-1 end-to-end fixture.

## Gate 1 closure

Final sign-off run `37603222703` passed on commit `34bfa1c0999d2e5e196ce900edc9e2bd563ed3d8`.

The run rebuilt the portable core from a clean checkout, reran Python and C++ tests, and reran the complete parity matrix against the verified v1.014 executable/data. It recorded:

- 3 launch-to-rest straight/draw/fade cases;
- 6 ordinary surface cases;
- 3 hazard cases;
- 3 putter cases;
- 3 green-slope cases;
- 1 hole-capture case;
- 6 PRNG sequence samples;
- 3 PRNG-driven interaction branches;
- 81,920 MAPI collision lookup cases;
- the real MAPI -> landing-code 9 -> event 11 -> PRNG interaction integration fixture;
- 20 zero-tolerance comparison files;
- all unit/core tests passing.

Durable evidence:

- `analysis/evidence/gate1_final_signoff.json`
- `analysis/evidence/gate1_course_interaction_parity.json`

**Gate 1 is COMPLETE.**

## Non-gating cleanup

These items may continue after Gate 1 without threatening gameplay fidelity:

- final human-readable naming of every internal club/data index where labels are not independently trustworthy;
- broader DOS-vs-Windows behavioural cross-checks;
- additional course/hole regression fixtures;
- packaging the recovered timing/meter logic behind the future desktop/mobile input layer.

## Active phase — Gate 2

Gate 2 is **Complete classic game/core integration**, not a desktop product.

The target architecture is:

```
parity-proven C++ classic simulation
        ↓
platform-neutral course / hole / round layer
        ↓
Android NDK/JNI host
        ↓
mobile input + rendering + audio
```

### Burst 1 — platform-neutral shot-model bridge: COMPLETE

- production `ClassicShotModel` implements `IClassicModel`;
- original 0..4095 direction units preserved;
- exact scheduler interval `0x3A8` exposed;
- normal shot, putter and hazard terminal paths covered;
- Gate-1 golden masters stayed green.

Primary commit: `8000141a11f8e34d5ad99879a4003507a82552ee`.

### Burst 2 — course/resource model: COMPLETE

- real MAPM/MAPI/SPT bundle loader;
- MAPM dimensions and tile lookup;
- parity-proven MAPI subcell lookup from recovered 16.16 ball coordinates;
- 77 original terrain descriptors available;
- SPT records 0..3 recovered as per-player tee/start coordinates;
- SPT record 4 recovered as cup/hole coordinates;
- original commercial files remain external;
- real v1.014 course-resource validation passes.

Validated checkpoint: `97c89e13ca0f829b9977ff6b887ef7217235d0f8`.

Successful validation evidence:

- Gate-2 course resource validation `37616105293`;
- CI `37616105377`;
- all Gate-1 golden masters green on the same checkpoint.

### Burst 3 — hole/session/round state: IN PROGRESS

#### Integrated

- tee -> shot -> moving terrain lookup -> rest lifecycle;
- stroke counting and recovered player-counter ownership;
- flat-green club-12 path;
- code-8 cup terminal;
- code-9/code-10 special-green terminals;
- original hazard terminal pause of 100 logical ticks;
- safe-anchor tracking and recovered post-hazard position relocation;
- exact original distance-to-hole helper `0x40B998`;
- separate `CupTerminal` and `HoleScored` phases;
- recovered single-player score update;
- recovered next-hole ownership and end-of-round condition;
- cross-hole `ClassicRoundSession` score/progression owner;
- rejection of stale/duplicate hole results;
- 18-hole round progression regression coverage;
- single-player hazard recovery handoff back to `ReadyForShot`;
- original 18-byte hole-order-table resource selection;
- exact `mapmNN.map` / `mapsNN.map` / `mapmNN.spt` requests;
- plan-aware par/resource validation;
- `ClassicGameSession` ownership of resource request -> active hole -> scored-hole commit -> next resource request;
- real original resource 42 bootstrap at tee `(247,726)` with cup `(414,85)`;
- complete real resource-42 hole in 3 shots using recovered physics, score/commit through `ClassicGameSession`, then load original resource-50 MAPM/MAPS/SPT at tee `(401,855)` and cup `(88,97)`.

#### Scored-hole boundary now recovered

The original does **not** equate landing-code 8 / cup capture with the complete scored-hole state.

The recovered sequence is now modeled as:

```
code-8 cup terminal
        ↓
later distance-to-hole pre-update
        ↓
recovered distance value == 0
        ↓
single-player counter correction
        ↓
par / relative-score update
        ↓
HoleScored
        ↓
next-hole routine
```

The portable `ClassicHoleSession` uses optional `ClassicHoleMetadata` containing the zero-based hole index and par. On the currently proven single-player putter path, a cup terminal is promoted on a later tick only when the recovered distance helper returns zero.

#### Recovered single-player score fields

The currently promoted roles are:

- player `+0x52`: current-hole stroke counter;
- player `+0x56`: stroke total used by the recovered score calculation;
- player `+0x58`: cumulative par;
- player `+0x48`: cumulative par minus cumulative strokes;
- player `+0x70`: completed-hole counter.

The code-8 putter terminal has a transient extra increment to `+0x52/+0x56`; the recovered single-player turn-adjust path removes that increment before scoring.

#### Next-hole / round ownership

Original routine `0x40935B` increments the current zero-based hole index. If the incremented value equals **18**, it sets the round-finished state; otherwise it performs next-hole setup.

This ownership is now carried through the portable layer. `ClassicRoundSession` validates that an accepted scored hole has the expected metadata/index and recovered next-hole state before aggregating its score contribution. This prevents stale or duplicate result application.

Merged implementation checkpoint:

- scored-hole integration: `d42a9e1b8c380cd7358c6205b4a1d51d9ce99c7a`;
- round progression PR: **#5**;
- merged round checkpoint: `53c27378cd66c68c6037fbe228a169ab375d2f45`.

#### Verification of the round checkpoint

Final branch checkpoint `9aababcd85b4ca89c0d675351787de0dc7f66a1c` passed every requested verification run:

- push CI: `37672902841`;
- PR CI: `37672942401`;
- Gate 2 course resource validation: `37672902177`;
- Gate 2 hazard recovery parity: `37672902138`;
- Gate 2 putter terminal parity: `37672902332`;
- Golden Master putter parity: `37672902330`;
- Golden Master live player: `37672902586`;
- Golden Master normal surfaces: `37672902230`;
- Golden Master hazards: `37672902392`;
- Golden Master flat landing/rest: `37672902517`;
- Golden Master hole capture: `37672902350`;
- Golden Master green slope: `37672902256`;
- Golden Master PRNG parity: `37672902219`;
- Golden Master PRNG shot interactions: `37672902119`;
- Golden Master course collision lookup: `37672902433`.

**Result: all green; no Gate-1 fidelity regressions.**

Durable evidence is recorded in `analysis/evidence/gate2_burst3_round_checkpoint.json`.

#### Active remaining Burst-3 work

1. close the remaining unsupported terminal cases; code-9/code-10 special-green replay gating is integrated with the recovered 100-tick pause, and zero-distance scored-hole continuation now works for non-putter cup terminals as well as the putter path;
2. feed the session-owned recovered PRNG state into the already parity-proven interaction branches required by complete-hole replay;
3. **COMPLETE (real-course checkpoint):** resource 42 scored from its real tee and original resource 50 actually loaded; continue broadening parity and gameplay coverage rather than claiming all original courses are finished.

New durable checkpoints:
- hazard handoff integration: `31c238565a8f6fa89f1d9fd82e6065e32e2ddcc5`;
- next-hole resource selection merge: `da6cae84ab86e24d9dd3f96a160039e959c989df`;
- platform-neutral game-session merge: `0e03b81a64a621ee82e68f18e729f9e0f4c90693`;
- next-hole evidence: `analysis/evidence/gate2_next_hole_resource_checkpoint.json`;
- controller/bootstrap evidence: `analysis/evidence/gate2_classic_game_session_checkpoint.json`.

The first original order-slot transition is verified as resource 42 (par 4) -> resource 50 (par 4), and the real resource-42 game session bootstraps at SPT tee `(247,726)` with cup `(414,85)`.

Burst 3 remains **IN PROGRESS** until those paths are closed. The mobile host should not begin reimplementing any of them independently.

A desktop executable may be used internally for tests/debugging, but it is not a product milestone.


#### 2026-10-09 real original hole-to-hole proof — PASS

Workflow [37959306670](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37959306670)
passed on commit `aebed46e03403342b42a155c18ab0af8d54b7323`,
with normal CI [37959306617](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37959306617)
also passing (16 C++ portable-core tests).

- First hole: original resource **42**, par 4, real SPT tee `(247,726)`, cup `(414,85)`.
- Solver input sequence: club/power/accuracy/aim `0/105/63/1882`, `2/95/63/1882`, `3/5/63/1756`.
- Original recovered simulation reached scored-hole state in **3 strokes**, and the game session committed the score.
- Original next request: resource **50** (round index 1, par 4).
- **Actual second-hole load** from original `MAPM50.MAP`, `MAPS50.MAP`, `MAPM50.SPT` succeeded; the new active hole is ReadyForShot at SPT tee `(401,855)`, cup `(88,97)`.
- Real MAPS42 diagnostic found **zero landing-code-8 cells**. The original's separately recovered zero-distance pre-update scoring branch must therefore work independently of a code-8 terminal; the portable session now scores a normal resting ball on the next logical tick when the recovered helper returns zero.
- Code-8 putter terminal transient counter correction remains separate, and additional normal-rest and normal-putt tests guard against applying that correction erroneously.
- **No recovered shot physics constants were changed.**

Evidence: `analysis/evidence/gate2_real_hole_full_load_checkpoint.json`.

**Gate 2 is still IN PROGRESS:** this is a passing real-hole pathway, not
proof of every original surface, PRNG interaction, special terminal, or all 18
real holes. Ten individual Gate-1 Golden Master workflows **all passed** at gameplay-code
commit `b1dd77f` (green slope, PRNG, course collision, live player, hazards,
PRNG interactions, hole capture, normal surfaces, putter, flat landing/rest).
Normal CI and 16 core tests passed. A fresh *consolidated* Gate-1 final
sign-off workflow has not been rerun.


### Original second-hole audit (2026-10-09)

A separate real-course analysis workflow attempted **original resource 50**
(par 4; SPT tee `(401,855)`, cup `(88,97)`) and the original following
resource **58** (par 3). The second-hole replay is **NOT yet proven**.

- Latest audit: [37960397473](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37960397473) — **FAIL** (resource 50 not scored).
- Tested original physics with bounded greedy shot selection, severe-rough penalties,
  finer course-distance tie breaking, and explicit exclusion of unresolved
  code-9/code-10 event-11 continuation.
- The latest audit reached recovered hole-distance **135** before getting
  stuck in small nonproductive movements on terrain code 5.
- **Known limitation is in the analysis-only shot planner / incomplete
  special-terminal continuation.** This failure does not establish a bug in
  recovered physics, and the core must not fabricate cup collisions or drops
  to force success.
- The resource-42 -> **loaded resource-50** proof remains green, as does core CI.
- Next proof task: bounded multi-step/beam-search shot planning and original
  event-11 continuation research, followed by original-binary comparison.

Evidence: `analysis/evidence/gate2_second_hole_audit.json`.

The **first real-hole success stands**; Gate 2 remains **IN PROGRESS**.

## Continuous real-original-hole proof — 2026-10-09

**PASS: two original holes, one actual `ClassicGameSession`, cumulative scorecard,
then real third-hole loading.** This supersedes the earlier isolated-hole
proof as the strongest Gate-2 gameplay checkpoint.

| Actual original slot-0 order | Replayed strokes | Par | Verification |
|---|---:|---:|---|
| Resource 42; original tee (247,726), cup (414,85) | 3 | 4 | Scored and committed |
| Resource 50; original tee (401,855), cup (88,97) | 4 | 4 | Scored and committed in the **same** game session |
| Resource 58; original tee (370,383), cup (138,96) | Not played | 3 | Loaded original MAPM/MAPS/SPT, ready for shot |

- Cumulative **7 strokes**, **8 par**, original par-minus-strokes
  score **+1** (one under par); two holes completed, next index 2,
  round not finished.
- Original Windows v1.014 order `42,50,58` and par `4,4,3`
  are independently extracted/validated by the workflow. The two
  known deterministic shot paths use the unchanged recovered simulation.
- New strict continuous-round workflow:
  [37962471601](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37962471601)
  **PASS**; same-commit [CI 37962471478](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37962471478)
  **PASS**.
- The independent original resource-50 solver audit
  [37961865206](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37961865206)
  **PASS**: a bounded spatially diverse multi-shot beam found a four-stroke
  solution and replayed it through the authoritative game session.
  Earlier greedy shot selection stalled at original distance 135.
- After integrating the continuous probe's CMake build registration, the
  10 individual Gate-1 golden-master workflows **all passed** on commit
  `635cf90fb576bf20e4e4dd5b98cbc4d7726ea90d`.
  Transient Internet Archive HTTP 502 and partial-download errors were
  observed in earlier parallel workflow attempts; no original
  physics constants were modified to clear those errors.

Evidence:
`analysis/evidence/gate2_continuous_two_hole_round_checkpoint.json`,
`analysis/evidence/gate2_second_hole_audit.json`.

**Gate 2 is still IN PROGRESS.** The original code-9/code-10 event-11
post-stop continuation has not been fully recovered, the full 18-hole
original round has not been proven, and the Android playable host is
not yet built. The multi-shot solver is a development/audit utility,
not shipping gameplay AI. Never fabricate an original terminal drop or
reposition to force passing tests.

### Three-hole continuous original round — latest checkpoint (2026-10-09)

**PASS: original 42 → 50 → 58 scored in one authoritative
`ClassicGameSession`, then original resource 38 physically loaded.**

- Original hole 42 (par 4): **3 strokes**.
- Original hole 50 (par 4): **4 strokes**.
- Original hole 58 (par 3): **1 stroke** (hole-in-one, recovered
  original physics; club 0, power 105, accuracy 63, aim 2491).
- After three scored holes: **8 total strokes / cumulative par 11**,
  recovered par-minus-strokes **+3** (three under par), next round index 3,
  round incomplete.
- Original fourth hole 38 (par 4) loaded from genuine MAPM/MAPS/SPT,
  actual SPT tee **(310,766)**, cup **(282,154)**; phase
  `ReadyForShot`.
- Strict passing continuous workflow:
  [37963036059](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37963036059).
  Same-commit core
  [CI 37963035978](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37963035978)
  **PASS**.
- Independent hole-58 solver proof:
  [37962765890](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37962765890)
  **PASS**. The existing two-hole continuous workflow remains
  [green 37963002776](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37963002776).
  Ten individual Gate-1 golden-master workflows **all passed** on the
  continuous-fixture extension commit `a40e14b`.

Durable fixture and explicit limitations:
`analysis/evidence/gate2_continuous_three_hole_round_checkpoint.json`.
The original commercial assets remain outside the public repo. This is a
**3/18-hole real-resource integration proof** and not final Gate-2 signoff:
code-9/code-10 event-11 post-stop continuation and complete original-round
behaviour remain outstanding; the Android playable build is not yet started.

### Continuous four-hole original-game checkpoint — 2026-10-09

**PASS — original resource sequence 42 → 50 → 58 → 38 scored
continuously in one `ClassicGameSession`; resource 70 loaded next.**

| Original hole | Par | Strokes |
|---|---:|---:|
| 42 | 4 | 3 |
| 50 | 4 | 4 |
| 58 | 3 | 1 |
| 38 | 4 | 2 |
| **Cumulative** | **15** | **10** |

The recovered par-minus-strokes field is **+5** (five under par).
The round has four accepted completed holes and next original round index
4; it is **not** marked complete. Original fifth course resource **70**
(par 3) is physically loaded from MAPM/MAPS/SPT at tee **(341,367)**
and cup **(88,94)**, ready for a first stroke.

- Four-hole continuous original-resource workflow:
  [37963459155](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37963459155)
  **PASS**; same-commit
  [CI 37963459038](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37963459038)
  **PASS**.
- Independent hole-38 audit:
  [37963249179](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37963249179)
  **PASS** (two recovered shots).
- The retained two- and three-hole continuous regressions both
  **PASS**, and all ten individual Gate-1 golden-master workflows
  **PASS** on the source-extension commit `c464268`.
- Machine-readable exact original shot requests, resource data, cumulative
  score, run IDs and caveats are preserved in
  `analysis/evidence/gate2_continuous_four_hole_round_checkpoint.json`.

**Gate 2 is still IN PROGRESS.** This is 4/18 original holes scored in
a single continuous portable-core round, not a full original-game
completion or playable mobile application. The original code9/code10
event11 post-stop continuation is unproven and must not be approximated.
The recovered physics implementation was not modified to pass this test.

### Reusable five-hole original-round fixture — latest status 2026-10-09

**PASS: five authentic original holes scored consecutively in one
`ClassicGameSession`, sixth original course physically loaded.**

- Original slot-0 order: **42 → 50 → 58 → 38 → 70 → loaded 44**.
- Recovered shot counts: **3, 4, 1, 2, 2**, over original
  pars **4, 4, 3, 4, 3**.
- After five scored holes: **12 cumulative strokes / 18 par**,
  recovered par-minus-strokes **+6** (six under par), no premature
  18-hole completion.
- Following original resource **44** (par 4) physically loaded from
  MAPM/MAPS/SPT, SPT tee **(302,723)**, cup **(148,107)**.
- The new **data-driven** replay harness
  `engine/tools/classic_real_round_fixture_probe.cpp` accepts
  `engine/tests/fixtures/original_round_first_five.txt`. It checks
  each hole's genuine SPT tee/cup, every transition/score update,
  original-par values, and keeps one authoritative game session throughout.
  The executable can accommodate later course fixtures and a full
  original 18-hole round; no more nested bespoke C++ fixture branches
  are required as future shot paths are recovered.
- Strict workflow
  [37964103361](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37964103361)
  **PASS**; same-commit
  [CI 37964103351](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37964103351)
  **PASS**.
- The independent original resource-70 audit
  [37963698000](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37963698000)
  **PASS** and identified the following original resource 44.

Durable record:
`analysis/evidence/gate2_data_driven_five_hole_checkpoint.json`.

**Gate 2 remains IN PROGRESS.** Five of eighteen authentic holes have
been completed continuously. The original code9/code10 event11
post-stop continuation is still unresolved; original multiplayer/full
round parity and the Android playable app are not yet complete.
Commercial original assets are never checked into the public repository.
