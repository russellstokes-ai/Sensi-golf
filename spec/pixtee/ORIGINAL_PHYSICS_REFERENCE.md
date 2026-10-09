# Sensible Golf v1.014 — detailed physics and ball-behaviour research reference for Pixtee

**Purpose:** Preserve the verified technical findings so Pixtee development does not start from guesswork. This is a RESEARCH reference, **not** a ready-to-import Pixtee physics implementation and **not** a declaration that recovered code/tables or original assets are cleared for reuse. Research source: `engine/src/recovered_{flight,ground,distance,interactions,prng,hazard_recovery}.cpp`, `engine/src/classic_{shot_model,hole_session,course_resources}.cpp`, `docs/RECOVERED_PHYSICS_1_014.md`, `analysis/evidence/gate1_final_signoff.json`; reference binary: **Windows GOLFWIN.EXE v1.014, SHA-256 3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8**.

## 1. Evidence and fidelity scope

- Gate-1 consolidated signoff: [GitHub Actions 37603222703](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37603222703); zero-tolerance suite, 3 launch-to-rest, 6 ordinary-surface, 3 hazard, 3 putter, 3 green-slope, 1 cup, 6 PRNG, 3 interaction comparisons and **81,920** course lookup cases.
- Continuous full original 18-hole run: [37965502008](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37965502008). Additional independent repeat with per-shot trajectory fingerprint: [37967817808](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37967817808).
- **Scope limitation:** parity on tested branches and one deterministic 18-hole route; not proof for every accuracy input, multiplayer branch or special terminal.
- **Do not infer camera/screen pixels** from engine coordinates. Rendering scale and art dimensions still require separate observation.

## 2. State, units and direction

The original ball's record advances in **44-byte** slots (0x2C). Recovered fields, relative to record base:

| Offset | Field | Notes |
| --- | --- | --- |
| +0x00, +0x04 | X, Y | signed 32-bit fixed-point-style, high word yields integer map coordinate |
| +0x08 | V | vertical force |
| +0x0C | H | horizontal force |
| +0x10 | D | direction, masked to 12 bits |
| +0x14 | Z | ball height |
| +0x18 | distance to cup | rounded/scaled helper |
| +0x1C | pause/timing | timer/terminal lifecycle (word-level detail depends on path) |
| +0x1E | surface identifier | 16-bit terrain descriptor |
| +0x26, +0x2A | slope direction/magnitude-like fields | lower-level map lookup provides active slope |

`D` lives in **0..4095**, wrapping mod 4096, one full revolution. Direction zero points toward increasing **Y**; quarter-turn 1024 points increasing **X**, inferred from recovered `sin(D)` and `sin(D+1024)` projections. The original Q14 trig values represent approximately `trunc(sin(2πD/4096) * 16384)` with the negative peak -16383. Signed arithmetic/right shift uses floor semantics for negatives, **not truncation towards zero**:

```text
vx_raw = floor((H * sin_q14(D)) / 16384)
vy_raw = floor((H * sin_q14(D+1024)) / 16384)
X += vx_raw; Y += vy_raw
D = (D - 2*swing_adjuster) & 4095  [when movement branch executes]
```

Intermediate multiply is wider than 32-bit and result is narrowed after shifting. Observable behavior is sensitive to ordering and signed shifts; a floating-point approximation can match the *look* but will not guarantee bit-exact deterministic trajectories.

## 3. Shot input and three-click meter

Authoritative request: `(club_index 0..12, aim 0..4095, power_tick 0..105, accuracy_tick)`.

- The first press starts power; the second captures power; the third captures accuracy.
- Welly power counter reaches **105 = 0x69**; original falling branch starts at 70 and converts its reading to power reaching 105 at the upper peak.
- Accuracy target is **63**, but note that the currently parity-integrated normal-shot launcher only supports the selected profile's limited accuracy range; **do not promise full extreme miss behavior yet**.
- Input/hit is accepted only in a ready-for-shot hole. Original shot simulator owns all movement, terrain and scoring.
- On launch: `error = accuracy_tick - 63`, `effective_power = max(0, captured_power - abs(error))`, `D = (aim - 16*error) & 0xFFF`.
- The club/lie selects a swing/curvature profile. For the verified profile range `even_error = floor(error/2)*2` and `swing_adjuster = profile_samples[even_error]`.
- `V0 = vertical_base[club] + power_scale[club]*effective_power`.
- `H0 = horizontal_base[club] + power_scale[club]*effective_power`.
- Club 12 follows a separate putter tick path *after* common launch initialization.
- Source: `engine/src/recovered_flight.cpp` and `docs/RECOVERED_PHYSICS_1_014.md`. Exact physical meter travel time/layout and screen frame appearance still need observational calibration.

## 4. Recovered 13-club numeric launch reference

These numbers are **reverse-engineering findings from the original executable**. Treat them as restricted reference inputs for legal and engineering review, **not** a licence to copy into Pixtee shipping code.

| Club index | Loaded vertical base V | Loaded horizontal base H | Power multiplier | Model path |
| ---: | ---: | ---: | ---: | --- |
| 0 | 8192 | 81920 | 3168 | loft/drive |
| 1 | 16384 | 73728 | 3120 | loft/drive |
| 2 | 36864 | 65536 | 3120 | loft/drive |
| 3 | 49152 | 49152 | 3072 | loft/drive |
| 4 | 65536 | 49152 | 2944 | loft/drive |
| 5 | 71680 | 49152 | 2880 | loft/drive |
| 6 | 77824 | 49152 | 2560 | loft/drive |
| 7 | 81920 | 49152 | 2496 | loft/drive |
| 8 | 112640 | 48128 | 2464 | loft/drive |
| 9 | 114688 | 48128 | 2368 | loft/drive |
| 10 | 131072 | 47104 | 2176 | loft/drive |
| 11 | 131072 | 40960 | 2112 | loft/drive |
| 12 | 0 | 8192 | 1536 | **putter**, separate ground tick |

The executable stores the first two base terms doubled; the loader halves them. Club index-to-name mapping must not be inferred from numeric ordering without an independently checked original menu/UI. Source: `engine/src/recovered_flight.cpp`, `engine/include/sensigolf/recovered_math.hpp`.

## 5. Accuracy, lie and draw/fade — recovered selector reference

The original uses a **13×10** club/lie selector into **11** swing profiles. Source is `engine/src/recovered_flight.cpp`, presented as factual recovered data, **not Pixtee runtime content**.

```text
clubs 0,1,2: 1 2 3 8 7 8 4 1 1 1
club 3:      1 2 4 8 7 8 4 2 2 2
clubs 4,5:   1 2 5 8 7 8 4 2 2 2
clubs 6,7:   2 3 5 8 7 8 4 2 2 2
club 8:      3 3 6 8 7 8 4 2 2 2
club 9:      4 4 6 8 7 8 4 4 4 4
club 10:     5 6 8 8 8 8 4 5 5 5
club 11:     6 7 8 8 8 8 4 5 5 5
club 12:     1 1 1 11 11 11 1 1 1 1
```

Profiles 1/2 accept accuracy **61..65**; profiles 3/4 **60..66**; profiles 5/6 **58..68**; profiles 7..11 **55..70**. The 8 sample columns correspond to evenized errors **-8,-6,-4,-2,0,+2,+4,+6**. Original profile samples:

```text
1:   0  0  0 -2  0  2  0  0
2:   0  0  0 -2  0  2  0  0
3:   0  0 -4 -2  0  2  0  0
4:   0  0 -1 -1  0  1  0  0
5:   0 -2 -1 -1  0  1  1  0
6:   0 -2 -1 -1  0  1  1  0
7:  -2 -2 -1 -1  0  1  1  1
8:  -2 -2 -1  0  0  0  1  1
9:  -2 -2 -1  0  0  0  1  1
10: -2 -2 -1  0  0  0  1  1
11: -2 -1 -1  0  0  0  1  1
```

This mechanism combines direct aim error, lost power and per-tick curvature; a simplistic accuracy-angle penalty would not match its characteristic flight. The original model's full out-of-profile accuracy behaviour is **unresolved** in the portable core.

## 6. Ball flight, frame-by-frame

Each original physics tick follows this recovered general path:

1. Read current ball state and surface at its current position.
2. Decrement vertical force `V -= 0x2100` (**8448** raw/tick).
3. Update `height += V`; on ground crossing, clamp to zero and enter landing resolution.
4. Otherwise apply normal horizontal drag `H -= 0x0F00` (**3840** raw/tick).
5. If H was positive and crossed below zero, clamp to 0 and **return without direction/movement that tick**. If H was already zero on entry, normal directional step may still execute.
6. Update direction for swing curvature, then add Q14 projected H to X and Y.
7. Repeat at logical interval `0x03A8` / 65536 seconds: **65536/936 ≈ 70.0171 Hz**. Drawing can be interpolated independently at 60/90/120Hz.

The exact flight uses signed integer/fixed-point arithmetic and branch ordering. Original coordinates/forces and renderer screen pixels are distinct. Wind-named debug state appears in v1.014, but original xref analysis found **no consumed wind vector in the released shot path**; Pixtee classic-style default should be **no wind**, with added wind isolated as optional new behavior.

## 7. Landing, rebound, rolling and stop

When `height` crosses the ground, the terrain code is checked. For ordinary bounce/roll:

```text
V = floor((-V) / 2)              [signed arithmetic right shift]
H = H + floor(V / 2)             [horizontal energy from bounce]
if H <= 0: H=0, V=0, ball rests
else: apply the normal drag/planar movement branch
```

A shot can bounce multiple times; if ground drag crosses H below zero it immediately re-enters landing resolution rather than advancing position. This detail prevents small nonphysical extra steps and matches original rest results. `H` and `V` are **forces in raw engine units**, not m/s or pixel/s. No fixed "bounce count" is imposed.

Normal terrain descriptions proven: SKIRT, FAIRWAY, SEMI ROUGH, ROUGH, V ROUGH, SAND. These alter the selected lie/profile and play context. Water/NO GO/OOB are handled separately; do not invent generic "every rough has 0.75 friction" coefficients—the original implementation relies on terrain descriptor and shot branch semantics.

## 8. Putting and green slope

Putter is **club 12**. On normal green landing-code 1:
- Height forced to zero, gravity bypassed.
- On each active putt tick, `H -= 0x0780` (**1920** raw/tick), exactly **half** the ordinary 0x0F00 drag.
- When H crosses below zero: zero H and V, mark at rest, **no movement or slope applied on that stopping tick**.
- While active: update direction, Q14 XY displacement plus green slope displacement.

Green slope vector originates from `slope_direction` and `slope_magnitude` extracted from original terrain metadata. Recovered multiplication model: a scaled magnitude times Q14 sin/cos is wrapped to 32-bit before an arithmetic right shift of 14 yields XY slope displacement. **The precise truncation/overflow ordering matters** for identical trajectories.

For a putt on ordinary *non-green* codes 2..7: no gravity and **full 3840 drag**, no green slope term. Landing codes 8/9/10 use special putter terminal handling, not ordinary rolling.

## 9. Green coordinate transformation and course lookup

The original detailed terrain is NOT the visible graphics tilemap. Gameplay uses separate **MAPM/MAPS/MAPI/SPT** logic:
- MAPM metadata header is **0x60** bytes. Tilemap `width,height` are big-endian 16-bit at offsets `0x54` and `0x56`.
- Logical MAPM tiles consume **16 X-coordinate units × 8 Y-coordinate units**. Each MAPI lookup tile ref describes **8×4** two-unit subcells.
- Two equal-sized MAPI banks (8-byte aligned) determine one of four descriptor words per subcell; 77 recovered terrain descriptors include normal lies, hazards and green variants.
- Green MAPS region transforms coordinates relative to a marker-derived origin and doubles local coordinates. On exiting the green window, the inverse transform applies. The playable green detail window and original green map bounds are distinct.
- A non-putter in green terrain landing-code 1 is currently rejected by the portable integration rather than fabricated; this does **not** prove that all original non-putter-on-green behaviours are completely recovered.
- SPT records 0..3 supply tee/start coordinates by player; record 4 supplies the cup position.

**Critical warning:** these tile dimensions are collision/world data indexing, NOT a measured original camera zoom or artwork pixel size. The camera's physical screen scale and scroll thresholds remain unmeasured.

## 10. Hazard relief, cups and score

- Water, NO GO, OOB hit recovered landing code **35 = 0x23**; contact immediately zeros V/H/height; hazard stop invokes **100 logical ticks** of pause. Safe-anchor tracking periodically keeps a prior playable point with an original -15-unit offset; recovery uses original course extents / green mode, then returns to play after acknowledgement.
- Landing-code **8** invokes cup terminal handling; for putter this transiently increments original counters before the later scored-hole adjustment.
- **The original zero-distance pre-update scoring branch is independent of code 8.** Resource-42 original green detail has no code-8 cells, but an ordinary shot can land at recovered distance zero, then score on the subsequent tick.
- The distance helper takes upper-word integer X,Y (with special green-mode coordinate halving), computes integer Euclidean norm and scales its integer square root by **6/10**. This rounding can yield zero despite a nonzero subunit positional difference; do not substitute float distance in the restoration.
- Original single-player score counters recovered: strokes `+0x52/+0x56`, par `+0x58`, par-minus-strokes `+0x48`, completed holes `+0x70`, 18-hole round completion when next zero-based hole index equals **18**.
- Landing-code **9/10** yields special-green terminal **event 11** with a 100-tick pause. **The continuation after the pause has NOT been original-oracle-verified; never assume drop, score, or restart behaviour.**

## 11. PRNG-dependent rare events

Recovered 16-bit state generator and three original mutation families:
- code-9 low-height deflection: approximately half-turn plus biased PRNG angular offset;
- near-hole lip deflection: horizontal force halved, vertical launched from previous H, heading randomized;
- flag-coordinate deflection: half-turn/bias plus unsigned vertical boost by one quarter.

The isolated mutations and PRNG are parity tested. **Their complete activation/dispatcher lifecycle is not necessarily fully integrated in every normal game-session shot.** This is an explicit high-fidelity backlog item, not permission to add approximations to restored physics.

## 12. Rendering and playback separation

The camera, sprites and UI are **not** part of Gate-1 physics verification. The reference engine exposes world X/Y, height, direction, velocities, surface and active/rest/terminal phases, while the host renders them. To reproduce perceived shot feel:
- sample actual reference clip frames to measure ball/avatar/green proportions;
- synchronize impact cues, rising ball sprite, arc, shadow and camera following **to authoritative ball ticks**, not frame-time-driven physics;
- interpolate only visuals between snapshots; do not feed interpolated position back into collision;
- maintain the same logical tick count when 60Hz/120Hz screens update, rotate or pause;
- test zoom on small phone and unfolded foldable separately.

## 13. Pixtee implementation boundary and route to actual fidelity

**Track A — Sensi Golf restoration:** Already owns the matching reference C++ engine; continue to ship a user-test build only subject to original assets licensing/import requirements.

**Track B — Pixtee Golf independent:** This numeric dossier is **restricted research**. The independent implementation must not copy recovered source text, generated trig arrays, literal tables or commercial MAPM/MAPS/MAPI/SPT assets without reviewing its IP/licensing basis. A wholly independent developer may use a gameplay-visible functional spec and black-box behaviour tests, then produce original physics parameterization and independently drawn courses. If exact recovered implementation and original tables are a non-negotiable commercial requirement, pursue appropriate rights and **call Pixtee a licensed remaster**, not a clean-room build.

No number of shifted bunkers, recoloured sprites or a changed name is automatically a safe legal threshold.

## 14. Original-source verification list

- `engine/src/recovered_flight.cpp` — real 13-club loaded constants, accuracy selector/samples, launch, air movement.
- `engine/src/recovered_ground.cpp` — contact, bounce, drag, putting, slope.
- `engine/src/recovered_distance.cpp` — exact integer distance helper.
- `engine/src/recovered_interactions.cpp`, `engine/src/recovered_prng.cpp` — recovered event mutations.
- `engine/src/recovered_hazard_recovery.cpp` — exact hazard/relief coordinate handling.
- `engine/src/classic_course_resources.cpp` — world/surface lookup and green coordinate transformations.
- `engine/src/classic_hole_session.cpp` — shot/hazard/cup/score phase changes.
- `analysis/evidence/gate1_final_signoff.json` — quantitative parity.
- `analysis/evidence/gate2_full_original_eighteen_hole_checkpoint.json` — complete deterministic round proof.
- `engine/tests/fixtures/original_round_all_eighteen.txt` — **restoration-only**, recorded numeric 18-hole shot inputs/tees/cups; not a new Pixtee course design.
