# Gate 1 — Original Gameplay and Physics Recovery

Status: **PROVISIONAL GO — requires executable/data verification**

## Purpose

This is the project-killing gate. Before graphics, mobile UI or enhancements, prove that the original Sensible Golf gameplay can be recovered faithfully enough for a modern port.

## What must be recovered

1. Shot aiming and heading representation.
2. Welly-o-meter timing and power mapping.
3. Accuracy timing -> straight/draw/fade mapping.
4. Club table: maximum distance, launch/height behaviour, and any club-specific modifiers.
5. Lie/surface penalties and safe-zone changes.
6. Airborne ball movement and curvature.
7. Tree/obstacle collision behaviour.
8. Landing, bounce and roll.
9. Water/out-of-bounds handling.
10. Putting physics and green slope influence.
11. Hole/cup capture behaviour.
12. Any deterministic/random variation affecting shot outcome.

## Evidence already established

- The PC release contains a DOS4GW executable (`GOLFDOS.EXE`) and a separate PE32 Windows executable (`GOLFWIN.EXE`). Two implementations are useful for cross-checking recovered logic.
- The DOS port was produced by East Point Software.
- East Point's EPF archive format used by Sensible Golf is documented: it stores 8.3 filenames, compression flags and compressed/decompressed sizes. Compression is documented LZW (9–14 bit dynamic codes), so game data is not an opaque container.
- The original manual describes a deliberately compact arcade model rather than a high-dimensional golf simulation: three-click power/accuracy, draw/fade, club trajectory differences, lie penalties, and green slope behaviour.
- Manual calibration points include a 1 Wood maximum of 240 yards, woods travelling far/low, higher irons pitching over obstacles, worse lies reducing distance, and green slopes altering direction/speed.

## Recovery strategy

### Phase A — Inventory and unpack

Input required: a legally held DOS installation/archive containing the original game files.

- SHA-256 every input file.
- Identify PE/LE executables and resource/archive files.
- Enumerate every EPF FAT entry without modifying the originals.
- Extract EPF content to an analysis workspace.
- Classify likely course, tile, palette, sound, lookup-table and gameplay-data files.

### Phase B — Static executable analysis

Prefer `GOLFWIN.EXE` first because PE32 analysis is simpler, then cross-check `GOLFDOS.EXE`.

Locate code/data via:

- references to club names/distances and UI values;
- reads of current club, lie and Welly-o-meter state;
- writes to ball X/Y/height/velocity-like state;
- terrain lookup calls during flight/landing/roll;
- green arrow/slope data access;
- collision and cup checks;
- fixed-point math, lookup tables and PRNG calls.

Name recovered functions by observed behaviour, never by guesswork.

### Phase C — Dynamic black-box measurements

Record repeatable original-game shots with controlled inputs:

- same hole/tee/aim/club;
- several exact power points;
- centre accuracy plus symmetric draw/fade offsets;
- fairway/rough/semi-rough/bunker lies;
- representative woods and irons;
- putting uphill/downhill/across slope.

For every run capture frame/tick trajectory, landing point, bounce(s), final rest point and surface transitions.

### Phase D — Reimplementation

Write a deterministic portable gameplay core independent of renderer and frame rate. Preserve original fixed-point/table behaviour where it materially affects feel.

Target API shape:

```
ShotInput -> OriginalGameplayCore -> BallState[t] -> RestResult
```

Rendering must consume simulation state; it must not drive physics.

## Hard parity tests

The gate passes only when a representative suite demonstrates:

- same shot direction for identical aiming input;
- same power/distance mapping within measurement tolerance;
- same sign and practical magnitude of draw/fade;
- same club trajectory class and obstacle-clearing behaviour;
- same lie penalties;
- same terrain outcome and hazard handling;
- same green slope response;
- same landing and final-rest coordinates within agreed tolerance;
- deterministic repeatability where the original is deterministic.

A pretty approximation does **not** pass.

## Go / No-Go criteria

### GO

Proceed if we can identify the relevant state and either:

A. translate the original routines/tables directly into portable code, or
B. reproduce their externally observed behaviour with a deterministic model that passes the parity suite.

### NO-GO

Stop the Enhanced-port approach if, after binary/data analysis, the shot result depends on unrecoverable encrypted/obfuscated logic, inaccessible external data, or behaviour that cannot be reproduced consistently from controlled tests.

Current evidence makes those failure modes look unlikely, but this remains unproven until the actual binary/data set is inspected.

## Immediate next input

Place/provide a legally held copy of the original DOS game archive/install for analysis. Do **not** commit it to this public repository unless the licence explicitly grants redistribution rights.
