# Golden-Master Gameplay Harness

## Current status

**Live-player launch-to-rest parity is now proven for the controlled generic-flat path.**

The oracle executes original Sensible Golf Windows v1.014 machine code directly under 32-bit x86 Unicorn. The portable C++17 core is compared against it at **zero tolerance**.

## Verified full-shot cases

Workflow: `.github/workflows/golden-master-flat-rest.yml`  
Successful run: **37523005402**

| Case | Input summary | Samples | Result |
|---|---|---:|---|
| straight_1w | club 0, lie 0, power 105, accuracy 63, heading 0 | 153 | exact |
| draw_mid | club 5, lie 2, power 83, accuracy 59, heading 777 | 131 | exact |
| fade_high | club 11, lie 0, power 60, accuracy 67, heading 3072 | 111 | exact |

Compared every logical sample for:

- X;
- Y;
- height;
- vertical force;
- horizontal force;
- direction;
- swing adjuster;
- adjusted power.

Landing and final-rest events also match exactly.

Representative event anchors:

- straight 1W: landing tick 80 at (0, 20615840), rest tick 152 at (0, 26251460);
- draw mid: landing tick 70 at (9595717, 2006927), rest tick 130 at (10661268, 2038783);
- fade high: landing tick 59 at (-3188529, -454500), rest tick 110 at (-3759359, -636210).

## Important recovered edge case

The draw test exposed a one-tick quirk in the original drag branch. If horizontal force is positive at tick entry but normal drag pushes it below zero while the ball is airborne, v1.014 sets horizontal force to zero and ends the tick **without** applying the direction/curve step. On the next tick, an already-zero force does pass through the direction step.

The portable core now preserves this exact behavior.

## Scope of this proof

The controlled oracle supplies a neutral generic terrain result (surface code 0). Everything from live launch through flight, landing, bounce, drag, movement and final rest is original machine code versus independently recovered portable code.

This proof does **not** yet cover:

- every terrain/lie modifier;
- tree/obstacle collision;
- water/out-of-bounds;
- green/putter behavior;
- cup capture;
- any remaining random/state-dependent game rules.

Evidence record: `analysis/evidence/full_shot_generic_flat_parity.json`.
