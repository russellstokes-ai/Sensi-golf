# Sensi Golf Enhanced

A preservation-first modern port/remaster investigation for **Sensible Golf**.

> **Current gate:** Gameplay/physics recovery  
> **Status:** PROVISIONAL GO — original binaries/data still need to be inspected and parity-tested.

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
- `tools/` — clean analysis/import utilities
- `tests/` — automated tests for tools and later gameplay parity
- `.github/workflows/` — CI
- `original/` — **local only**, ignored; licensed commercial game files must not be committed here

## Start here

1. Read [Gate 1 — Gameplay Recovery](docs/GAMEPLAY_RECOVERY_GATE.md).
2. Read [Original Input Policy](docs/ORIGINAL_INPUT_POLICY.md).
3. Put a legally held DOS game copy in a local ignored `original/` directory.
4. Run `python tools/hash_inputs.py original`.
5. Run `python tools/epf_inspect.py <archive.epf> --json`.
6. Record findings in [Analysis Log](docs/ANALYSIS_LOG.md).
7. Build controlled-shot traces described in [Parity Test Plan](docs/PARITY_TEST_PLAN.md).

## Scope after Gate 1

If parity succeeds, the project moves to a portable deterministic gameplay core, a modern renderer, Android/iOS input and display layers, then HD assets and enhanced features.

See [Roadmap](docs/ROADMAP.md).

## Rights

This public repository is for newly written tooling, documentation and port code. Original commercial executables, archives, graphics, music, course data and other licensed material stay out of the repository unless redistribution rights explicitly allow them.
