# Porting Strategy

## Approach

This project is a **modern source-level reimplementation of recovered original behaviour**, not an emulator wrapper and not a new golf game inspired by the original.

Where original source code is unavailable, recover behaviour from:
1. licensed executable/data inspection;
2. static analysis;
3. controlled black-box measurements;
4. cross-comparison of DOS and Windows PC builds;
5. documented game data and archive formats.

## Architecture target

```
Original data/import
        ↓
Portable deterministic gameplay core
        ↓
Simulation state snapshots
        ↓
Platform-independent presentation API
       ↙ ↘
 Android  optional iOS / developer test harness
```

## Separation rules

### Gameplay core
Owns:
- aim/heading;
- club parameters;
- swing power;
- accuracy/draw/fade;
- ball flight;
- collisions;
- surface/lie effects;
- bounce/roll;
- green slope/putting;
- scoring/hole state;
- deterministic random state if present.

It must not own:
- touch coordinates;
- screen resolution;
- rendering timing;
- audio devices;
- filesystem paths;
- Android/iOS APIs.

### Renderer
May interpolate visual positions between simulation ticks. It must never alter authoritative ball state.

### Import layer
Reads licensed original files and converts them into a documented internal representation. Original commercial assets remain external to the public repo.

## Product direction

Android is the first playable product target. Desktop executables may exist only as developer/test utilities; they are not a release milestone and must not acquire gameplay logic that belongs in the portable core.

## First executable to analyse

Prefer `GOLFWIN.EXE` for initial static analysis if available because PE32 tooling is straightforward. Cross-check recovered tables/routines against `GOLFDOS.EXE` so we do not accidentally preserve a port-specific bug as universal behaviour without documenting it.
