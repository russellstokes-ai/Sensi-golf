# Project Status

Last updated: **2026-10-07**

This is the canonical current-status document for the Sensible Golf Enhanced recovery/port project. Specialist documents contain deeper technical detail, but this page is the authoritative summary of what is complete, what is parity-proven, and what remains in the active mobile-first integration work.

## Executive status

- **Step 2 — original PC acquisition/inventory: COMPLETE**
- **Gate 1 — original gameplay/physics recovery: COMPLETE / GO**
- **Gate 2 — complete classic game/core integration: IN PROGRESS**
- Gate 1 is formally signed off. Gate 2 is building the platform-neutral classic game/session layer that the mobile product will host. Android remains the first actual playable product target; desktop executables are optional developer/test utilities only.

The project-killing feasibility question has been answered positively: the selected Windows v1.014 gameplay engine is recoverable and the portable core reproduces the original machine code at zero tolerance across the principal shot, terrain, hazard, putting, cup, PRNG and course-collision branches.

The current Gate-2 checkpoint has now moved beyond shot/hole terminal recovery into **scored-hole and cross-hole round progression**. Cup capture, scored-hole activation and next-hole/round state are deliberately modeled as separate recovered stages, matching the original program rather than collapsing them into one invented event.

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

## Key Gate-1 workflow evidence

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
- consolidated Gate-1 sign-off: `37603222703`
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
platform-neutral course / hole / round layer
        ↓
Android NDK/JNI host
        ↓
mobile input + rendering + audio
```

### Burst 1 — platform-neutral shot-model bridge: COMPLETE

- production `ClassicShotModel` implements `IClassicModel`;
- original 0..4095 direction units preserved;
- exact scheduler interval `0x3A8` exposed;
- normal shot, putter and hazard terminal paths covered;
- Gate-1 golden masters stayed green.

Primary commit: `8000141a11f8e34d5ad99879a4003507a82552ee`.

### Burst 2 — course/resource model: COMPLETE

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

### Burst 3 — hole/session/round state: IN PROGRESS

#### Integrated

- tee -> shot -> moving terrain lookup -> rest lifecycle;
- stroke counting and recovered player-counter ownership;
- flat-green club-12 path;
- code-8 cup terminal;
- code-9/code-10 special-green terminals;
- original hazard terminal pause of 100 logical ticks;
- safe-anchor tracking and recovered post-hazard position relocation;
- exact original distance-to-hole helper `0x40B998`;
- separate `CupTerminal` and `HoleScored` phases;
- recovered single-player score update;
- recovered next-hole ownership and end-of-round condition;
- cross-hole `ClassicRoundSession` score/progression owner;
- rejection of stale/duplicate hole results;
- 18-hole round progression regression coverage.

#### Scored-hole boundary now recovered

The original does **not** equate landing-code 8 / cup capture with the complete scored-hole state.

The recovered sequence is now modeled as:

```
code-8 cup terminal
        ↓
later distance-to-hole pre-update
        ↓
recovered distance value == 0
        ↓
single-player counter correction
        ↓
par / relative-score update
        ↓
HoleScored
        ↓
next-hole routine
```

The portable `ClassicHoleSession` uses optional `ClassicHoleMetadata` containing the zero-based hole index and par. On the currently proven single-player putter path, a cup terminal is promoted on a later tick only when the recovered distance helper returns zero.

#### Recovered single-player score fields

The currently promoted roles are:

- player `+0x52`: current-hole stroke counter;
- player `+0x56`: stroke total used by the recovered score calculation;
- player `+0x58`: cumulative par;
- player `+0x48`: cumulative par minus cumulative strokes;
- player `+0x70`: completed-hole counter.

The code-8 putter terminal has a transient extra increment to `+0x52/+0x56`; the recovered single-player turn-adjust path removes that increment before scoring.

#### Next-hole / round ownership

Original routine `0x40935B` increments the current zero-based hole index. If the incremented value equals **18**, it sets the round-finished state; otherwise it performs next-hole setup.

This ownership is now carried through the portable layer. `ClassicRoundSession` validates that an accepted scored hole has the expected metadata/index and recovered next-hole state before aggregating its score contribution. This prevents stale or duplicate result application.

Merged implementation checkpoint:

- scored-hole integration: `d42a9e1b8c380cd7358c6205b4a1d51d9ce99c7a`;
- round progression PR: **#5**;
- merged round checkpoint: `53c27378cd66c68c6037fbe228a169ab375d2f45`.

#### Verification of the round checkpoint

Final branch checkpoint `9aababcd85b4ca89c0d675351787de0dc7f66a1c` passed every requested verification run:

- push CI: `37672902841`;
- PR CI: `37672942401`;
- Gate 2 course resource validation: `37672902177`;
- Gate 2 hazard recovery parity: `37672902138`;
- Gate 2 putter terminal parity: `37672902332`;
- Golden Master putter parity: `37672902330`;
- Golden Master live player: `37672902586`;
- Golden Master normal surfaces: `37672902230`;
- Golden Master hazards: `37672902392`;
- Golden Master flat landing/rest: `37672902517`;
- Golden Master hole capture: `37672902350`;
- Golden Master green slope: `37672902256`;
- Golden Master PRNG parity: `37672902219`;
- Golden Master PRNG shot interactions: `37672902119`;
- Golden Master course collision lookup: `37672902433`.

**Result: all green; no Gate-1 fidelity regressions.**

Durable evidence is recorded in `analysis/evidence/gate2_burst3_round_checkpoint.json`.

#### Active remaining Burst-3 work

1. close the post-hazard terminal/gameflow handoff, including terminal-flag release, penalty/turn ownership and return to the playable state;
2. wire the recovered next-hole index into real next-hole resource/setup selection rather than stopping at progression state;
3. close remaining unsupported/special green continuations and prove any required non-putter scored-hole continuation;
4. move original PRNG seed ownership into the session where required for complete deterministic hole replay;
5. prove a complete **real-course** hole from original SPT tee through cup, scoring and next-hole setup using the original resource bundle.

Current hazard-gameflow recovery work is already being traced on `main` through the dedicated Gate-2 workflow; the trace has been extended through the post-hazard handoff, terminal flag release and presentation-timer ownership.

Burst 3 remains **IN PROGRESS** until those paths are closed. The mobile host should not begin reimplementing any of them independently.

A desktop executable may be used internally for tests/debugging, but it is not a product milestone.
