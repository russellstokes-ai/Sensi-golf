# Golden Master — Live Launch and Clear-Air Result

Status: **PARITY VERIFIED**

Reference executable:

- Sensible Golf Windows v1.014
- SHA-256: `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

## Method

The original PE machine code was loaded into a 32-bit x86 Unicorn emulator.

The oracle executes:

- live-player launch at `0x40C84F`;
- normal clear-air update at `0x40A603`;
- direction/planar integration through the `0x40A96F` path.

The portable C++ core is run independently with the same controlled inputs.

Comparison fields per logical tick:

- X;
- Y;
- height;
- vertical force;
- horizontal force;
- direction.

Tolerance is **zero**.

## Result

| Case | Inputs | Samples | Mismatches |
|---|---|---:|---:|
| straight_1w | club 0, power 105, direction 0, swing 0 | 21 | 0 |
| curved_mid | club 5, power 83, direction 777, swing +1 | 21 | 0 |
| curved_high | club 11, power 60, direction 3072, swing -2 | 16 | 0 |

**58 samples total, six state fields each, zero mismatches.**

Examples:

### Straight case, tick 20

```
x = 0
y = 7484800
height = 5042560
vertical_force = 171872
horizontal_force = 337760
direction = 0
```

### Curved mid case, tick 20

```
x = 4546647
y = 1973471
height = 4440320
vertical_force = 141760
horizontal_force = 211392
direction = 737
```

### Curved high case, tick 15

```
x = -2051472
y = 94121
height = 2853120
vertical_force = 131072
horizontal_force = 110080
direction = 3132
```

The original and recovered values are identical in every tested field.

## Correction discovered during parity preparation

The active portable core previously used an interpretation of the routine around `0x40AD1D` that directly folded an accuracy error into launch power and heading.

Later v1.014 analysis had already established that the **live-player launch path is `0x40C84F`**:

```
ball.direction = player.direction
scaled = club.power_scale * DropPower
ball.V = club.vertical_base + scaled
ball.H = club.horizontal_base + scaled
```

The active core has now been corrected to that live-player path. Swing/profile output is applied separately by the recovered per-tick adjustment path.

## What this proves

We now have the first subsystem promoted from static recovery to **parity-verified**:

**normal non-putter live-player launch + normal clear-air flight.**

This is materially stronger than comparing two hand-written formulas because the reference side is the original v1.014 x86 machine code itself.

## What it does not prove

Gate 1 remains open. This result does not yet verify:

- meter/profile selection from real user input;
- lie and terrain lookup semantics;
- ground contact on actual course data;
- bounce and roll through final rest;
- green/putting path;
- water/out-of-bounds;
- tree/obstacle collision;
- cup capture;
- logical tick frequency;
- full in-game black-box shot trace.

Next parity target: **ground contact → bounce → roll → final rest**, then real-course traces.
