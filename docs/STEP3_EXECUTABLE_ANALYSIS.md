# Step 3 — Executable Analysis and Physics Recovery

Status: **ANALYSIS HARNESS READY — original executable required for evidence work**

## Objective

Recover the implementation of the classic shot and ball model rather than approximate it.

## Analysis order

1. `GOLFWIN.EXE` first when the selected build contains it.
   - PE32 has explicit sections and standard tooling.
   - Locate strings/data references and candidate numeric tables.
2. `GOLFDOS.EXE` second.
   - Cross-check shared constants/tables and behaviour.
   - Distinguish genuine gameplay rules from Windows-port-specific implementation.
3. `GOLF.EPF` extracted data.
   - Determine which gameplay parameters live in data versus executable code.

## New probe

```bash
python tools/executable_probe.py GOLFWIN.EXE --number 240 -o analysis/private/golfwin-probe.json
python tools/executable_probe.py GOLFDOS.EXE --number 240 -o analysis/private/golfdos-probe.json
```

The `240` search is only a useful anchor because the manual documents a 240-yard maximum-power 1 Wood. A hit is **not** evidence by itself; it becomes meaningful only when referenced by code or adjacent to a coherent club table.

The probe records:
- SHA-256 and CRC-32;
- executable format;
- PE section metadata;
- printable strings and offsets;
- requested 16/32-bit integer occurrences.

It intentionally does not claim to be a decompiler.

## Recovery sequence

### A. Find state anchors

Search for:
- club names or club-selection UI text;
- lie/surface labels;
- score/hole strings;
- maximum-distance display strings/numbers;
- Welly-o-meter related UI resources.

Trace cross-references from those anchors to candidate state variables/tables.

### B. Identify authoritative ball state

Find the variables updated while a shot is active:
- horizontal position;
- height;
- direction;
- velocity or per-tick deltas;
- flight/ground state;
- surface;
- bounce/roll state.

Do not name a field until observed reads/writes support the name.

### C. Recover shot initialization

Map:
- aim input -> heading;
- selected club -> base parameters;
- upper meter timing -> power;
- lower meter timing -> straight/draw/fade;
- lie -> modifier.

### D. Recover per-tick update

Separate:
- airborne update;
- curvature;
- collision query;
- landing;
- bounce;
- ground roll;
- hazard/cup termination.

### E. Putting

Treat putting independently if the original does:
- power mapping;
- surface/slope vector;
- friction/deceleration;
- cup capture.

## Evidence standard

Every recovered formula/table must have:
1. binary/data location;
2. observed callers/readers;
3. human-readable interpretation;
4. one or more original-game traces that exercise it;
5. parity test against the portable implementation.

## Stop condition

If the executable cannot be obtained legally, Step 3 can be prepared but not honestly completed. We do not fill unknown physics with guessed values.
