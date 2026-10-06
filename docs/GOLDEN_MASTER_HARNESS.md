# Golden-Master Gameplay Harness

## Current status

The harness now distinguishes two original v1.014 paths correctly:

1. **Internal predictor:** `0x40C84F` / predictor simulation — zero-tolerance arithmetic parity already demonstrated.
2. **Live player launch:** `0x40AD1D` — current authoritative gameplay target.

The predictor result is useful evidence but does not count as live-player launch parity.

## Live-player oracle

`tools/original_v1014_oracle.py` executes the original v1.014 PE machine code directly under 32-bit x86 Unicorn.

Inputs:

- club;
- lie/surface selector slot;
- Welly power;
- accuracy tick;
- heading.

The original routine itself selects the appropriate accuracy profile, reduces power by accuracy error, alters heading and writes the swing-adjuster value.

The portable C++ core independently reproduces the same process.

Compared at zero tolerance:

- X;
- Y;
- height;
- vertical force;
- horizontal force;
- direction;
- swing adjuster;
- adjusted power.

Workflow: `.github/workflows/golden-master-airborne.yml`.

## Full-shot gate

After live launch/clear-air parity, extend the oracle through:

ground contact -> bounce -> roll -> final rest

and then exercise real course terrain, hazards, putter/green behavior and cup capture.

Synthetic comparator fixtures are never treated as gameplay evidence.
