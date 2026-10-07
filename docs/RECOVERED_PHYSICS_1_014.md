# Recovered Physics — Windows v1.014

Status: **principal gameplay mechanics recovered and runtime parity-proven**

Canonical current snapshot: [PROJECT_STATUS.md](PROJECT_STATUS.md).

Reference executable:

- `GOLFWIN.EXE`
- size: 239,616 bytes
- CRC-32: `23c300b4`
- SHA-256: `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

All addresses below refer to that exact build.

## Ball state structure

The live ball record uses a `0x2C` / 44-byte stride.

| Offset | Recovered field |
|---:|---|
| +0x00 | X coordinate, dword |
| +0x04 | Y coordinate, dword |
| +0x08 | vertical force, dword |
| +0x0C | horizontal force, dword |
| +0x10 | direction, dword |
| +0x14 | height, dword |
| +0x18 | distance to hole, dword |
| +0x1C | pause/timing field, dword |
| +0x1E | terrain/surface id, word |
| +0x26 | terrain/slope direction-like value |
| +0x2A | terrain/slope magnitude-like word |

X/Y are 32-bit fixed-point-style values with integer map coordinates in the upper word.

## Direction and trigonometry

Direction is masked with `0xFFF`: **4096 angular units per circle**.

The original signed Q14 sine table is regenerated exactly by:

```
trunc(sin(2*pi*i/4096) * 16384)
```

with the negative peak clamped to `-16383`.

Horizontal integration:

```
dx = arithmetic_shift_right(H * sin_q14(direction), 14)
dy = arithmetic_shift_right(H * sin_q14(direction + 1024), 14)
X += dx
Y += dy
```

The generated table matches the executable exactly.

## Club physics

The 13 records at `0x41F108` contain:

```
raw vertical base : int32
raw horizontal base : int32
power scale : int32
```

The first two values are halved when loaded.

| Index | Loaded V | Loaded H | Power scale |
|---:|---:|---:|---:|
| 0 | 8,192 | 81,920 | 3,168 |
| 1 | 16,384 | 73,728 | 3,120 |
| 2 | 36,864 | 65,536 | 3,120 |
| 3 | 49,152 | 49,152 | 3,072 |
| 4 | 65,536 | 49,152 | 2,944 |
| 5 | 71,680 | 49,152 | 2,880 |
| 6 | 77,824 | 49,152 | 2,560 |
| 7 | 81,920 | 49,152 | 2,496 |
| 8 | 112,640 | 48,128 | 2,464 |
| 9 | 114,688 | 48,128 | 2,368 |
| 10 | 131,072 | 47,104 | 2,176 |
| 11 | 131,072 | 40,960 | 2,112 |
| 12 | 0 | 8,192 | 1,536 |

Index 12 is the special putter path and is now runtime parity-tested.

## Welly-o-meter and launch power

The original raw captured-power range is **0..105**.

The rising meter counter caps at `0x69` / 105. The descending branch starts from 70 and applies the original conversion that also reaches 105 at the top.

The meter tail, dispatcher and logical timer producer are now traced through the Windows scheduler rather than treating `DropPower` as an unexplained external input.

For the live normal-shot launch:

```
error = accuracy_tick - 63
adjusted_power = max(0, captured_power - abs(error))
direction = (player_direction - error * 16) & 0xFFF
swing_adjuster = selected_profile[evenized(error)]

V = club.vertical_base + club.power_scale * adjusted_power
H = club.horizontal_base + club.power_scale * adjusted_power
```

## Accuracy / draw / fade

The executable contains an exact 13x10 club/lie profile selector and 11 signed swing profiles.

The recovered portable core reproduces selected profile, accuracy bounds, power loss from off-centre timing, initial direction change and per-tick direction change from the swing adjuster.

Straight, draw and fade launch-to-rest cases match original v1.014 machine code at zero tolerance.

## Airborne flight

Normal airborne update:

```
V -= 0x2100
height += V
if height < 0:
    height = 0
```

Normal horizontal drag is `0xF00`.

The original zero-crossing branch is preserved: if positive H crosses below zero during a drag step, v1.014 ends that tick before the direction/movement step.

## Landing, bounce and roll

Recovered ground-contact response:

```
V = arithmetic_shift_right(-V, 1)
H += arithmetic_shift_right(V, 1)
if H <= 0:
    H = 0
    V = 0
```

Launch-to-rest golden masters match the original complete state trajectory at zero tolerance.

## Ordinary surfaces and hazards

Named ordinary descriptor classes parity-tested:

- skirt;
- fairway;
- semi rough;
- rough;
- very rough;
- sand.

Hazard/terminal paths parity-tested:

- water;
- NO GO;
- out-of-bounds.

## Putting and green slope

Green rolling drag is `0x780`.

Club 12 follows the special putter path. Flat-green putter movement and controlled slope projection are runtime parity-tested.

The slope adjustment uses the recovered terrain direction/magnitude fields and the same Q14 projection machinery.

## Cup / hole capture

The original code-8 terminal hole branch has been isolated, implemented and zero-tolerance parity-tested.

## Course collision lookup

The authoritative course lookup is the MAPI path, not the `WOOD*.BIN` graphics resources.

The portable MAPI lookup is exhaustively compared with original v1.014 machine code over **81,920 cases** with zero mismatches in the successful sign-off run.

## Original PRNG and shot interactions

The original game uses a recovered 16-bit ranged PRNG.

Its output and seed evolution are parity-tested independently.

Three shot-state mutation fragments are also parity-tested against original machine code:

- code-9 low-height deflection;
- near-hole lip deflection;
- flag-coordinate deflection.

These preserve the original bit-width, wrap and shift behaviour rather than replacing it with a modern random generator.

The final Gate 1 integration fixture should join the real course lookup/activation path to one of these already-verified mutations.

## Logical timing

The Windows timing layer reads `GetTickCount` and converts elapsed milliseconds to 16.16 seconds:

```
elapsed_fixed = (elapsed_ms << 16) / 1000
```

The gameplay timer callback is registered with interval:

```
0x3A8 = 936
```

fixed units.

That is approximately:

```
65536 / 936 = 70.02 callbacks/second
```

Classic simulation should preserve this logical cadence. Rendering refresh belongs above the simulation layer.

## Wind-labelled state

Wind-named debug globals and a wind-vector calculation exist, but the computed wind X/Y result is not consumed by the released Windows shot path found in the xref analysis.

Classification remains **vestigial/debug unless contradictory runtime evidence appears**.

## Portable core boundary

The recovered portable implementation now contains parity-evidenced mechanics for direction/trig, club launch data, power and accuracy launch logic, swing curvature, airborne flight, ground contact/bounce/roll, ordinary surfaces/hazards, putting/slope, cup capture, original PRNG, interaction state mutations and course MAPI collision lookup.

Gate 1 no longer depends on inventing a substitute golf model.

## Gate status

**COMPLETE — GO.**

The end-to-end lookup/activation/interaction fixture passed in run `37603087138`, and the consolidated Gate-1 suite passed in run `37603222703` with CI `37603222791`. The recovered classic physics model is now the frozen fidelity baseline for subsequent development.
