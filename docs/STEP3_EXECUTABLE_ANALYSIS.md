# Step 3 — Executable Analysis and Physics Recovery

Status: **IN PROGRESS — substantial Windows v1.014 static recovery complete**

## Objective

Recover the implementation of the classic shot and ball model rather than approximate it.

## Reference implementation

Primary:
- `GOLFWIN.EXE` v1.014 — PE32 and the clearest static-analysis target.

Cross-check:
- `GOLFDOS.EXE` from the same selected package.

Data:
- completely extracted `GOLF.EPF` — 277 entries.

Exact fingerprints are recorded in [STEP2_VERIFIED_RESULTS.md](STEP2_VERIFIED_RESULTS.md).

## Completed static recovery

The current analysis has recovered or strongly mapped:

- authoritative ball-state structure and stride;
- fixed-point X/Y handling;
- 4096-step heading system;
- exact clean-generated Q14 trig;
- 13-row club physics table;
- club launch parameters;
- raw `DropPower` launch formula;
- swing/accuracy direction adjustment;
- 11 accuracy profile tables;
- gravity and vertical integration;
- landing/bounce;
- rolling drag;
- green state/bounds;
- terrain/slope projection;
- distance-to-hole path;
- DOS/Windows debug-state cross-check.

The portable C++17 math/kernel work intentionally implements only evidenced mechanics.

Detailed formula record: [RECOVERED_PHYSICS_1_014.md](RECOVERED_PHYSICS_1_014.md).

## Remaining static/dynamic closure

### Input mapping
- prove the complete user-facing upper-meter timing -> raw `DropPower` mapping;
- prove club/player -> accuracy-profile selection;
- determine whether any draw/fade curvature continues after launch.

### Terrain/game rules
- decode lie/surface IDs and modifiers;
- map obstacle/tree collision;
- map water/out-of-bounds transitions;
- recover cup capture / near-hole special branch.

### Timing/determinism
- determine logical simulation tick frequency;
- identify any PRNG dependency that affects shot outcomes.

### Runtime evidence
Capture controlled original v1.014 shots and compare per-tick state with the portable core using the golden-master harness.

## Evidence standard

Every promoted formula/table requires:
1. binary/data location or unambiguous runtime evidence;
2. observed readers/writers/callers;
3. human-readable interpretation;
4. an original-game trace exercising it where applicable;
5. parity coverage in the portable implementation.

## Stop condition

Do not fill unresolved behaviour with guessed modern golf physics. If a rule is not recovered, it stays explicitly open until binary/data/runtime evidence resolves it.
