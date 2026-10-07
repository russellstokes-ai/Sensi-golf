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

They must not independently calculate golf physics, terrain results, collision, putting, PRNG outcomes, hazards, cup rules or scoring transitions.

## Current checkpoint

Current recovery/integration head: `b9f6aaaca958d5c9479745423c16f3af5a43e461`.

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

### Burst 3 — hole/session state machine

**IN PROGRESS — MAJOR TERMINAL/RECOVERY PATHS INTEGRATED**

Delivered:

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
- explicit unsupported state where a rule is still not integrated.

Key implementation checkpoints:

- `0f83c2f89e0db324d0fbad0304088d26104eb7bd` — putter cup-edge terminal integration;
- `37e2f16cbf1420f4a83b59072f08c0915b68f45e` — hazard pause/position recovery integration.

Key analysis/parity evidence:

- `37616408565` — special-green dispatcher;
- `37616542794` — special landing tail;
- `37616747731` — code 10/50/60 non-putter trajectory parity;
- `37617022417` — terrain profile/variant analysis;
- putter code-8/9/10 parity was added before product integration;
- post-hazard recovery was parity-tested before product integration;
- `37623146366` — scoring/stroke-state analysis;
- `37623430352` — original stroke-counter ownership trace;
- `37623654428` — actual hole-completion semantic check;
- current-head CI `37623654112` — PASS.

#### Recovered score/stroke facts

The original player structure has two counters at `+0x52` and `+0x56`.

Controlled original-machine-code evidence shows:

- both change from 0 -> 1 on both a normal iron launch and a putter launch;
- in the scoped putter terminal oracle, code-8/cup changes initial 7/11 -> 8/12;
- code-9 and code-10 special terminals leave 7/11 unchanged;
- score-transfer code executes `player+0x56 -= player+0x52` and then clears `player+0x52`;
- hole-result flow compares a stroke/score source with the course par and clamps the displayed relative result to the original range -3..+8.

These observations are strong enough to structure the next session work, but the counters will not receive stronger human-readable names until the remaining lifecycle xrefs are closed.

#### Cup capture versus hole completion

The original code separates:

1. ball/cup terminal detection;
2. terminal presentation/event state;
3. score/result calculation;
4. full hole/session completion and next-state flow.

Therefore `ClassicShotOutcome::Holed` currently means the recovered ball/cup outcome. The session still needs a separate completed-hole/scoring transition before it can advance to the next hole.

#### Remaining Burst-3 work

1. finalize session-level ownership of the two recovered shot/score counters;
2. implement score/result state using the original par comparison rules;
3. distinguish cup terminal from completed/scored hole state;
4. recover and implement next-hole transition ownership;
5. move original PRNG seed ownership into the session where needed by full-hole replay;
6. prove one complete original hole from SPT tee through scored completion.

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

A complete original hole can be played deterministically through the platform-neutral game/session layer, with no Android-specific gameplay logic and with unsupported original rules resolved rather than approximated.

Android then becomes the first actual playable product build.
