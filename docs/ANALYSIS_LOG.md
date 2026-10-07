# Analysis Log

This is a durable chronological record. For the latest status, use [PROJECT_STATUS.md](PROJECT_STATUS.md).

## 2026-10-05 — Repository bootstrap

### Verified

- Recovery is a hard gate before enhancement work.
- Public repository excludes original commercial binaries/assets.

## 2026-10-05 — External reference preparation

### Verified

- Internet Archive preservation item identified.
- TDC reference metadata recorded for independent comparison.
- Original manual/behaviour notes captured as behavioural constraints rather than physics formulas.

## 2026-10-06 — Original PC payload ingested

### Verified reference

- Windows `GOLFWIN.EXE` v1.014 identified and fingerprinted.
- DOS companion executable identified.
- `GOLF.EPF` extracted successfully: **277/277 entries**.
- Original commercial payload remains outside the public repository.

## 2026-10-06 — Static physics recovery

### Verified

Recovered from Windows v1.014:

- 44-byte ball state;
- fixed-point X/Y;
- 4096-step heading;
- exact Q14 sine table;
- 13 club physics rows;
- live launch path;
- Welly raw power range;
- 11 swing/accuracy profiles;
- gravity, drag, bounce and roll;
- green/slope data;
- terrain descriptor structure;
- distance-to-hole path.

## 2026-10-06 — Zero-tolerance live shot parity

### Parity-verified

Run `37523005402` matched three original live-player shots from launch through final rest:

- straight 1W: 153 samples;
- draw mid: 131 samples;
- fade high: 111 samples.

All compared state fields and landing/rest events matched exactly.

The exercise exposed and fixed the original one-tick H-drag zero-crossing quirk.

## 2026-10-06 — Terrain and terminal branches

### Parity-verified

The golden-master suite was expanded to:

- skirt;
- fairway;
- semi rough;
- rough;
- very rough;
- sand;
- water;
- NO GO;
- out-of-bounds;
- club-12 putter;
- green slope;
- code-8 hole/cup capture.

Representative successful runs are listed in [PROJECT_STATUS.md](PROJECT_STATUS.md).

## 2026-10-06 — Collision-path investigation

### Verified

`WOOD1.BIN` through `WOOD4.BIN` are used by graphics-copy/rendering code and are not the authoritative ball collision data path.

The actual course lookup is the MAPI path.

The portable MAPI implementation was compared with original v1.014 machine code across four MAPI bank pairs and **81,920 cases** at zero tolerance.

Successful workflow: `37545810759`.

## 2026-10-06 — PRNG and interaction recovery

### Verified / parity-verified

The internal 16-bit ranged PRNG was recovered and implemented independently.

Original-machine-code parity now covers:

- PRNG output/seed evolution;
- code-9 low-height deflection;
- near-hole lip deflection;
- flag-coordinate deflection.

The mutation fragments are exact. Their final lookup-to-activation integration is the remaining Gate 1 end-to-end fixture.

## 2026-10-06 — Welly/timing closure

### Verified

The meter tail and dispatcher were traced through the platform timer system.

The Windows scheduler obtains elapsed time from `GetTickCount`, converts milliseconds to 16.16 seconds using:

```
(elapsed_ms << 16) / 1000
```

and dispatches timer callbacks.

The gameplay callback interval is `0x3A8` fixed units, approximately **70.02 Hz**.

Successful analysis runs:

- meter/tick closure: `37546007071`
- timing/meter dispatcher: `37546141205`
- timer source closure: `37546519169`

## 2026-10-07 — Current checkpoint

### Status

Gate 1 is **GO and ready for final sign-off**.

The major gameplay recovery risks are no longer open unknowns. Remaining work is consolidation:

1. one end-to-end real course lookup -> interaction activation -> PRNG mutation trace;
2. one full current-head sign-off run across all existing parity/core suites;
3. freeze evidence documentation and close Issue #1.

Do not begin enhancement physics. Classic mode remains defined by the parity-proven original behaviour.


## 2026-10-07 — Gate 1 formally closed

### Parity-verified / sign-off

The missing integration fixture passed in workflow `37603087138`.

A real MAPI course cell from `MAPI01.RAW` / `MAPI02.RAW`, tile 137 subcell (0,0), resolved through original v1.014 to descriptor 23 / landing code 9, activated event 11 and executed the original ranged-PRNG deflection. Recovered lookup and interaction state matched exactly at zero tolerance.

The consolidated sign-off workflow `37603222703` then rebuilt from a clean checkout and passed:

- all Python and C++ core tests;
- 3 launch-to-rest cases;
- 6 ordinary surface cases;
- 3 hazard cases;
- 3 putter cases;
- 3 green-slope cases;
- hole capture;
- PRNG sequence parity;
- 3 interaction branches;
- 81,920 MAPI lookup cases;
- the end-to-end course-interaction fixture.

CI `37603222791` passed on the same sign-off commit.

Durable evidence was committed as:

- `analysis/evidence/gate1_final_signoff.json`
- `analysis/evidence/gate1_course_interaction_parity.json`

**Gate 1 — Original Gameplay and Physics Recovery: COMPLETE / GO.**

Gate 2 — complete platform-neutral classic game/core integration for the mobile product — is now the next project phase.


## 2026-10-07 — Gate 2 Burst 1: production classic shot bridge

### Implemented

`ClassicShotModel` now implements the production `IClassicModel` boundary using only the Gate-1 recovered mechanics.

It preserves original direction units, exposes the exact `0x3A8` logical timer interval and gives platform shells deterministic snapshots without duplicating physics.

Primary commit: `8000141a11f8e34d5ad99879a4003507a82552ee`.

Gate-1 golden masters remained green.

## 2026-10-07 — Gate 2 Burst 2: original course/resource model

### Implemented / verified

The portable core now loads the original MAPM/MAPI/SPT course bundle without committing commercial payloads.

Recovered and integrated:

- MAPM width/height and 10-bit tile references;
- MAPI descriptor/selector banks;
- original 16.16 ball-coordinate -> MAPM/MAPI subcell decomposition;
- all 77 terrain descriptor records;
- SPT records 0..3 words 2/3 as player tee/start coordinates;
- SPT record 4 words 2/3 as cup/hole coordinates.

Validated checkpoint: `97c89e13ca0f829b9977ff6b887ef7217235d0f8`.

Successful evidence:
- course-resource validation `37616105293`;
- CI `37616105377`;
- all Gate-1 golden masters green.

## 2026-10-07 — Gate 2 Burst 3 checkpoint: hole/session state and special surfaces

### Implemented

`ClassicHoleSession` now owns:

- tee/start position;
- shot lifecycle;
- stroke count;
- authoritative moving-ball terrain refresh;
- normal rest;
- hazard terminal state;
- hole terminal state;
- flat-green putter;
- explicit unsupported-rule state.

### Special-rule analysis

Successful workflows:

- special-green dispatcher: `37616408565`;
- special landing tail: `37616542794`;
- special-surface zero-tolerance trajectory parity: `37616747731`;
- terrain variant/profile-field analysis: `37617022417`.

Normal non-putter ball trajectory for landing codes 10, 50 and 60 matches original v1.014 at zero tolerance.

The putter-specific branches for codes 9/10/50/60 and the full session side effects are still being integrated. Hazard recovery/drop/penalty and next-hole/scoring state remain open.

No unsupported branch is approximated.
