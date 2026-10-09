# Pixtee Golf — independent Android experiment

**Working name:** Pixtee Golf (not a trademark clearance)
**Product goal:** feel as close to classic 1990s overhead arcade golf as legally achievable, with cleaner pixel-art assets, smoother rendering, original courses, optional enhancements, offline career, stats, and rewards.
**Source project:** intended to move into its own clean repository. These are planning/data-contract files only, stored here because the GitHub connection does not expose new-repository creation. Do not link or import the Sensi-golf source tree into Pixtee builds.

## Documents
- [FIDELITY_AND_PROVENANCE.md](FIDELITY_AND_PROVENANCE.md) — behavioural targets, what can be independently reproduced, legal boundaries and unknown exact display measurements
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
