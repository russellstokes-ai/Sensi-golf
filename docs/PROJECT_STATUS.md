# Project Status

Last updated: **2026-10-07**

This is the canonical current-status document for the Sensible Golf Enhanced recovery/port project. Specialist documents contain deeper technical detail, but this page is the authoritative summary of what is complete, what is parity-proven, and what remains in the active mobile-first integration work.

## Executive status

- **Step 2 — original PC acquisition/inventory: COMPLETE**
- **Gate 1 — original gameplay/physics recovery: COMPLETE / GO**
- **Gate 2 — complete classic game/core integration: IN PROGRESS**
- Gate 1 is formally signed off. Gate 2 now builds the complete platform-neutral classic game/session layer for the mobile product. Android is the first actual playable product target; desktop executables are optional developer/test utilities only.

The project-killing feasibility question is now answered positively: the selected Windows v1.014 gameplay engine is recoverable and the portable core reproduces the original machine code at zero tolerance across the principal shot, terrain, hazard, putting and cup branches.

Gate 1 was formally closed on 2026-10-07 after a consolidated current-head sign-off passed all existing parity/core coverage and an end-to-end course-interaction trace joined the real MAPI lookup to code-9 activation and the recovered PRNG mutation.

## Verified reference build

Windows reference:

- `GOLFWIN.EXE`
- version: **1.014**
- size: **239,616 bytes**
- CRC-32: `23c300b4`
- SHA-256: `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`

Companion DOS executable:

- `GOLFDOS.EXE`
- size: **582,895 bytes**
- SHA-256: `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed`

Game archive:

- `GOLF.EPF`
- size: **833,273 bytes**
- SHA-256: `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`
- **277/277 entries extracted**
- original commercial payloads remain outside the public repository.

## Gate 1 parity matrix

| Area | Status | Evidence |
|---|---|---|
| Aim / heading / trig | Parity-verified | 4096-angle direction model and exact Q14 trig |
| Club launch physics | Parity-verified | 13-row original club table |
| Welly-o-meter power path | Recovered | raw 0..105 power path plus timer/dispatcher mapping |
| Accuracy / draw / fade | Parity-verified | live profile selection, power penalty, heading and per-tick curve |
| Airborne flight | Parity-verified | zero-tolerance original-machine-code traces |
| Landing / bounce / roll | Parity-verified | launch-to-rest golden masters |
| Ordinary surfaces | Parity-verified | skirt, fairway, semi rough, rough, very rough, sand |
| Hazards | Parity-verified | water, NO GO and out-of-bounds |
| Putter | Parity-verified | flat-green club-12 path |
| Green slope | Parity-verified | controlled slope vectors |
| Cup/hole capture | Parity-verified | code-8 terminal branch |
| PRNG | Parity-verified | original 16-bit ranged PRNG |
| PRNG shot interactions | Parity-verified fragments | code-9, near-hole lip and flag-coordinate mutations |
| Course collision lookup | Parity-verified | exhaustive MAPI lookup, 81,920 zero-tolerance cases |
| Logical timing source | Recovered | Windows wall clock -> 16.16 timer scheduler |
| Wall-clock cadence | Recovered | interval `0x3A8` 16.16 seconds, about 70.02 Hz |

## Key successful workflow evidence

Representative successful runs:

- launch-to-rest generic flat: `37523005402`
- ordinary surface parity: `37535123112`
- hazard parity: `37535123148`
- putter parity: `37535123117`
- green slope parity: `37535123104`
- hole capture parity: `37535155917`
- PRNG / interaction parity: `37545660767` and `37545660869`
- exhaustive course collision lookup: `37545810759`
- meter/tick closure: `37546007071`
- final timing/meter dispatch: `37546141205`
- timer source closure: `37546519169`
- end-to-end course interaction: `37603087138`
- consolidated Gate 1 sign-off: `37603222703`
- CI on the sign-off commit: `37603222791`

## Important recovered timing result

The Windows scheduler converts `GetTickCount` milliseconds into 16.16 seconds using:

```
elapsed_fixed = (elapsed_ms << 16) / 1000
```

The gameplay timer callback is registered with interval:

```
0x3A8 = 936
```

in those 16.16-second units, equivalent to roughly **70.02 logical callbacks per second**.

The recovered simulation should therefore preserve original logical update cadence while modern rendering may interpolate independently at 60/90/120 Hz.

## Important collision result

`WOOD1.BIN` through `WOOD4.BIN` were investigated and proved to feed graphics-copy/rendering paths, not the authoritative ball collision logic.

The course collision path is instead driven by the MAPI course lookup. The recovered portable lookup matches original v1.014 machine code over **81,920 cases at zero tolerance**.

Separate post-physics interaction branches use the recovered original PRNG. Their state mutations were parity-tested independently and then joined to the real course/object lookup in the final Gate-1 end-to-end fixture.

## Gate 1 closure

Final sign-off run `37603222703` passed on commit `34bfa1c0999d2e5e196ce900edc9e2bd563ed3d8`.

The run rebuilt the portable core from a clean checkout, reran Python and C++ tests, and reran the complete parity matrix against the verified v1.014 executable/data. It recorded:

- 3 launch-to-rest straight/draw/fade cases;
- 6 ordinary surface cases;
- 3 hazard cases;
- 3 putter cases;
- 3 green-slope cases;
- 1 hole-capture case;
- 6 PRNG sequence samples;
- 3 PRNG-driven interaction branches;
- 81,920 MAPI collision lookup cases;
- the real MAPI -> landing-code 9 -> event 11 -> PRNG interaction integration fixture;
- 20 zero-tolerance comparison files;
- all unit/core tests passing.

Durable evidence:

- `analysis/evidence/gate1_final_signoff.json`
- `analysis/evidence/gate1_course_interaction_parity.json`

**Gate 1 is COMPLETE.**

## Non-gating cleanup

These items may continue after Gate 1 without threatening gameplay fidelity:

- final human-readable naming of every internal club/data index where labels are not independently trustworthy;
- broader DOS-vs-Windows behavioural cross-checks;
- additional course/hole regression fixtures;
- packaging the recovered timing/meter logic behind the future desktop/mobile input layer.

## Active phase — Gate 2

Gate 2 is **Complete classic game/core integration**, not a desktop product.

The target architecture is:

```
parity-proven C++ classic simulation
        ↓
platform-neutral course / hole / session layer
        ↓
Android NDK/JNI host
        ↓
mobile input + rendering + audio
```

### Gate 2 progress

**Burst 1 — platform-neutral shot-model bridge: COMPLETE**

- production `ClassicShotModel` implements `IClassicModel`;
- original 0..4095 direction units preserved;
- exact scheduler interval `0x3A8` exposed;
- normal shot, putter and hazard terminal paths covered;
- Gate-1 golden masters stayed green.

Primary commit: `8000141a11f8e34d5ad99879a4003507a82552ee`.

**Burst 2 — course/resource model: COMPLETE**

- real MAPM/MAPI/SPT bundle loader;
- MAPM dimensions and tile lookup;
- parity-proven MAPI subcell lookup from recovered 16.16 ball coordinates;
- 77 original terrain descriptors available;
- SPT records 0..3 recovered as per-player tee/start coordinates;
- SPT record 4 recovered as cup/hole coordinates;
- original commercial files remain external;
- real v1.014 course-resource validation passes.

Validated checkpoint: `97c89e13ca0f829b9977ff6b887ef7217235d0f8`.

Successful validation evidence:
- Gate-2 course resource validation `37616105293`;
- CI `37616105377`;
- all Gate-1 golden masters green on the same checkpoint.

**Burst 3 — hole/session state machine: IN PROGRESS**

Implemented:
- tee -> shot -> moving terrain lookup -> rest lifecycle;
- stroke counting;
- hazard terminal state;
- hole terminal state;
- flat-green club-12 session path;
- explicit `UnsupportedTerrain` stop instead of invented rules;
- real SPT tee/cup initialization.

Current Burst-3 recovery/integration:

- putter code-8/9/10 terminal oracle and product integration complete;
- code 8 maps to the cup terminal outcome;
- codes 9 and 10 map to the original special-green stop outcome rather than being approximated as ordinary green roll;
- original hazard terminal pause of 100 logical ticks is integrated;
- original post-hazard position recovery is integrated, including the safe-anchor path and recovered course extents;
- normal-shot trajectory for codes 10/50/60 remains zero-tolerance parity-proven;
- original per-player scoring/stroke fields are now traced further, but final semantic naming/next-hole ownership is not yet promoted.

New evidence:
- putter terminal integration commit `0f83c2f89e0db324d0fbad0304088d26104eb7bd`;
- hazard recovery integration commit `37e2f16cbf1420f4a83b59072f08c0915b68f45e`;
- scoring-state analysis `37623146366`;
- stroke-counter trace `37623430352`;
- hole-completion semantic check `37623654428`;
- CI on current head `37623654112` — PASS.

Important scoring/completion boundary:

- both recovered per-player counters at offsets `+0x52` and `+0x56` increment on a normal shot launch;
- the scoped code-8 putter terminal increments those counters again, while code-9/10 special terminals do not;
- original score-transfer code subtracts `+0x52` from `+0x56` and clears `+0x52`;
- hole-result presentation uses the current stroke field or mode-specific score source and compares it with course par, clamped to the original display range;
- cup capture/terminal state and the later full hole-completion/score/next-state routines are separate original flows.

The session therefore must not equate “ball entered cup” with “all scoring/next-hole state has already advanced.”

Current burst plan is recorded in `docs/GATE2_MOBILE_INTEGRATION.md`.

A desktop executable may be used internally for tests/debugging, but it is not a product milestone.
