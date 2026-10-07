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

Current recovery/integration head: `ddbde48870f6a6a02864592825a18d6860fcb02e`.

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

**IN PROGRESS**

Already delivered:

- real SPT tee initialization;
- real SPT cup coordinate exposure;
- tee -> aim/club/input -> shot -> moving terrain lookup -> rest lifecycle;
- stroke counting;
- per-tick terrain refresh from the authoritative ball position;
- hazard terminal state;
- hole terminal state;
- flat-green putter path;
- explicit `UnsupportedTerrain` state instead of fallback/guessed rules.

Current recovery evidence:

- `37616408565` — special-green dispatcher analysis;
- `37616542794` — special landing-tail analysis;
- `37616747731` — zero-tolerance non-putter trajectory parity for landing codes 10, 50 and 60;
- `37617022417` — terrain variant/profile-field xref analysis.

Current rule boundary:

| Landing code | Known role | Product integration |
|---:|---|---|
| 1 | normal GREEN H4 / flat putter path | supported |
| 2..7 | ordinary playable surfaces | supported |
| 8 | cup/hole terminal | supported for recovered non-putter terminal path |
| 9 | special near-hole / PRNG interaction path | interaction arithmetic proven; full putter/session control flow pending |
| 10 | special green terminal/event path for putter; normal-shot trajectory parity proven | putter/session integration pending |
| 35 | immediate-stop hazard family | terminal physics supported; recovery/drop/penalty pending |
| 50 | special green/down-state family; normal-shot trajectory parity proven | putter/state integration pending |
| 60 | mud/down-state family; normal-shot trajectory parity proven | putter/state integration pending |

Remaining Burst-3 work:

1. parity-test and integrate the putter-specific code 8/9/10/50/60 branches;
2. move original PRNG seed ownership into the session;
3. recover hazard recovery/drop/penalty semantics;
4. recover scoring/hole-completion and next-hole transition state;
5. prove a complete original hole through the platform-neutral session layer.

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
