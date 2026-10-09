# Gate 2 — Classic Game/Core Integration

Status: **IN PROGRESS**

Gate 2 is not a desktop product milestone. The target product is mobile, with Android as the first playable product build.

The purpose of this gate is to turn the Gate-1 parity-proven shot engine into a complete, platform-neutral classic game layer that Android can host without reimplementing gameplay rules in Kotlin/Java/UI code.

## Architecture rule

The recovered C++ simulation is authoritative.

Android, future iOS code and any developer desktop test executable are platform shells only. They may:

- submit inputs;
- provide licensed/original files and presentation assets;
- schedule logical ticks;
- render snapshots;
- play audio;
- persist opaque game/session state.

They must not independently calculate golf physics, terrain results, collision, putting, PRNG outcomes, hazards, cup rules, scoring transitions or hole progression.

## Current checkpoint

Current merged integration checkpoint: `0e03b81a64a621ee82e68f18e729f9e0f4c90693`.

This includes the recovered scoring/round layer, hazard handoff, original next-hole resource selection and the platform-neutral `ClassicGameSession` controller.

### Burst 1 — platform-neutral shot-model bridge

**COMPLETE**

Delivered:

- production `ClassicShotModel` implementing `IClassicModel`;
- Gate-1 recovered launch/flight/ground/putter mechanics only;
- exact original direction units;
- exact original timer interval `0x3A8`;
- deterministic `BallState` snapshots;
- normal shot, flat-green putter and hazard terminal tests.

Primary implementation commit: `8000141a11f8e34d5ad99879a4003507a82552ee`.

### Burst 2 — course/resource model

**COMPLETE**

Delivered:

- validated MAPM/MAPI/SPT bundle;
- MAPM dimensions and 10-bit tile references;
- Gate-1 parity-proven MAPI subcell lookup;
- exact coordinate decomposition from the moving 16.16 ball state;
- all 77 original terrain descriptors with original names, landing code, profile slot and variant;
- SPT records 0..3 recovered as player tee/start coordinates;
- SPT record 4 recovered as the cup/hole coordinate;
- raw access retained for SPT fields whose semantics remain unproven;
- synthetic malformed-input tests;
- real original-course validation without committing commercial files.

Validated checkpoint: `97c89e13ca0f829b9977ff6b887ef7217235d0f8`.

Evidence:

- resource validation `37616105293` — PASS;
- CI `37616105377` — PASS;
- all existing Gate-1 golden masters — PASS.

### Burst 3 — hole/session/round state machine

**IN PROGRESS — REAL HOLE COMPLETED; REMAINING CLASSIC-RULE COVERAGE**

#### Delivered

- real SPT tee initialization and cup coordinates;
- tee -> aim/club/input -> shot -> moving terrain lookup -> rest lifecycle;
- stroke counting at the portable session layer;
- per-tick terrain refresh from authoritative ball position;
- normal rest and continuation;
- flat-green putter path;
- code-8 putter cup terminal;
- code-9 and code-10 putter special-green terminals;
- explicit shot outcomes: rest / hazard / holed / special-green-stop;
- original hazard pause of 100 logical ticks;
- recovered safe-anchor tracking;
- recovered post-hazard position relocation using original course extents;
- exact recovered distance-to-hole helper;
- explicit separation between cup terminal and scored-hole state;
- recovered single-player par-relative score update;
- recovered zero-based next-hole increment and 18-hole finish rule;
- cross-hole `ClassicRoundSession` state;
- stale/duplicate scored-hole rejection;
- 18-hole synthetic round progression regression;
- hazard recovery acknowledgement back to playable state;
- original order-table-driven next-hole resource requests;
- exact `mapmNN.map`, `mapsNN.map`, `mapmNN.spt` filename templates;
- plan-aware par/resource validation;
- `ClassicGameSession` round/hole orchestration;
- real original resource-42 bootstrap through the controller.

Key implementation checkpoints:

- `0f83c2f89e0db324d0fbad0304088d26104eb7bd` — putter cup-edge terminal integration;
- `37e2f16cbf1420f4a83b59072f08c0915b68f45e` — hazard pause/position recovery integration;
- `d42a9e1b8c380cd7358c6205b4a1d51d9ce99c7a` — scored-hole integration;
- `53c27378cd66c68c6037fbe228a169ab375d2f45` — merged cross-hole round progression.

#### Recovered score/stroke facts

For the currently proven single-player path:

- `+0x52` is the current-hole stroke counter;
- `+0x56` is the stroke total used by the original score update;
- `+0x58` accumulates par;
- `+0x48` is `cumulative par - cumulative strokes`;
- `+0x70` counts completed holes;
- a code-8 putter terminal transiently increments `+0x52/+0x56` again;
- the recovered single-player turn-adjust path removes that transient increment before the score update.

The portable per-hole session starts its recovered counters from zero. `ClassicRoundSession` owns the cross-hole aggregate that must survive between physical hole sessions.

#### Cup capture versus scored-hole completion

The original flow is now represented as separate stages:

1. code-8 cup terminal detection;
2. terminal/presentation state;
3. later distance-to-hole pre-update;
4. scored-hole counter/par update;
5. next-hole progression.

The exact Windows v1.014 distance helper at `0x40B998` has been recovered and parity-tested. A cup terminal is not promoted merely because landing code 8 occurred; the currently proven single-player putter completion path requires the later recovered distance value to be zero.

`ClassicHoleMetadata` supplies the zero-based hole index and par to the portable hole session. The session exposes the recovered next-hole index and round-complete result.

#### Cross-hole round state

`ClassicRoundSession` now:

- accepts only `HoleScored` results;
- validates that hole metadata matches the current round index;
- validates the recovered next-hole index;
- validates the recovered 18-hole completion state;
- accumulates strokes/par and relative-to-par using original-width arithmetic;
- rejects stale/duplicate results;
- preserves the round state across separate physical hole-session instances.

The regression suite walks all 18 holes and verifies that the round becomes complete only when the zero-based hole index reaches 18.

#### Verification

Checkpoint `9aababcd85b4ca89c0d675351787de0dc7f66a1c`:

- CI push `37672902841` — PASS;
- CI PR `37672942401` — PASS;
- course resource validation `37672902177` — PASS;
- hazard recovery parity `37672902138` — PASS;
- putter terminal parity `37672902332` — PASS;
- putter golden master `37672902330` — PASS;
- live-player golden master `37672902586` — PASS;
- normal-surface golden master `37672902230` — PASS;
- hazard golden master `37672902392` — PASS;
- flat landing/rest golden master `37672902517` — PASS;
- hole-capture golden master `37672902350` — PASS;
- green-slope golden master `37672902256` — PASS;
- PRNG golden master `37672902219` — PASS;
- PRNG-interaction golden master `37672902119` — PASS;
- course-collision golden master `37672902433` — PASS.

No Gate-1 parity regression was introduced.

Durable record: `analysis/evidence/gate2_burst3_round_checkpoint.json`.

#### Remaining Burst-3 work

1. **Remaining terminal continuations:** close unsupported/special-green continuations and any non-putter scored-hole completion path required by the original.
2. **Session PRNG integration:** the game session now owns an optional captured original seed/state across hole transitions and restores that deterministic baseline on reset; next feed it into the parity-proven interaction branches required by complete-hole replay.
3. **Real-course end-to-end proof: PASS.** Resource 42 has been scored from the real tee in 3 recovered shots, committed through `ClassicGameSession`, and followed by successful loading of resource 50 from original MAPM/MAPS/SPT (not merely a filename request).

Recent merged checkpoints:
- `31c238565a8f6fa89f1d9fd82e6065e32e2ddcc5` — single-player hazard handoff;
- `da6cae84ab86e24d9dd3f96a160039e959c989df` — original next-hole resource selection;
- `0e03b81a64a621ee82e68f18e729f9e0f4c90693` — platform-neutral game-session controller.

Real-resource evidence:
- workflow `37686556999` proves original slot-0 resource transition 42 -> 50;
- workflow `37687404791` proves resource 42/par 4 bootstraps through the controller at tee `(247,726)`, cup `(414,85)`, ready for input;
- all controller/core and Gate-1 golden-master checks remained green.

### Burst 4 — save/replay contract

**NOT STARTED**

Deliverables:

- deterministic input replay;
- versioned classic save-state schema;
- resume at stable shot/hole boundaries;
- regression replay through the same C++ session model.

### Burst 5 — Android harness

**NOT STARTED**

Deliverables:

- Android project and NDK/JNI bridge;
- landscape-first shell;
- original-course rendering from C++ snapshots;
- touch aiming and three-click Welly input;
- gamepad support;
- fixed classic simulation cadence with independent display interpolation;
- complete original hole on Android.

## Gate 2 exit

Gate 2 exits only when a complete **real original hole** can be played deterministically through the platform-neutral game/session layer, including terminal, scoring and next-hole handoff, with no Android-specific gameplay logic and with unsupported original rules resolved rather than approximated.

Android then becomes the first actual playable product build.


### 2026-10-09 reproducible real-course checkpoint

The analysis-only real-hole probe `sensigolf_real_hole_solver_probe` and
`.github/workflows/gate2-real-hole-end-to-end.yml` now exercise:
original resource 42 tee -> three physics-driven shots -> recovered
zero-distance pre-update scoring -> `ClassicGameSession` score commit ->
original resource 50 request -> **real resource 50 MAPM/MAPS/SPT load**.

- First hole: original resource 42, par 4, tee `(247,726)`, cup `(414,85)`.
- First hole: **3 strokes** (club/power/accuracy/aim:
  `0/105/63/1882`, `2/95/63/1882`, `3/5/63/1756`).
- Second hole: actual original resource 50, par 4, tee `(401,855)`,
  cup `(88,97)`, phase `ReadyForShot`.
- Gate 2 workflow **37959306670: PASS**; CI **37959306617: PASS**
  (16 portable-core C++ tests). Checkpoint commit `aebed46e`.
- Analysis discovered no landing-code-8 cells in the actual resource-42
  MAPS terrain bank. Original zero-distance scoring and code-8 terminal are
  *distinct* recovered routes; the core now checks the original zero-distance
  branch on the following idle tick for a normal resting shot.
- The extra transient putter/code-8 counter correction is applied only for
  that terminal path, not for a normal resting putt.
- The recovered physics constants and tables remain unchanged.

Canonical machine-readable record:
`analysis/evidence/gate2_real_hole_full_load_checkpoint.json`.

This is **not complete Gate-2 exit**: full legacy interaction coverage,
all remaining terminal/surface rules, full original 18-hole replay and a
a fresh consolidated Gate-1 final sign-off remain necessary before universal
original-game fidelity can be claimed. The ten individually triggered Gate-1
Golden Master workflows all passed at the gameplay-code commit `b1dd77f`. Android is still the first planned
playable product target; the C++ solver remains a test utility.


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
