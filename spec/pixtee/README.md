# Pixtee Golf — independent Android experiment

**Working name:** Pixtee Golf (not a trademark clearance)
**Product goal:** feel as close to classic 1990s overhead arcade golf as legally achievable, with cleaner pixel-art assets, smoother rendering, original courses, optional enhancements, offline career, stats, and rewards.
**Source project:** intended to move into its own clean repository. These are planning/data-contract files only, stored here because the GitHub connection does not expose new-repository creation. Do not link or import the Sensi-golf source tree into Pixtee builds.

## Documents
- **[ORIGINAL_BINARY_HARD_DATA_MASTER_2026-10-10.md](ORIGINAL_BINARY_HARD_DATA_MASTER_2026-10-10.md) — START HERE for real Windows v1.014 executable/EPF-extracted numerical gameplay data; audited club power, all slopes/green resources, exact mechanics, file hashes and unresolved game-file/animation research. Source extraction PASSED on GitHub Actions run 38089050718.**
- [tools/extract_pixtee_original_binary_hard_data.py](../../tools/extract_pixtee_original_binary_hard_data.py) — reproducibly extract hard source data from the actual game ZIP; includes 72 MAPM/MAPS/SPT resource sets and the MAPI slope histogram. [Source-verification workflow](../../.github/workflows/pixtee-original-binary-hard-data.yml).
- [FIDELITY_AND_PROVENANCE.md](FIDELITY_AND_PROVENANCE.md) — behavioural targets, what can be independently reproduced, legal boundaries and unknown exact display measurements
- [ORIGINAL_PHYSICS_REFERENCE.md](ORIGINAL_PHYSICS_REFERENCE.md) — detailed numerical physics, ball states, launch, swing, flight, bounce, putting, slope, hazards, cup, scoring and verified unknowns
- [ORIGINAL_PHYSICS_RESEARCH.json](ORIGINAL_PHYSICS_RESEARCH.json) — research-only machine-readable 13 clubs, 11 swing profiles, 13×10 lie selector and 77 terrain descriptors
- [PHYSICS_BEHAVIOUR_BASELINES.md](PHYSICS_BEHAVIOUR_BASELINES.md) — exact original-restoration launch/flight/putting/terminal test vectors and independent fidelity acceptance gates
- [TECHNICAL_DESIGN.md](TECHNICAL_DESIGN.md) — mobile runtime, camera coordinates, simulation, meter and input contracts, save architecture
- [CONTENT_AND_PROGRESS.md](CONTENT_AND_PROGRESS.md) — original courses, artwork, career, stats, rewards, anti-exploit requirements
- [ACCEPTANCE_AND_PHASES.md](ACCEPTANCE_AND_PHASES.md) — independent proof gates, gameplay tests, art controls and release preparation
- [game_data.schema.json](game_data.schema.json) — proposed original-course and career content contract

## Fundamental constraint

A fresh application written to match **observable gameplay rules and functional behaviour** can be independently developed, but copying recovered source, tables, binary content, extracted sprites, original course geometry and soundtrack is **not** clean-room work. Renaming the game or moving some bunkers does not cure copying. For exact reuse of original intellectual property pursue written commercial permission. The original Sensi-golf restoration and Pixtee Golf are separate commercial/IP routes.

## Decisions fixed
- Target platform Android; phone landscape first, foldable open/closed support.
- Preserve original-style shot feel, 13 club slots, overhead framing, three-click swing, no-wind classic default, fast loop, short load times.
- Classic style **not** replaced by modern 3D; integer/pixel-aligned terrain presentation, optional smooth 60/90/120Hz animations.
- Physically independent courses and artwork, yet familiar strategic hole archetypes (par/length/dogleg/hazard types, avoiding substantially matching original topology).
- Optional wind must be explicitly OFF in the fidelity mode; no forced stat bonuses to physics.
- Offline-first career, tour tiers, round and per-shot stats, achievements, cosmetic rewards; local saves, export/backup and optional later cloud.
- Name provisional pending app store and UK/international trademark clearance.

No Pixtee implementation or compiled Android APK is claimed by this specification.

## Research reference reproducibility

The restricted numerical reference has an automated correspondence audit:
`python tools/validate_pixtee_physics_reference.py` (standard-library only).
GitHub Actions: `.github/workflows/pixtee-physics-reference-audit.yml`.
Neither the audit nor the documented facts grants redistribution or source-code reuse rights.
