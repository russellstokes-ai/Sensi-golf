# Sensi Golf Enhanced

A preservation-first modern port/remaster investigation for **Sensible Golf**.

> **Step 2: COMPLETE** — original PC v1.014 identified, fingerprinted and unpacked.  
> **Gate 1: GO** — static recovery is strong; live-player golden-master parity is in progress.

## Project rule

No HD art, mobile UI or enhancement work begins until original gameplay behavior is recovered and validated numerically.

## Verified original PC reference

- Sensible Golf Windows v1.014
- GOLFDOS.EXE — 582,895 bytes
- GOLFWIN.EXE — 239,616 bytes
- GOLF.EPF — 833,273 bytes
- GOLF.EPF — 277/277 entries extracted

Original commercial payloads remain outside the public repository.

## Parity status

A zero-tolerance machine-code oracle already verified the internal predictor/clear-air arithmetic for three cases. Later call-site tracing correctly classified that launch routine as an internal predictor, not the live player shot.

The live player launch is now established at `0x40AD1D`. It consumes:

- club index;
- lie/surface selector slot;
- raw Welly power;
- accuracy tick centred on 63;
- player heading.

The repo now contains the recovered 13x10 profile selector matrix, 11 profile bounds and signed swing profiles needed to reproduce that live launch path. The live-player zero-tolerance workflow targets this routine directly.

## Still required before Gate 1 closes

- live launch + airborne parity;
- landing/bounce/roll through final rest;
- terrain/lie semantics;
- hazards/obstacles;
- putting and cup capture;
- logical tick frequency;
- complete original-course shot traces.

See `docs/GAMEPLAY_RECOVERY_GATE.md` and `docs/GOLDEN_MASTER_HARNESS.md`.
