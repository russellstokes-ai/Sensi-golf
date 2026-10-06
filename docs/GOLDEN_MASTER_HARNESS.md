# Golden-Master Gameplay Harness

## Purpose

A visual resemblance test is insufficient. The golden-master harness compares original Sensible Golf state evolution with the recovered core numerically.

## Stage 1 result — exact machine-code parity

The first oracle stage now passes.

The original Windows v1.014 PE machine code is executed directly in Unicorn for the live-player launch and normal clear-air paths, then compared with the portable C++ core.

Result:

- three controlled cases;
- 58 total state samples;
- X/Y/height/vertical force/horizontal force/direction compared;
- **zero mismatches**;
- **tolerance 0**.

See [GOLDEN_MASTER_AIRBORNE_RESULT.md](GOLDEN_MASTER_AIRBORNE_RESULT.md).

This is a subsystem golden master. It does **not** close Gate 1 because landing, real terrain, hazards, cup behavior and final rest are not yet covered.

## Full-shot trace contract

Canonical schema: `spec/shot_trace.schema.json`.

A full trace records:
- exact original build identity;
- course/hole;
- shot inputs;
- per-simulation-tick X/Y and optional height;
- landing point;
- final rest point;
- hazard and holed state.

## Full-shot comparator

```bash
python tools/compare_traces.py \
  original-shot.json recovered-shot.json \
  --xy-tolerance <native-units> \
  --event-tolerance <native-units>
```

A full-shot pass requires:
- every reference tick is present;
- no unexpected candidate ticks;
- maximum XY deviation within tolerance;
- landing/rest within tolerance;
- hazard outcome identical;
- holed state identical.

## Exact machine-code oracle

For recovered pure mechanics, the stricter stage-1 harness uses:

- `tools/original_v1014_oracle.py`
- `engine/tools/recovered_probe.cpp`
- `tools/compare_airborne_oracle.py`
- `.github/workflows/golden-master-airborne.yml`

The oracle reads club parameters from the original executable and executes the original x86 instructions. It does not copy expected numeric outputs into the portable implementation.

## Tolerance policy

Do not choose tolerances to make tests pass.

- Use zero tolerance for exact integer/fixed-point state where practical.
- For later black-box capture, use the smallest tolerance justified by capture precision.
- Keep measurement noise separate from implementation error.

## No invented fixtures

Synthetic traces in `tests/` validate comparators only. They are not gameplay evidence.

Machine-code oracle results and future real-game traces are stored as analysis artifacts; original commercial binary payloads remain out of the public repository.
