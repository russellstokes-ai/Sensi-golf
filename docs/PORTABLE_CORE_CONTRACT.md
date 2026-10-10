# Portable Classic-Core Contract

Status: **GATE 1 PHYSICS COMPLETE; GATE 2 PRODUCT BRIDGE IN PROGRESS**

## Purpose

The classic core is the single authoritative implementation of recovered Sensible Golf behaviour.

It is portable **C++17** so the same deterministic game logic can be hosted by Android NDK, optional future iOS code and developer/test executables without duplicating physics or rules.

## Core rules

### 1. Gate-1 parity is immutable by default

Changes to recovered shot, terrain, hazard, putting, PRNG, collision or cup behaviour require new original-game evidence and must keep the zero-tolerance Gate-1 suites green.

### 2. Fixed logical simulation

`IClassicModel::step()` advances exactly one original logical update.

The recovered Windows scheduler uses interval `0x3A8` in 16.16-second units, approximately 70.02 callbacks/second. Rendering frequency is independent and may interpolate at the display refresh rate.

### 3. Platform shells do not own gameplay

Android may provide touch/gamepad input, rendering, audio, files and lifecycle handling. It must not calculate ball flight, surfaces, collision, PRNG outcomes, putting or hole rules.

### 4. Authoritative state is renderer-independent

The C++ game layer owns ball state, shot phase, terrain/hazard/hole results and deterministic random/session state.

### 5. Original data remains external

Licensed/original commercial files are loaded through the import layer and are not committed to the public repository unless redistribution rights explicitly permit it.

## Gate-2 bridge

`ClassicShotModel` is the first production implementation of `IClassicModel`.

It:

- wraps the Gate-1 parity-proven launch/flight/ground/putter routines;
- accepts original 0..4095 aim units;
- preserves the exact scheduler interval constant `0x3A8`;
- exposes deterministic `BallState` snapshots;
- surfaces hazard and holed terminal state;
- prevents terrain context changes during a live shot.

This class is a bridge, not a second physics implementation.

## Next layers

The remaining Gate-2 architecture is:

```
ClassicShotModel
      ↓
course/resource model
      ↓
hole/session state machine
      ↓
save/replay contract
      ↓
Android NDK/JNI host
```

See [GATE2_MOBILE_INTEGRATION.md](GATE2_MOBILE_INTEGRATION.md).
