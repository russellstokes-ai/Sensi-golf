# Portable Classic-Core Contract

Status: **CHASSIS COMPLETE — NO GAME PHYSICS IMPLEMENTED YET**

## Why this exists now

The recovered model needs somewhere stable to land as individual routines and tables are identified. Creating that boundary now prevents reverse-engineering discoveries from being mixed directly into Android UI/rendering code.

The chassis is deliberately behaviour-free. It does **not** guess Sensible Golf physics.

## Language

The classic core is currently defined in portable **C++17** because it can be compiled for:
- Android NDK;
- iOS;
- desktop analysis/test harnesses;
- future rendering engines via a narrow native interface.

The platform choice can still change above the core without changing recovered gameplay behaviour.

## Core rules

### 1. Raw units until proven

Coordinates and velocities are `RawScalar` integer values.

We do not yet assert that original Sensible Golf uses:
- pixels;
- yards;
- metres;
- floating point;
- 16.16 fixed point;
- any particular scaling.

Once native representation is recovered, the raw-value meaning will be documented and tests updated.

### 2. Fixed logical ticks

`IClassicModel::step()` advances exactly one original logical simulation tick.

The renderer may later interpolate those states at 60/90/120 fps. Rendering frequency must never change shot results.

### 3. No approximate default engine

`IClassicModel` is an interface. There is intentionally no fallback implementation with invented gravity, drag or friction.

A real classic implementation is introduced only after its rules are evidenced.

### 4. Authoritative state is renderer-independent

The core owns:
- ball state;
- shot phase;
- surface/hazard/holed state;
- later: club/lie/rules parameters and any deterministic PRNG.

The renderer and mobile input layer consume or produce inputs; they do not mutate physics behind the core.

### 5. Evidence travels with recovery work

`EvidenceAnchor` records where a recovered fact came from:
- executable offset;
- data-file offset;
- black-box trace;
- manual constraint.

Recovered constants/formulas should not enter the classic core without an evidence record.

## Current files

```
engine/
  include/sensigolf/
    classic_model.hpp
    evidence.hpp
    trace.hpp
    types.hpp
  src/
    trace.cpp
  tests/
    core_contract_tests.cpp
```

## What the current test proves

The contract test proves:
- the core compiles as C++17;
- state equality is exact;
- traces preserve ordered logical ticks;
- duplicate/non-increasing ticks are rejected;
- a model can be stepped independent of any renderer.

The mock movement in the test is **not Sensible Golf physics** and must never be used as such.

## Next implementation milestone

After the original PC executable is ingested:

1. resolve original tick rate/units;
2. implement recovered shot initialization;
3. implement one recovered per-tick path;
4. record one original golden-master shot;
5. require the C++ core to match that shot before expanding to other clubs/lies.
