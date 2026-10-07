# Gate 2 — Classic Game/Core Integration

Status: **IN PROGRESS**

Gate 2 is not a desktop product milestone. The target product is mobile.

The purpose of this gate is to turn the Gate-1 parity-proven shot engine into a complete, platform-neutral classic game layer that Android can host without reimplementing gameplay rules in Kotlin/Java/UI code.

## Architecture rule

The recovered C++ simulation is authoritative.

Android, future iOS code and any developer desktop test executable are platform shells only. They may:

- submit inputs;
- provide files/assets;
- schedule logical ticks;
- render snapshots;
- play audio;
- persist opaque game/session state.

They must not independently calculate golf physics, terrain results, collision, putting, PRNG outcomes or cup rules.

## Burst plan

### Burst 1 — platform-neutral shot-model bridge

- implement the existing `IClassicModel` contract using Gate-1 recovered code;
- preserve the exact original timer interval `0x3A8` in 16.16-second units;
- expose deterministic shot state to platform shells;
- cover normal shot, putter and hazard termination with tests.

### Burst 2 — course/resource model

- load the required extracted course resources through a clean data interface;
- represent hole tee, target/cup, MAPI banks, surface lookup and object/event data;
- keep commercial payloads outside the public repository;
- add synthetic/open test fixtures plus hash-based validation of licensed/original data.

### Burst 3 — hole/session state machine

- player at tee -> aim/club -> Welly input -> shot -> landing/rest/hazard/hole;
- stroke counting and hole completion;
- deterministic PRNG/session seed ownership;
- transition to next hole.

### Burst 4 — save/replay contract

- deterministic input replay;
- versioned classic save-state schema;
- resume at hole/shot boundaries;
- regression replay through the same C++ model.

### Burst 5 — Android harness

- Android project and NDK/JNI bridge;
- landscape-first shell;
- render a complete original hole from C++ snapshots;
- touch three-click input and aiming;
- gamepad input;
- fixed classic simulation cadence with independent display interpolation.

## Gate 2 exit

A complete original hole can be played through the platform-neutral game/session layer with deterministic replay and no Android-specific gameplay logic.

Android then becomes the first actual playable product build.
