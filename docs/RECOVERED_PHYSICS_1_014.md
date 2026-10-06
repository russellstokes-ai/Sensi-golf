# Recovered Physics — Windows v1.014

Status: **substantial static recovery complete; runtime golden-master parity still required**

Reference executable:

- `GOLFWIN.EXE`
- size: 239,616 bytes
- CRC-32: `23c300b4`
- SHA-256: `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

All addresses below refer to that exact build.

## Ball state structure

Embedded debug descriptors point directly at the live ball structure. Base for the first observed slot is `0x4287D6`; slots use a `0x2C` (44-byte) stride.

| Offset | Recovered field |
|---:|---|
| +0x00 | X coordinate, dword |
| +0x04 | Y coordinate, dword |
| +0x08 | vertical force, dword |
| +0x0C | horizontal force, dword |
| +0x10 | direction, dword |
| +0x14 | height, dword |
| +0x18 | distance to hole, dword |
| +0x1C | pause, dword |
| +0x1E | terrain/surface id, word |
| +0x26 | terrain/slope direction-like value |
| +0x2A | terrain/slope magnitude-like word |

`DWball x position` and `DWball y position` point at X+2/Y+2. Code also shifts X/Y by 16 before map/distance operations, establishing a 32-bit fixed-point-style representation with integer map coordinates in the high word.

## Direction and exact trigonometry

Direction is masked with `0xFFF`: **4096 angular units per circle**.

The original lookup at `0x41A670` is signed Q14 sine. The orthogonal component indexes 1024 entries later.

The table is reproduced exactly by:

```
trunc(sin(2*pi*i/4096) * 16384)
```

with the negative peak clamped to `-16383`.

A 5120-entry comparison against the executable produced **zero mismatches**.

Horizontal integration:

```
dx = arithmetic_shift_right(H * sin_q14(direction), 14)
dy = arithmetic_shift_right(H * sin_q14(direction + 1024), 14)
X += dx
Y += dy
```

The new core's lookup header is mathematically generated from this formula, not copied from the original table bytes.

## Club physics

Routine around `0x40A1C1` indexes 13 records at `0x41F108`, each 12 bytes:

```
raw vertical base : int32
raw horizontal base : int32
power scale : int32
```

Vertical and horizontal bases are divided by two when loaded.

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

Index 12 follows a special ground path and has zero vertical base, supporting its identification as the putter. The binary-string search did **not** yield trustworthy names for indices 0–11; names remain unpromoted rather than guessed.

## Shot initialization

Around `0x40C84F`:

```
ball.direction = shot.direction
power_component = club.power_scale * DropPower
ball.V = club.loaded_vertical_base + power_component
ball.H = club.loaded_horizontal_base + power_component
```

`DropPower` is an unsigned word at `0x41D59E`.

The exact meter-to-`DropPower` mapping remains open, so the portable kernel takes raw `DropPower` as an input.

## Accuracy / draw-fade profiles

The one-time direction rule around `0x40A96F` is:

```
ball.direction -= 2 * swing_adjuster
ball.direction &= 0xFFF
```

A further static pass recovered **11 profile tables** starting at `0x41F204`, each 28 bytes = 14 signed 16-bit slots. The launch path selects signed values around a zero centre using even accuracy-error offsets.

Examples:

- profiles 0–2: `... -4, -4, -4, -2, 0, 2, 4, 4, 4 ...`
- profiles 3–6: progressively gentler `... -4, -2, -1, -1, 0, 1, 1, 2, 4 ...`
- later profiles introduce a wider zero/dead zone.

This is strong evidence for club/profile-specific accuracy sensitivity. The remaining task is to prove the exact profile-selection mapping and whether draw/fade also changes direction continuously during flight. The portable core therefore accepts a recovered `swing_adjuster` but does not invent a meter/profile selector or continuous curve.

## Vertical flight

Normal airborne update:

```
V -= 0x2100
height += V
if height < 0:
    height = 0
```

Gravity: **8448 raw units per logical update**.

Logical tick frequency is still open.

## Rolling drag and green mode

```
drag = 0xF00
if green_mode:
    drag = 0x780
H = max(0, H - drag)
```

Green mode is explicit state at `0x41EE7A`. Entry/exit code compares integer ball X/Y against:

- x1 `0x41E5EC`
- x2 `0x41E5EE`
- y1 `0x41E5F0`
- y2 `0x41E5F2`

## Bounce

Recovered ground-contact response:

```
V = arithmetic_shift_right(-V, 1)
H += arithmetic_shift_right(V, 1)
if H <= 0:
    H = 0
    V = 0
```

## Terrain / slope

Terrain lookup writes direction/magnitude-like values at ball +0x26/+0x2A. Routine `0x40B965` projects them through the same Q14 trig system to X/Y adjustments used by movement under relevant conditions.

The exact semantic labels and all surface IDs remain provisional until map/terrain records are decoded.

## Distance to hole

Routine `0x40B998` derives integer X/Y, handles green coordinate scaling, computes Euclidean distance to the hole, multiplies by six and divides by ten, then stores the result at ball +0x18.

## Wind-labelled state

Wind-named debug variables and a wind-vector calculation exist. Static xrefs show the computed wind X/Y result globals being written but not otherwise read in this Windows build.

That is consistent with wind not being active in released gameplay. Classification: **vestigial/debug state unless runtime evidence proves otherwise**.

## Portable kernel boundary

`recovered_math` now implements only statically evidenced mechanics:

- 4096-step direction wrapping;
- exact clean-generated Q14 trig;
- x86-compatible signed Q14 projection;
- all 13 club parameter rows;
- raw `DropPower` launch formula;
- recovered swing-adjuster heading rule;
- gravity/height integration;
- normal/green rolling drag;
- bounce transfer;
- horizontal X/Y integration.

It intentionally excludes unresolved terrain/hazard/cup rules, meter timing, profile selection, tick frequency, and possible continuous curvature.

## Gate status

The central feasibility question is now answered strongly in favour of **GO**: the original ball engine is compact deterministic fixed-point/integer logic and its major state/tables/update arithmetic are recoverable.

Gate 1 remains open until at least one **runtime golden-master shot** from the original build is reproduced numerically from launch through final rest.
