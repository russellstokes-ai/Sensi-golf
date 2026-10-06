# Recovered Classic Flight Model

Status: **IMPLEMENTED FOR NORMAL LAUNCH + CLEAR AIRBORNE TICKS**

This document records only behaviour directly recovered from the PC executable.
It is not a guessed golf model.

## Reference build

Windows executable:

- size: 239,616 bytes
- CRC-32: `23c300b4`
- SHA-256: `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

## Original state layout

The first ball record begins at `0x4287D6`, with a stride of `0x2C`.

| Ball offset | Recovered meaning |
|---|---|
| +0x00 | X coordinate |
| +0x04 | Y coordinate |
| +0x08 | vertical force |
| +0x0C | horizontal force |
| +0x10 | direction |
| +0x14 | height |
| +0x18 | distance/state field |
| +0x1C | pause/timing field |

The 16-bit developer displays for X/Y reference the upper words of the 32-bit
X/Y values, and other code shifts integer map coordinates by 16 before storing
them. X/Y are therefore strongly evidenced as 16.16-style fixed-point values.

Current-player and current-ball pointers are stored at `0x41F3A8` and
`0x41F3AC`.

## Direction / trig

Direction is masked with `0xFFF`, giving 4096 angular positions.

The lookup table beginning at `0x41A670` was compared exhaustively against
clean mathematical regeneration. All values match:

```
int(sin(2*pi*i/4096) * 16384)
```

with the negative peak clamped to `-16383`.

The Y lookup is the same periodic table offset by 1024 samples (one quarter
cycle). Products are shifted right by 14.

## Club launch parameters

The original loader at `0x40A1C1` indexes a 13-record table at
`0x41F108`, 12 bytes per record. It halves the first two dwords and copies
the third unchanged.

Post-load values consumed by launch:

| Index | V base | H base | Power scale | Special |
|---:|---:|---:|---:|---|
| 0 | 8192 | 81920 | 3168 | |
| 1 | 16384 | 73728 | 3120 | |
| 2 | 36864 | 65536 | 3120 | |
| 3 | 49152 | 49152 | 3072 | |
| 4 | 65536 | 49152 | 2944 | |
| 5 | 71680 | 49152 | 2880 | |
| 6 | 77824 | 49152 | 2560 | |
| 7 | 81920 | 49152 | 2496 | |
| 8 | 112640 | 48128 | 2464 | |
| 9 | 114688 | 48128 | 2368 | |
| 10 | 131072 | 47104 | 2176 | |
| 11 | 131072 | 40960 | 2112 | |
| 12 | 0 | 8192 | 1536 | putter path |

Index 12 is explicitly handled as a distinct putter path in gameplay code.
The normal-club index cycles over 0..12. External game documentation confirms
the title uses 1/2/3 Woods, numbered irons, PW, SW and a putter; exact
human-readable mapping for every middle index remains a separate UI/data
cross-check and is not required by the physics core.

## Welly-o-meter power

The rising power counter increments until `0x69` (105). The descending phase
starts at 70 and applies a 1.5x conversion when captured, also yielding 105 at
the top.

Therefore:

```
0 <= captured_power <= 105
```

## Accuracy and launch

The normal launch initializer at `0x40AD1D..` uses an accuracy error relative
to centre value 63.

```
power = max(0, captured_power - abs(accuracy_error))
direction = (player_direction - accuracy_error*16) & 0xFFF
vForce = vertical_base + power_scale*power
hForce = horizontal_base + power_scale*power
```

A club/lie mapping selects one of the 28-byte swing profiles at `0x41F204`.
The centre entry (`accuracy_error == 0`) is exactly zero for every recovered
profile. Thus a centre-timed shot begins with no per-tick curve adjustment.

## Clear-air update

For the normal clear-air path recovered around `0x40A250..` / `0x40A96F..`:

```
vertical_force -= 0x2100
height += vertical_force

horizontal_force = max(0, horizontal_force - 0x0F00)

direction = (direction - 2*swing_adjuster) & 0xFFF

x += (horizontal_force * trig[direction]) >> 14
y += (horizontal_force * trig[direction + 1024]) >> 14
```

There is a special path using half horizontal drag and optional X/Y adjustment
globals. Those paths are intentionally not folded into the first implementation
until their activation conditions are completely classified.

## Implemented scope

`engine/src/recovered_flight.cpp` currently implements:

- the recovered 13-entry launch-parameter table;
- original 4096-angle Q14 trig regeneration;
- normal non-putter launch math;
- accuracy direction/power penalty;
- clear-air gravity;
- clear-air horizontal decay;
- draw/fade direction stepping when the caller supplies the recovered
  swing-adjuster value;
- fixed-point planar movement.

It deliberately stops before ground contact and returns
`NeedsLandingResolution`.

Not yet claimed/implemented:

- bounce;
- terrain friction;
- lie-dependent launch mapping in the public C++ API;
- hazards;
- cup capture;
- putting/green slope;
- special half-drag/adjustment path;
- original logical tick rate;
- black-box golden-master parity.

The next gate is to recover ground contact/bounce/roll and then compare one
complete original shot from launch through final rest.
