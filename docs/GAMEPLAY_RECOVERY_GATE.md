# Gate 1 — Original Gameplay and Physics Recovery

Status: **GO — ready for final sign-off**

Canonical current snapshot: [PROJECT_STATUS.md](PROJECT_STATUS.md).

## Purpose

Before HD graphics, mobile presentation or enhancement features, prove that original Sensible Golf gameplay can be recovered faithfully enough for a modern port.

That feasibility question is now answered positively. The selected Windows v1.014 build is accessible to the analysis harness, the original machine code can be executed as an oracle, and the portable recovery matches the principal gameplay branches at zero tolerance.

## Recovery checklist

1. **Shot aiming / heading** — recovered and parity-tested.
2. **Welly power** — raw 0..105 path recovered; meter control flow, dispatcher and logical timer producer traced through the wall-clock source.
3. **Accuracy / draw / fade** — recovered and parity-tested, including profile selection, power penalty, heading and per-tick curve.
4. **Club table** — 13 physics records recovered; normal clubs and club-12 putter path parity-tested.
5. **Lie/surface penalties** — named ordinary surface classes exercised and parity-tested: skirt, fairway, semi rough, rough, very rough and sand.
6. **Airborne movement** — parity-verified.
7. **Course collision / obstacle lookup** — recovered MAPI lookup parity-tested exhaustively over 81,920 cases. Post-physics interaction branches are mapped separately.
8. **Landing / bounce / roll** — parity-verified through final rest.
9. **Water / NO GO / out-of-bounds** — parity-verified.
10. **Putting / green slope** — flat-green putter and controlled slope paths parity-verified.
11. **Hole/cup capture** — code-8 terminal branch parity-verified.
12. **Randomness** — original ranged 16-bit PRNG recovered and parity-tested; shot-affecting interaction mutations parity-tested.
13. **Logical simulation timing** — source recovered. `GetTickCount` deltas are converted to 16.16 seconds and the gameplay timer interval is `0x3A8`, about 70.02 Hz.
14. **Golden-master runtime parity** — achieved across the principal gameplay branches above.

## Representative zero-tolerance evidence

- generic launch-to-rest: run `37523005402`
- ordinary surfaces: run `37535123112`
- hazards: run `37535123148`
- putter: run `37535123117`
- green slope: run `37535123104`
- cup capture: run `37535155917`
- PRNG/shot interactions: runs `37545660767`, `37545660869`
- exhaustive course MAPI lookup: run `37545810759`
- meter/tick closure: run `37546007071`
- timing/meter dispatcher: run `37546141205`
- timer source closure: run `37546519169`

The original/reference files are not committed to the public repository.

## Collision/interaction boundary

The collision investigation corrected an early false lead: `WOOD1.BIN` through `WOOD4.BIN` are used by graphics-copy/rendering code rather than the authoritative ball collision path.

The recovered course-object path uses the MAPI lookup. Its portable implementation has been compared with original v1.014 machine code over 81,920 cases with byte-for-byte/zero-tolerance agreement.

Three original PRNG-driven state-mutation fragments are also parity-tested:

- code-9 low-height deflection;
- near-hole lip deflection;
- flag-coordinate deflection.

Their mutation arithmetic is verified. The final sign-off task is to preserve an end-to-end fixture in which the real lookup/activation path reaches one of these recovered interaction fragments.

## Timing boundary

The Windows timing layer uses `GetTickCount` and converts elapsed milliseconds to 16.16 seconds:

```
elapsed_fixed = (elapsed_ms << 16) / 1000
```

The main gameplay callback is scheduled at `0x3A8` fixed units, approximately 70.02 Hz.

This lets the port preserve classic simulation cadence while keeping rendering interpolation independent.

## Gate 1 exit condition

Gate 1 may be marked COMPLETE when:

- the full current-head golden-master/core test set passes in one consolidated sign-off run;
- at least one complete lookup -> interaction activation -> state-mutation trace is retained as evidence;
- the final evidence ledger and Issue #1 are reconciled.

No modern substitute physics are required or permitted for the recovered classic mode.
