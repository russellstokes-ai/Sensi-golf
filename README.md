# Sensi Golf Enhanced

A preservation-first modern port/remaster investigation for **Sensible Golf**.

> **Step 2: COMPLETE** — original PC v1.014 identified, fingerprinted and unpacked.  
> **Current gate:** Gameplay/physics recovery — **GO**, with runtime golden-master parity still required before enhancement work.

## Project rule

We do **not** start HD art, mobile UI, new courses, progression or other enhancement work until the original gameplay model is recovered and numerically validated.

The target is not "similar arcade golf". The target is:

```
identical controlled input
        ↓
original Sensible Golf
        ↓
trajectory / landing / bounce / roll

vs.

identical controlled input
        ↓
portable recovered core
        ↓
trajectory / landing / bounce / roll
```

Only when representative outputs match within the documented tolerance does Gate 1 close.

## Verified original PC reference

Selected parity build:

- **Sensible Golf v1.014**
- `GOLFDOS.EXE` — 582,895 bytes
- `GOLFWIN.EXE` — 239,616 bytes
- `GOLF.EPF` — 833,273 bytes
- `GOLF.EPF`: **277/277 entries extracted successfully**

The commercial payload was processed in disposable/private analysis storage and is not committed to this public repository.

See:
- [Step 2 verified results](docs/STEP2_VERIFIED_RESULTS.md)
- [Recovered physics v1.014](docs/RECOVERED_PHYSICS_1_014.md)
- [Gameplay recovery gate](docs/GAMEPLAY_RECOVERY_GATE.md)

## Repository layout

- `docs/` — recovery plan, evidence, decisions and parity specifications
- `reference/` — metadata-only fingerprints of known historical builds
- `analysis/evidence/` — non-copyright recovery evidence
- `tools/` — clean analysis/import/recovery utilities
- `engine/` — portable C++17 recovered classic-core work
- `tests/` — automated tooling/core tests
- `.github/workflows/` — reproducible analysis and CI
- `original/` and `analysis/private/` — local/private only and ignored

## Recovery state

Completed or substantially recovered:
- PC build identity;
- complete EPF extraction;
- 72-hole data family discovery;
- PE/DOS binary cross-check anchors;
- ball structure and fixed-point coordinates;
- 4096-step direction system and exact Q14 trig;
- 13-row club physics table;
- launch-force formula;
- swing/accuracy profile tables;
- gravity;
- landing/bounce;
- normal and green rolling drag;
- terrain/slope projection path;
- deterministic portable math/kernel scaffolding.

Still required before Gate 1 closes:
- exact user-facing meter timing -> raw shot inputs where not yet fully proven;
- remaining surface/lie and hazard semantics;
- cup-capture/near-hole behaviour;
- logical simulation tick frequency;
- controlled original-game runtime traces;
- numerical golden-master parity through final rest.

## Step 2 ingestion tooling

A legal local PC ZIP can still be re-ingested reproducibly with:

```bash
python tools/ingest_pc_build.py /path/to/SENSEGOLF.ZIP
```

No original game payload is committed.

## Rights

This public repository is for newly written tooling, documentation and port code. Original commercial executables, archives, graphics, music, course data and other licensed material stay out of the repository unless redistribution rights explicitly allow them.
