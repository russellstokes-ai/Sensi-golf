# Step 3 — Executable Analysis and Physics Recovery

Status: **LATE-STAGE — principal gameplay paths recovered and parity-tested**

Canonical current snapshot: [PROJECT_STATUS.md](PROJECT_STATUS.md).

## Objective

Recover the implementation of the classic shot and ball model rather than approximate it.

## Reference implementation

Primary:

- `GOLFWIN.EXE` v1.014 — PE32 and the principal machine-code oracle.

Cross-check:

- `GOLFDOS.EXE` from the same selected package.

Data:

- completely extracted `GOLF.EPF` — 277 entries.

Exact fingerprints are recorded in [STEP2_VERIFIED_RESULTS.md](STEP2_VERIFIED_RESULTS.md).

## Recovery completed

The analysis now covers:

- authoritative ball-state structure and stride;
- fixed-point X/Y handling;
- 4096-step heading system;
- exact clean-generated Q14 trig;
- 13-row club physics table;
- raw and live-player launch paths;
- Welly-o-meter power cap and timer/dispatcher chain;
- 11 accuracy profiles and selector matrix;
- straight/draw/fade power, heading and per-tick curvature;
- gravity and vertical integration;
- landing/bounce/roll;
- ordinary surface classes;
- green state, putting and slope projection;
- water / NO GO / out-of-bounds terminal paths;
- cup/hole capture;
- original ranged 16-bit PRNG;
- three PRNG-driven interaction mutations;
- course MAPI collision lookup;
- Windows wall-clock -> logical timer scheduling.

## Runtime parity completed

Original v1.014 machine code is now used as a numerical oracle rather than relying only on static inference.

Zero-tolerance coverage includes:

- representative straight/draw/fade shots from launch to final rest;
- six ordinary surface classes;
- hazards;
- putter;
- green slope;
- hole capture;
- PRNG outputs and interaction mutations;
- 81,920 course MAPI collision lookup cases.

See [GOLDEN_MASTER_HARNESS.md](GOLDEN_MASTER_HARNESS.md).

## Timing result

The Windows platform layer samples `GetTickCount`, converts elapsed milliseconds to 16.16 seconds as:

```
(elapsed_ms << 16) / 1000
```

and dispatches registered timer callbacks from that fixed-point accumulator.

The main gameplay timer is registered at interval `0x3A8`, equivalent to roughly 70.02 callbacks per second.

## Remaining analysis/sign-off

No major classic shot-physics formula remains to be invented.

Before formal Gate 1 closure:

- retain an end-to-end course lookup -> interaction activation -> PRNG mutation trace;
- run the complete current-head parity suite together;
- consolidate the evidence ledger and close Issue #1.

Additional naming/cross-check work after Gate 1 is allowed so long as it does not silently change parity-proven classic behaviour.

## Evidence standard

Every promoted gameplay claim requires:

1. binary/data location or unambiguous runtime evidence;
2. observed readers/writers/callers;
3. human-readable interpretation;
4. original-machine-code exercise where applicable;
5. parity coverage in the portable implementation.

## Stop condition

Do not replace unresolved classic behaviour with guessed modern golf physics. If a rule is ever found outside current coverage, it remains explicitly open until evidence resolves it.
