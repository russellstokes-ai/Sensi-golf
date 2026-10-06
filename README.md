# Sensi Golf Enhanced

A preservation-first modern port/remaster investigation for **Sensible Golf**.

> **Step 2: COMPLETE** — original PC v1.014 identified, fingerprinted and unpacked.  
> **Gate 1: GO** — the first exact original-machine-code parity stage now passes; full-shot parity through final rest remains required.

## Project rule

We do **not** start HD art, mobile UI, new courses, progression or other enhancement work until the original gameplay model is recovered and numerically validated.

The target is not "similar arcade golf". The target is original behavior reproduced deterministically.

## Verified original PC reference

Selected parity build:

- **Sensible Golf v1.014**
- `GOLFDOS.EXE` — 582,895 bytes
- `GOLFWIN.EXE` — 239,616 bytes
- `GOLF.EPF` — 833,273 bytes
- `GOLF.EPF`: **277/277 entries extracted successfully**

Original commercial payloads remain outside the public repository.

## First exact parity milestone

The live-player launch and normal clear-air paths are now **parity-verified** against the original v1.014 x86 machine code.

Three cases were executed through an original-code Unicorn oracle:

- straight club 0 / power 105;
- curved club 5 / power 83;
- opposite-sign curved club 11 / power 60.

Result:

**58 state samples × 6 fields, zero mismatches, tolerance 0.**

Compared fields:

- X;
- Y;
- height;
- vertical force;
- horizontal force;
- direction.

See [Golden Master — Live Launch and Clear-Air Result](docs/GOLDEN_MASTER_AIRBORNE_RESULT.md).

## Important correction

Parity preparation exposed an older interpretation error in the active core.

The normal live-player launch is `0x40C84F`, not the previously used `0x40AD1D` interpretation. The active C++ core has been corrected so raw `DropPower` drives launch force and swing/profile adjustment remains a separate path.

## Repository layout

- `docs/` — recovery plan, evidence and parity results
- `reference/` — metadata-only historical fingerprints
- `analysis/evidence/` — non-copyright recovery evidence
- `tools/` — clean analysis/oracle/import utilities
- `engine/` — portable C++17 classic core
- `tests/` — automated tests
- `.github/workflows/` — reproducible analysis and CI
- `original/`, `analysis/private/` — private/ignored

## Recovery state

Parity-verified:
- live-player non-putter launch;
- club launch scaling;
- initial heading;
- exact Q14 trig projection;
- clear-air gravity;
- normal clear-air drag;
- swing-adjuster direction stepping;
- X/Y fixed-point movement.

Recovered but not yet fully parity-verified:
- landing/bounce/roll;
- green drag and slope path;
- swing profile tables;
- terrain data structures.

Still required before Gate 1 closes:
- end-to-end meter/profile mapping;
- surface/lie semantics;
- obstacle/hazard behavior;
- putting/cup capture;
- logical tick frequency;
- original-course ground contact;
- complete shot traces through final rest.

## Rights

This public repository contains newly written tooling, documentation and port code only. Original commercial executables, archives, graphics, music, course data and other licensed material stay out of the repository unless redistribution rights explicitly allow them.
