# Sensi Golf Enhanced

A preservation-first modern port/remaster investigation for **Sensible Golf**.

> **Current gate:** Gameplay/physics recovery  
> **Step 2:** engineering complete; awaiting ingestion of a legal original PC package.

## Project rule

We do **not** start HD art, mobile UI, new courses, progression or other enhancement work until the original gameplay model is recovered and validated.

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

Only when those outputs match within an agreed tolerance does Gate 1 pass.

## Repository layout

- `docs/` — recovery plan, evidence, decisions and parity specifications
- `reference/` — metadata-only fingerprints of known historical builds
- `tools/` — clean analysis/import utilities
- `tests/` — automated tests for tools and later gameplay parity
- `.github/workflows/` — CI definition
- `original/` and `analysis/private/` — **local only**, ignored

## Step 2 ingestion

Given a legally held original PC ZIP:

```bash
python tools/ingest_pc_build.py /path/to/SENSEGOLF.ZIP
```

The pipeline:
1. safely unpacks the ZIP;
2. SHA-256 and CRC-32 fingerprints every file;
3. compares it with the known TDC PC reference build;
4. identifies the DOS/Windows executable formats;
5. locates and inventories `GOLF.EPF`;
6. decompresses all EPF entries into the ignored private workspace;
7. classifies extracted files;
8. creates metadata-only JSON and Markdown reports.

No original game payload is committed.

See [Step 2 PC Input Inventory](docs/STEP2_PC_INPUT_INVENTORY.md).

## Start here

1. Read [Gate 1 — Gameplay Recovery](docs/GAMEPLAY_RECOVERY_GATE.md).
2. Read [Original Input Policy](docs/ORIGINAL_INPUT_POLICY.md).
3. Run Step 2 ingestion against a legal PC copy.
4. Record verified findings in [Analysis Log](docs/ANALYSIS_LOG.md).
5. Continue to controlled-shot and executable analysis in [Parity Test Plan](docs/PARITY_TEST_PLAN.md).

## Scope after Gate 1

If parity succeeds, the project moves to a portable deterministic gameplay core, a modern renderer, Android/iOS input and display layers, then HD assets and enhanced features.

See [Roadmap](docs/ROADMAP.md).

## Rights

This public repository is for newly written tooling, documentation and port code. Original commercial executables, archives, graphics, music, course data and other licensed material stay out of the repository unless redistribution rights explicitly allow them.
