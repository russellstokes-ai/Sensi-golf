# Golden-Master Gameplay Harness

## Purpose

A visual resemblance test is insufficient. The golden-master harness compares an original Sensible Golf shot with the recovered core numerically.

## Trace contract

Canonical schema: `spec/shot_trace.schema.json`.

A trace records:
- exact original build identity;
- course/hole;
- shot inputs;
- per-simulation-tick X/Y and optional height;
- landing point;
- final rest point;
- hazard and holed state.

Native coordinate units and tick rate remain unspecified until recovered.

## Comparator

```bash
python tools/compare_traces.py \
  original-shot.json recovered-shot.json \
  --xy-tolerance <native-units> \
  --event-tolerance <native-units>
```

A pass requires:
- every reference tick is present;
- no unexpected candidate ticks;
- maximum XY deviation within tolerance;
- landing/rest within tolerance;
- hazard outcome identical;
- holed state identical.

It also reports RMS XY error and, when both traces contain height, maximum Z error.

## Why exact tick sets initially

During recovery, differing tick counts can hide timing errors. The first classic core therefore has to expose the same logical simulation ticks as the reference. Rendering interpolation to 60/90/120 Hz belongs above this layer and is not part of physics parity.

## Tolerance policy

Do not choose tolerances to make tests pass.

After native representation is known:
- use zero tolerance for exact integer/fixed-point state where practical;
- otherwise use the smallest tolerance justified by capture precision;
- document any unavoidable measurement noise separately from implementation error.

## No invented fixtures

Synthetic traces in `tests/` validate the comparator itself. They are **not gameplay evidence**. Real golden masters enter the analysis set only after capture from the identified original build.
