# Golden-Master Gameplay Harness

## Current status

**Broad original-v1.014 zero-tolerance parity is established.**

The harness executes original Sensible Golf Windows v1.014 machine code directly under 32-bit x86 Unicorn and compares it with the independently recovered portable C++17 core.

Synthetic test fixtures validate tooling only. Gameplay claims are promoted only when original code/data is exercised.

## Reference build

`GOLFWIN.EXE` SHA-256:

`3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

## Verified shot-path coverage

### Generic launch-to-rest

Run `37523005402`:

| Case | Input summary | Samples | Result |
|---|---|---:|---|
| straight_1w | club 0, lie 0, power 105, accuracy 63, heading 0 | 153 | exact |
| draw_mid | club 5, lie 2, power 83, accuracy 59, heading 777 | 131 | exact |
| fade_high | club 11, lie 0, power 60, accuracy 67, heading 3072 | 111 | exact |

Every logical sample matched X/Y, height, vertical force, horizontal force, direction, swing adjuster and adjusted power. Landing and final-rest events matched exactly.

### Ordinary surfaces

Run `37535123112` covers skirt, fairway, semi rough, rough, very rough and sand.

### Hazards

Run `37535123148` covers water, NO GO and out-of-bounds.

### Putting and green

- flat-green putter parity: `37535123117`
- controlled green-slope parity: `37535123104`
- code-8 cup/hole terminal parity: `37535155917`

### PRNG and shot interactions

The original ranged 16-bit PRNG is implemented independently and parity-tested.

Interaction parity covers the original state mutations for:

- code-9 low-height deflection;
- near-hole lip deflection;
- flag-coordinate deflection.

Representative successful runs: `37545660767`, `37545660869`.

### Course collision lookup

The course MAPI lookup is compared directly against original machine code.

Successful run `37545810759` checks four MAPI bank pairs and **81,920 lookup cases** at zero tolerance.

This is the authoritative course lookup path; `WOOD1.BIN` through `WOOD4.BIN` were proved to belong to graphics-copy/rendering code rather than ball collision physics.

## Exactness policy

Where original state is integer/fixed-point and can be captured directly, tolerance is **zero**.

Do not widen tolerances to make a test pass. A mismatch is treated as evidence of an implementation or harness defect until explained.

## Important recovered edge cases

The parity work has already exposed several behaviours that would be easy to lose in a modern rewrite.

One example is the horizontal-drag zero crossing: if positive H is reduced below zero during an airborne tick, v1.014 sets H to zero and ends that tick before applying the direction/curve update. The recovered core preserves this exact branch.

PRNG-driven interaction branches likewise preserve the original 16-bit arithmetic and seed evolution rather than replacing them with a modern random generator.

## Timing and the harness

Logical state comparisons use original simulation updates, not rendering frames.

The Windows timing source has now been traced:

```
elapsed_fixed = (GetTickCount_delta_ms << 16) / 1000
```

The gameplay callback interval is `0x3A8` fixed units, about 70.02 Hz.

A modern renderer may interpolate above this layer without changing classic simulation results.

## Final Gate 1 fixture still required

The isolated components are now strongly covered, but one final integration fixture should be retained before formal Gate 1 closure:

```
real course MAPI lookup
    -> interaction activation
    -> original PRNG branch
    -> recovered state mutation
    -> continuing ball state
```

After that fixture and a consolidated current-head suite pass, Gate 1 can be formally closed.
