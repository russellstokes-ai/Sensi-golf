# Original Sensi Golf numerical behaviour baselines — reference test vectors

**Scope:** Results observed and verified in the original restoration. Use these as **research / comparison checkpoints** for independent mobile gameplay; their presence does not authorize incorporating copyrighted original code, numeric arrays or original courses into a commercial clean-room game.

Source for every example is a checked-in C++ test; test results are part of Gate-1 parity and subsequent CI. These numerical values describe *internal engine units*, **not rendered screen pixels**.

## Launch and first airborne frame

From [`engine/tests/recovered_flight_tests.cpp`](../../engine/tests/recovered_flight_tests.cpp):

| Case | Inputs (club,lie,power,accuracy,aim) | Expected immediately after launch | After tick 1 |
| --- | --- | --- | --- |
| Straight power | 0, 0, 105, 63, 0 | adjusted power 105; swing 0; D 0; V 340832; H 414560 | V 332384; Z 332384; H 410720; X 0; Y 410720 |
| Left timing error | 5, 2, 83, 59, 777 | adjusted 79; swing -1; D 841; V 299200; H 276672 | D 843 |
| Right timing error | 11, 0, 60, 67, 3072 | adjusted 56; swing +1; D 3008; V 249344; H 159232 | -- |

- Trig sanity: `sin_q14(0)=0`, `sin_q14(1024)=16384`, `sin_q14(3072)=-16383`.
- Zero crossing: airborne V=100000, Z=200000, H=272, D=1023, swing=-1. After first tick H=0 and direction remains 1023; second tick direction becomes 1025 while X/Y stay 0. Any extra move on the crossing tick is a regression.
- Accuracy tick 80 is **out of currently supported profile bounds** in the recovered integration, so full extreme-error behaviour is not an exact covered baseline.

## Full straight shot and hazard timing

From [`engine/tests/recovered_ground_tests.cpp`](../../engine/tests/recovered_ground_tests.cpp) and [`engine/tests/classic_shot_model_tests.cpp`](../../engine/tests/classic_shot_model_tests.cpp):

| Input / terrain | First ground contact | End/rest | Terminal data |
| --- | ---: | ---: | --- |
| Club 0, lie 0, power 105, accuracy 63, aim 0, neutral flat | tick **80** | tick **152** | H=V=Z=0, final X=0, final Y=26251460 (raw) |
| Same shot on original hazard landing-code 35 | tick **80** | tick **80** | immediate hazard stop; H=V=Z=0 |

Hazard pause after terminal: 100 logical ticks (independent original session layer). These tests use synthetic flat/forced-hazard surfaces, **not** a complete original course bitmap.

## Putter — critical feel anchors

From [`engine/tests/recovered_ground_tests.cpp`](../../engine/tests/recovered_ground_tests.cpp):

| Original path | Input (club,lie,power,accuracy,aim) | Slope (direction,magnitude) | Rest tick | Final X raw | Final Y raw |
| --- | --- | --- | ---: | ---: | ---: |
| Green, short | 12,6,10,63,0 | 0,0 | **13** | 0 | **132864** |
| Skirt (not green) | 12,5,10,63,0 | not used | **7** | 0 | **60672** |
| Green, longer | 12,6,30,63,0 | 0,0 | **29** | 0 | **740096** |
| Green, north/south slope | 12,6,30,63,0 | 0,1 | **29** | 0 | **854784** |
| Green, sideways slope | 12,6,30,63,0 | 1024,1 | **29** | **114688** | **740096** |
| Green, diagonal slope | 12,6,30,63,0 | 512,3 | **29** | **243264** | **983360** |

Why these matter: exact speed and stopping differ **substantially** between green and skirt, even at the same raw power. Slopes change the final resting position without changing the sample's 29-tick duration. A new simulator should be evaluated for these qualitative relationships even if numeric internals are independently engineered.

## Rounded original distance rule

From [`engine/tests/recovered_distance_tests.cpp`](../../engine/tests/recovered_distance_tests.cpp):

- Ball (123,77), cup (123,77) => distance **0**.
- Ball (126,81), cup (123,77): Euclidean norm 5, original scaled distance **3**.
- Ball (124,78), cup (123,77): Euclidean norm approximately 1.414, original scaled distance **0**.
- Fractional 16.16 coordinate components do not affect integer-coordinate lookup.
- Green coordinate mode halves coordinates before extracting integer pair; its cup-relative origin must already be in the same reference space.

This is why a simple floating-point threshold test can disagree with the original hole completion.

## Hazard relief and special terminals

From [`engine/tests/recovered_hazard_recovery_tests.cpp`](../../engine/tests/recovered_hazard_recovery_tests.cpp) and [`engine/tests/recovered_putter_terminal_tests.cpp`](../../engine/tests/recovered_putter_terminal_tests.cpp):

- An in-bounds hazard example at (50,60) retains ball position, with player presentation point (35,45), height 0 and recovered pause 60.
- An out-of-bounds ball (500,600) using safe anchor (85,95) relocates ball to (100,110), with player presentation point (85,95) and pause 60.
- A code-8 putter terminal sets event **1**, pause **100**, transiently increments two stroke counters and sets flag 4. This is a terminal, **not itself full scored-hole completion**.
- Code-9/10 special-green putter terminal sets event **11**, pause **100**, without those extra transient stroke increments. Original post-pause continuation remains **UNRESOLVED**.

## Original full-course gameplay coverage

- Real original resource order: 42,50,58,38,70,44,64,48,25,52,40,60,68,17,66,56,46,72.
- One deterministic shot path per real hole is recorded in `engine/tests/fixtures/original_round_all_eighteen.txt`.
- C++ engine scored an 18-hole round with total 50 strokes against par 74, retaining score/hole state. This is a **solver-generated research run**, not a human leaderboard score.
- Latest double-run state fingerprint proof on `main` at `1cbdf99f`: 18252819364290903481 from two independent playthroughs; [run 37967817808](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37967817808). This fingerprint is engine-implementation-specific and should not be used as an expected Pixtee hash.

## Behaviour acceptance gates

1. **Shot launch:** independently reproduced shot trajectories respond to power, club choice, accuracy and aim as specified. Document comparative end-position, hang time, curvature and rest-time errors in each test.
2. **Bounce/rest:** no extra movement when H crosses below zero; no spurious bounce or negative-height drift.
3. **Green/skirt putting:** green rolls longer than skirt at equal power; slope direction is noticeable without a frame-rate dependency; no fake uniform roll-distance multiplier.
4. **Surface/lie:** fairway, semi, rough, heavy rough, sand, hazards have distinct player-visible behavior.
5. **Cup:** does not score merely because ball enters a visually painted flag sprite; test proper state-machine boundary.
6. **Controls:** genuine three presses, power/accuracy sampled from the same deterministic meter progression, no duplicate taps or mid-shot power changes.
7. **Mobile camera:** measure ball/flag/avatar scale and course visibility from visual references separately; never infer original pixels from world collision units.
8. **Risk audit:** isolated original event-11 branches, extreme accuracy cases and complete PRNG activation remain unresolved; no fabricated exactness.
9. **Commercial status:** shipping Pixtee cannot automatically include the recovered reference engine, numeric tables, original course files, graphics or sounds merely because the restoration is technically correct.

## Machine-readable reference and upkeep

- [`ORIGINAL_PHYSICS_RESEARCH.json`](ORIGINAL_PHYSICS_RESEARCH.json) includes the 13 recovered launch rows, 13×10 club/lie selectors, 11 bounds and swing profiles, and 77 terrain descriptors.
- [`ORIGINAL_PHYSICS_REFERENCE.md`](ORIGINAL_PHYSICS_REFERENCE.md) explains formulas, tick ordering, green conversion, distance, hazards and scoring.
- [`tools/validate_pixtee_physics_reference.py`](../../tools/validate_pixtee_physics_reference.py) verifies that these reference tables still agree with the verified original-restoration sources; a dedicated CI workflow runs this validator.
