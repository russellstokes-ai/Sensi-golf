# Gate 1 — Original Gameplay and Physics Recovery

Status: **GO — static recovery is substantial; runtime golden-master parity still required**

## Purpose

This is the project-killing gate. Before HD graphics, mobile presentation or enhancement features, prove that the original Sensible Golf gameplay can be recovered faithfully enough for a modern port.

The earlier binary/data feasibility question has now been answered positively. The selected PC v1.014 build has been ingested, its complete EPF archive extracted, and major shot/ball routines and tables recovered. The remaining gate is **runtime parity**, not access to the implementation.

## Recovery checklist

1. **Shot aiming and heading representation** — recovered: 4096 direction units/circle.
2. **Welly-o-meter timing and power mapping** — partially recovered; final user-facing timing mapping requires closure.
3. **Accuracy -> straight/draw/fade mapping** — 11 profile tables and launch adjustment recovered; final runtime behaviour requires parity.
4. **Club table** — 13 physics records recovered; putter special path identified.
5. **Lie/surface penalties** — incomplete.
6. **Airborne movement** — core fixed-point integration, trig and gravity recovered; any remaining continuous curvature must be proven.
7. **Tree/obstacle collision** — incomplete.
8. **Landing, bounce and roll** — major branch/arithmetic recovered.
9. **Water/out-of-bounds handling** — incomplete.
10. **Putting and green slope** — putter path, green drag/state and slope projection substantially recovered; full semantics/parity incomplete.
11. **Hole/cup capture** — incomplete.
12. **Randomness** — final classification pending.
13. **Logical simulation tick rate** — pending.
14. **Golden-master runtime parity** — pending and mandatory.

## Verified reference build

Selected parity build:

- `GOLFWIN.EXE`: v1.014, SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`
- `GOLFDOS.EXE`: SHA-256 `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed`
- `GOLF.EPF`: SHA-256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`

`GOLF.EPF` was extracted successfully: **277/277 entries**.

## Major static recovery already established

The Windows v1.014 executable exposes enough debug/state structure to trace the original engine directly.

Recovered evidence includes:
- 44-byte ball-state stride;
- fixed-point X/Y representation;
- vertical/horizontal force, direction, height and terrain-related fields;
- 12-bit direction mask = 4096 angular units/circle;
- exact Q14 trig regeneration;
- 13-row club physics table;
- raw launch formula from club parameters + `DropPower`;
- launch heading adjustment from swing accuracy;
- 11 swing/accuracy profile tables;
- gravity = `0x2100` raw units/logical update;
- normal rolling drag = `0xF00`;
- green rolling drag = `0x780`;
- bounce response and force transfer;
- terrain/slope vector projection;
- green-bounds state;
- distance-to-hole calculation.

See [RECOVERED_PHYSICS_1_014.md](RECOVERED_PHYSICS_1_014.md).

## Reimplementation rule

Portable code must implement recovered integer/fixed-point behaviour rather than substitute a modern physics engine.

Target boundary:

```
ShotInput -> ClassicGameplayCore -> BallState[t] -> RestResult
```

The renderer consumes simulation state and may interpolate visually; it never drives authoritative physics.

## Hard parity test

Gate 1 closes only when representative shots captured from the original v1.014 build are replayed through the portable core and demonstrate:

- same launch heading;
- same power/distance behaviour;
- same draw/fade behaviour;
- same trajectory class;
- same lie/surface effects;
- same collision/hazard outcomes;
- same green/slope behaviour;
- same landing/bounce/roll;
- same final rest;
- same hole/cup outcome;
- same logical tick progression where observable;
- deterministic repeatability where the original is deterministic.

A visually convincing approximation does **not** pass.

## Go / No-Go decision

### Current decision: GO

The original engine is compact fixed-point/integer logic with recoverable state, tables and update arithmetic. No encryption, inaccessible external dependency or irreducibly opaque simulation has been found.

### Remaining stop condition

If controlled runtime traces reveal material shot behaviour that cannot be explained/reproduced from the recovered state and routines, Gate 1 remains open. Enhancement work must still wait.

## Immediate next evidence

Capture at least one controlled **original v1.014 golden-master shot trace**, then reproduce it numerically through the portable core from launch to final rest. Expand that into the representative parity suite before closing Gate 1.
