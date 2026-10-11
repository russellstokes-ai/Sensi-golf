# Sensible Golf v1.014 — ACTUAL GAME-FILE HARD DATA (canonical source index)

**Status: original-file re-extraction verified, 10 Oct 2026.** This is a source-data ledger, **not** a manual-derived gameplay brief and **not** proof that the separately implemented Pixtee engine already matches it.

- Original source **GOLFWIN.EXE** SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`.
- Original game resource **GOLF.EPF** SHA-256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`.
- Authentic archive ingestion: 277/277 resources extracted in a disposable ignored private workspace; original commercial asset bytes were **never committed**.
- **NEW** independent source-file verification: [Actions #38089050718](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38089050718) — **PASS**, from commit `04c5980cdcc3027ac39ceb571225f42b70b2e416`.
- Original-produced metadata report: [artifact #11683242409](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38089050718/artifacts/11683242409), ZIP digest `sha256:0358b7680784693b2830e606be33259f98e94c5140807278c72b5ac027766121`. Expires after 30 days; generation scripts and the separately committed numerical research tables are durable, reproducible evidence.
- **Canonical permanently saved tables:** [ORIGINAL_PHYSICS_RESEARCH.json](ORIGINAL_PHYSICS_RESEARCH.json): 13 original club launch rows, **13×10** club/lie selector, 11 accuracy bounds and 11 swing/curvature sample rows, **77 terrain descriptors**, 70.017Hz mechanics and constants. This table was compared to source C++ and **re-extracted from the original executable**, with exact-data equality asserted in the new CI job. Do not treat the developer's approximate `PixteeCore.kt` constants as authoritative.

## Raw numeric club data recovered directly from the Windows binary

The executable at table VA `0x41F108` contains exactly 13 entries of three little-endian signed 32-bit numbers. The original launcher halves the first two when loading initial vertical/horizontal base force. The third is the club's power multiplier. **Index 12 is putter. The exact original printed names for indices 0–11 have NOT been verified**, so do not invent a mapping.

| Index | Loaded vertical base | Loaded horizontal base | Power scale |
|---:|---:|---:|---:|
| 0 | 8192 | 81920 | 3168 |
| 1 | 16384 | 73728 | 3120 |
| 2 | 36864 | 65536 | 3120 |
| 3 | 49152 | 49152 | 3072 |
| 4 | 65536 | 49152 | 2944 |
| 5 | 71680 | 49152 | 2880 |
| 6 | 77824 | 49152 | 2560 |
| 7 | 81920 | 49152 | 2496 |
| 8 | 112640 | 48128 | 2464 |
| 9 | 114688 | 48128 | 2368 |
| 10 | 131072 | 47104 | 2176 |
| 11 | 131072 | 40960 | 2112 |
| 12 | 0 | 8192 | 1536 |

**These are raw engine-force parameters, not displayed yardage or loft in degrees.** Distances are outcomes of entire per-tick simulated shots, terrain, power and accuracy. No invented `yards=275` or `loft=22` should be passed off as extracted original data.

## Other hard data extracted / verified

| Class | Direct executable or game resource | Permanently audited data | Exact behaviour / caveats |
|---|---|---|---|
| Power and accuracy | v1.014 meter routines + launcher | raw power **0..105**, ideal accuracy **63**; actual power/error transformations | Original hit sampling and ball physics are proven for tested cases. Precise horizontal mapping, red/yellow/black visual pixel boundaries, whole cursor timing still need binary/art frame verification. |
| Swing curvature | original tables VA `0x41D4C4`, `0x41D546`, `0x41F204` | complete verified 13×10 selector, 11 bounds and signed sample rows in JSON | The original changes heading and per-tick curvature; extreme/out-of-profile shots still need extra oracle cases. |
| Ground/flight | v1.014 update routines, golden masters | gravity **8448/tick**, ordinary horizontal drag **3840/tick**, green-putt drag **1920/tick**; bounce is half vertical energy with a quarter-energy transfer into horizontal component | **True repeated real bounces** and roll. No artificial one-bounce cap, no fake rolling timeout. Original fixed timer interval **936/65536 sec**. |
| Course lie/surfaces | original terrain descriptor table + `MAPM/MAPS/MAPI` collision files | **77** descriptor records: landing code, variant and lie profile slot; distinguish skirt/fairway/semi/rough/very rough/sand, green, water/OOB and terminal codes | No bitmap-colour guesswork. Relevant original-file exact descriptors in committed JSON; derived collision values in CI artifact. |
| Green arrows / slope | `MAPI01.RAW` and `MAPI02.RAW` plus all 72 `MAPSnn.MAP` | Each selected 16-bit descriptor word: low 8 bits terrain descriptor; upper byte slope direction low nibble (**nibble × 256** in 4096-angle units) and magnitude high nibble (**0..15**). New artifact includes actual 72-resource green **slope-direction/magnitude and terrain distribution histograms**. | Original putting slope vector behaviour in `recovered_ground.cpp` is traced/tested. The rendered green-arrow sprite frame mapping/timing is **not** extracted yet; don't invent it. |
| Hole tee/cup | 72 `MAPMnn.SPT` files | Original per-source 4 tee/player start points + 1 cup point; verified in new artifact. Original 18-hole deterministic round fixture also saved separately. | Original protected course-layout geometry is **research only** and not importable to independent Pixtee without rights. |
| Hazards | `MAPM/MAPS/MAPI` + original terminal/state code | water/no-go/out-of-bounds collision code **35**, immediate stop, original **100 logical tick** hazard pause, documented safe-anchor/relocation logic | Sand is **not water**: nonterminal surface/lie (code 7) and club/lie profile 0; actual bounce/roll uses the original ground branch. Original score/penalty transitions remain a separate state-machine gate. |
| Cup and putting | original code 8 terminal, original zero-distance check, special codes 9/10 | original integer distance scale **6/10**, putter index **12**, controlled green vs skirt rest ticks **13 vs 7** for same sample power | Do not replace with ball-within-5-world-units short-circuit or max 12 strokes. Special code 9/10 post-stop continuation remains open. |
| 3D-looking impacts / trees | original post-physics hit-pass, `WOOD1..4.BIN` graphics assets | `WOOD1..4.BIN` confirmed to serve graphics rendering, **not** as ball collision masks. Relevant MAPI lookup and isolated original interaction mutations parity-tested | **Full tree-impact classification, collision height response, deflection activation and tree hit animation timing are not extracted/verified**. Do NOT claim completed tree parity or arbitrarily make every decorative tree a hard circular collider. |
| Animations/audio | 15 original MCH, 19 LBM resources plus other data/sfx files | New artifact provides **56 original graphics/audio entry names, sizes and hashes**; this can drive bounded reverse engineering without importing payload | Sprite frame sequences and playback timers for water splash, bunker sand, struck tree, ball lie, birdie/cup, arrows, golfer and bounced ball **not yet fully extracted**. These must be derived from actual resources and original renderer calls rather than guesswork. |

### Raw green-slope bit decoding (evidenced by `recovered_course_lookup.cpp`)

Original `MAPI` banks contain 640 tile records each, **8 bytes/record**. A selector mask chooses 1 of 4 descriptor words for each of the 8×4 fine subcells per tile. The binary words decode as:

```text
raw16 = big_endian_descriptor_word
descriptor_index = low_byte (with original out-of-range clamp)
direction = (upper_byte & 0x0F) << 8
magnitude = upper_byte >> 4
```

These are the **actual numerical slope data** driving physics; displaying green arrows is a separate rendering rule still requiring extraction. The new CI report computes the distribution from every `MAPS` game file while retaining the course raster itself privately.

## Files and repeatable extraction commands

- [tools/extract_pixtee_original_binary_hard_data.py](../../tools/extract_pixtee_original_binary_hard_data.py) reads the actual original PE and extracted EPF, fingerprints them, runs binary table extractors, checks equality to the committed original-source reference, then reports each of the 72 map/green/SPT sets, the raw original MAPI-based green slope statistics and source media hashes.
- [.github/workflows/pixtee-original-binary-hard-data.yml](../../.github/workflows/pixtee-original-binary-hard-data.yml) reruns the exact ingestion/extraction automatically and uploads the factual data as a research artifact. It never uploads original binary payloads.
- [tools/validate_pixtee_physics_reference.py](../../tools/validate_pixtee_physics_reference.py) verifies committed reference consistency with the recovery source and fails on accidental changes.
- Original mechanics: `engine/src/recovered_flight.cpp`, `recovered_ground.cpp`, `recovered_course_lookup.cpp`, `classic_course_resources.cpp`, `classic_hole_session.cpp`, `recovered_interactions.cpp`, `recovered_prng.cpp`.
- Parity vectors: [PHYSICS_BEHAVIOUR_BASELINES.md](PHYSICS_BEHAVIOUR_BASELINES.md), `engine/tests/recovered_ground_tests.cpp` etc., [Gate 1 signoff](../../analysis/evidence/gate1_final_signoff.json) and [original 18-hole continuous checkpoint](../../analysis/evidence/gate2_full_original_eighteen_hole_checkpoint.json).
- [CANONICAL_GAMEPLAY_MECHANICS_LOCK_2026-10-10.md](CANONICAL_GAMEPLAY_MECHANICS_LOCK_2026-10-10.md) captures owner design and gameplay requirements; **this page is the source-of-truth index for extracted hard gameplay numbers**.
- **No manual or guesswork is an admissible source for numerical fidelity.** For an unresolved animation/collision branch, extract game files + disassemble/harness original engine, record machine observations/inputs/outputs, then promote it when verified.

## Important status, not a blanket parity claim

**Passed now:** authentic source EXE+EPF hash checks; direct extraction & exact equality for 13 club rows, 13x10 lie selectors, 11 accuracy bounds/profiles, 77 descriptors; enumeration of 72 MAPM+MAPS+SPT resource sets; MAPI-based full original-green slope category distributions; 56 asset inventory entries. Source branch verification run #38089050718.

**Original restoration already demonstrated:** known straight/draw/fade, 3-to-rest, six surface, hazards, putter, slope, cup, PRNG-interaction, 81,920 collision comparisons and one deterministic full 18-hole round.

**NOT done:** all possible interaction branches, exact meter movement/colours, tree impact response, water/sand/tree **visual frame** and sound animation timing, all original club displayed yardages, player-controlled 18-hole Android parity. Don't call these extracted or solved; these are next reverse-engineering deliverables.

**IP gate:** Original data is research evidence, not authorization to distribute original game files, graphics, music, or course geometry inside an independently branded commercial Pixtee. If literal reuse is essential, obtain appropriate game rights and treat the project as a licensed remaster.
