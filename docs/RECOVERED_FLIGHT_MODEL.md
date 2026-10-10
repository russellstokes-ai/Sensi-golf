# Recovered Classic Flight Model

Status: **LIVE PLAYER LAUNCH + NORMAL CLEAR AIR PARITY VERIFIED**

Reference:

- Windows v1.014
- SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

## Important launch-path correction

Earlier work treated the routine around `0x40AD1D` as the normal live-player launch and folded an accuracy error directly into power and initial heading.

That interpretation is **superseded for the active player core**.

The live-player launch path used for parity is `0x40C84F`:

```
ball.direction = player.direction
scaled = club.power_scale * DropPower
ball.V = club.vertical_base + scaled
ball.H = club.horizontal_base + scaled
```

Accuracy/profile selection is a separate subsystem. Its recovered output, `swing_adjuster`, is applied by the per-tick path.

## Original state layout

The first observed ball record begins at `0x4287D6`, with a `0x2C` byte stride.

| Offset | Meaning |
|---:|---|
| +0x00 | X |
| +0x04 | Y |
| +0x08 | vertical force |
| +0x0C | horizontal force |
| +0x10 | direction |
| +0x14 | height |
| +0x18 | distance/state |
| +0x1C | pause/timing |

X/Y use fixed-point-style dwords; game code uses their high words as integer map coordinates.

## Direction / trig

- 4096 angular positions.
- Direction mask: `0xFFF`.
- Signed Q14 sine table at `0x41A670`.
- Y component uses the same table +1024 entries.
- The clean generated lookup matches the original table exactly.

## Club launch parameters

The 13 records at `0x41F108` are 12 bytes each:

```
raw vertical base
raw horizontal base
power scale
```

The first two values are halved when loaded.

Index 12 is the separate putter path.

## Welly power

Raw captured `DropPower` is bounded to 0..105 in normal play.

The exact live-player launch uses raw `DropPower`; it is not reduced again inside `0x40C84F`.

## Clear-air update

Normal player clear-air behavior:

```
vertical_force -= 0x2100
height += vertical_force

horizontal_force = max(0, horizontal_force - 0x0F00)

direction = (direction - 2*swing_adjuster) & 0xFFF

x += (horizontal_force * trig[direction]) >> 14
y += (horizontal_force * trig[direction + 1024]) >> 14
```

The active C++ implementation now uses the exact generated original trig lookup rather than recomputing sine at runtime.

## Exact parity result

The original v1.014 machine code and the portable core were compared for:

- full-power straight low-launch club;
- curved mid club;
- opposite-sign curved high club.

Across 58 samples and six state fields per sample:

**0 mismatches, tolerance 0.**

See [GOLDEN_MASTER_AIRBORNE_RESULT.md](GOLDEN_MASTER_AIRBORNE_RESULT.md).

## Implemented and parity-verified

- normal non-putter player launch;
- club base/scale use;
- raw DropPower launch scaling;
- initial player heading;
- Q14 trig projection;
- gravity;
- normal clear-air horizontal drag;
- per-tick swing-adjuster direction change;
- fixed-point X/Y integration.

## Still open

- meter input -> profile/DropPower end-to-end;
- profile selector semantics for every club/lie;
- actual terrain IDs and lie modifiers;
- landing on real course data;
- bounce/roll through final rest;
- hazards;
- cup capture;
- putter/green path;
- special half-drag/adjustment path;
- logical tick rate;
- full in-game black-box parity.
