# PIXTEE GOLF — CANONICAL SOURCE LOCK & FALLBACK / RECOVERY BASELINE
**12 October 2026 · VERSION: BASELINE-01 · NEVER EDIT FROZEN ORIGINALS**

## Critical source-of-truth decision

**All completed or approved work is to remain immutable as its original baseline.** Further game engineering, shaders, Godot course conversions, asset import, display refinements and new courses are implemented as **separate derivative working copies** with clear provenance. No successful build or newly generated visual is permission to modify the original approved package or the frozen reference course.

**Canonical archive inventory (machine-readable and audit-friendly):**
[PIXTEE_CANONICAL_CHECKPOINT_2026-10-12.json](PIXTEE_CANONICAL_CHECKPOINT_2026-10-12.json). In a conflict between older art research manifests (which may say assets were not approved) and the user's newer explicitly approved asset ZIP, **the owner-approved ZIP controls**. Historical rejected images remain historical only.

## 1. Two GitHub FALLBACKS CREATED and confirmed

| Baseline | Immutable commit content | Archived GitHub branch / access | Do not |
|---|---|---|---|
| **Recovered Sensible Golf physics and Godot 4.5 bridge** | `1203f8c440cc96f56e18c3a50336a632ab10efaf` | [archive/pixtee-engine-pre-integration-2026-10-12](https://github.com/russellstokes-ai/Sensi-golf/tree/archive/pixtee-engine-pre-integration-2026-10-12) | No new commits, rewrites, merges, force-push or substitutions; base future engine work on a distinct branch |
| **Earlier Pixtee portrait game/framework and Git-committed art research** | `7b849d29db2a7e37ac039e51c39a80dce0106e0c` | [archive/pixtee-portrait-framework-2026-10-12](https://github.com/russellstokes-ai/Sensi-golf/tree/archive/pixtee-portrait-framework-2026-10-12) | Do not mistake old previews, unapproved art or rejected test assets for the newer final approved package |

- The **unmodified original Sensible Golf restoration** remains on `main`; never merge research-derived Pixtee changes into it as part of this freeze.
- Existing working branches `feat/pixtee-godot-recovered-core` ([PR #12](https://github.com/russellstokes-ai/Sensi-golf/pull/12)) and `feat/pixtee-native-portrait-framework` ([PR #11](https://github.com/russellstokes-ai/Sensi-golf/pull/11)) remain open draft development branches.
- **What GitHub saves:** all Git-tracked code/documents/test fixtures/preproduction asset source in the exact archived commits. It does **not** save excluded, ignored or uncommitted local user files or another Work session's `C:\` files.
- **Protection limitation:** archived commits are pinned by full SHA, and those archive branches should be treated as read-only by policy; **GitHub branch protection was not configured**. Do not assert repository access controls have been applied.

### Pinpointed engine evidence (saved in GitHub engine archival commit)

| Area | Canonical files |
|---|---|
| Original physics/table hard data | [ORIGINAL_PHYSICS_RESEARCH.json](ORIGINAL_PHYSICS_RESEARCH.json), [Godot research copy](../../pixtee-godot/reference/ORIGINAL_PHYSICS_RESEARCH.json), 13 clubs, 13×10 lies, 11 accuracy/curvature records, 77 surfaces |
| Original numeric 18-hole research trace | [ORIGINAL_ROUND_REFERENCE.txt](../../pixtee-godot/reference/ORIGINAL_ROUND_REFERENCE.txt), not Pixtee commercial course geometry |
| Recovered native C++ engine | [engine/src](../../engine/src), [engine/include/sensigolf](../../engine/include/sensigolf), [engine/tests](../../engine/tests) |
| Godot 4.5 integration / headless tests | [pixtee-godot](../../pixtee-godot), [native](../../pixtee-godot/native), [EngineBindingCI.gd](../../pixtee-godot/scripts/EngineBindingCI.gd) |
| Original 73-mechanic completeness / 15 original gameplay signoffs | [Engine completeness register](ENGINE_COMPLETENESS_GATE_2026-10-11.json), [15-gameplay recovery evidence](RECOVERY_15_GAMEPLAY_SCOPE_AND_EVIDENCE_2026-10-11.md) |
| Milestone One 27-area inventory and 14 commercial delivery gates | [M1 data inventory](MILESTONE_ONE_ENGINE_DATA_INVENTORY_2026-10-12.json), [Commercial engine handover](MILESTONE_ONE_COMMERCIAL_ENGINE_HANDOVER_2026-10-12.md) |
| Original behaviour remaining (engine parity research) | [Nine-area recovery checkpoint](NINE_AREA_EXECUTION_CHECKPOINT_2026-10-12.md) |
| Visual and game requirements | [Canonical gameplay lock](CANONICAL_GAMEPLAY_MECHANICS_LOCK_2026-10-10.md), [camera/world scale](SCREEN_RESOLUTION_AND_WORLD_SCALE_2026-10-10.md), [World FX and Game Feel](PIXTEE_MILESTONE_ONE_WORLD_FX_AND_FEEL_STANDARD_2026-10-12.md) |
| Initial independently authored engine test course | [pixtee_course_adapter.cpp](../../pixtee-godot/native/src/pixtee_course_adapter.cpp); **NOT THE FROZEN WORK-SESSION HTML HOLE 1** |
| Art provenance/history | [art source, previews and manifests on archived portrait branch](https://github.com/russellstokes-ai/Sensi-golf/tree/archive/pixtee-portrait-framework-2026-10-12/art), [approved visual direction](PIXTEE_CANONICAL_VISUAL_2026-10-10.md) |

**Reference executable SHA-256** `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8` (Sensible Golf v1.014) records research provenance only. Recovered numeric/code data still require legal review for commercial distribution. Do **not** commit the original protected executable, original course data or game media.

### Canonical documentation snapshot (third archive)

The most complete source-lock rules, exact fallback commit IDs, 27-area recovered-engine inventory, 14-point commercial checklist, **verified approved asset ZIP hash/Library backup**, course-effects standard, and Windows copy-only SHA-256 helper are preserved together in the **[Pixtee canonical verified-assets archive](https://github.com/russellstokes-ai/Sensi-golf/tree/archive/pixtee-canonical-verified-assets-2026-10-12)**. The [earlier first documentation snapshot](https://github.com/russellstokes-ai/Sensi-golf/tree/archive/pixtee-canonical-source-lock-2026-10-12) remains preserved, unchanged, as an additional fallback. Use this archival copy as the *read-only source of record* when development documentation subsequently changes. It does **not** include any uncommitted Windows Work files or the final approved ZIP unless those files were separately added and checked — they were not verified in this checkpoint.

## 2. Frozen Work course — LOCAL ORIGINAL, do not touch

Reported frozen visual/course reference (from ongoing Work session):

```text
C:\Users\86185\Documents\Codex\2026-10-11\can\outputs\pixtee-hole-01-reference-2026-10-12\
  index.html                     <- OWNER-APPROVED frozen Hole 1 visual source
  COURSE_PRODUCTION_STANDARD.md  <- Godot build standard, particles, shaders,
                                    lighting, collision and acceptance
  COURSE_CATALOGUE_50.csv        <- 50 future concept designs
  [existing checksum and dependency files, including any nested images]
```

**What is verified:** the user identified these frozen local files and says Work has file integrity hashes. **What is not verified here:** exact file contents/hashes, that they were uploaded to GitHub, or that a second copy was made elsewhere. The local Windows filesystem is not mounted through the GitHub tool. An older `Pixtee-Golf-Lakewood-Hole-1.html` exists in ChatGPT Library but **must not** be mistaken for the newer 12 October frozen Work deliverable.

**Freeze rule:** future Godot implementation must *reference or duplicate* this exact Hole 1 design without editing the original files. Maintain the same apparent sizes, view, assets, tee/green framing, sponsor placement, course composition and colour treatments. A new shader/particle effect is *additive* and requires review against the frozen screenshots and Work `COURSE_PRODUCTION_STANDARD.md`. New course generation from the 50-course catalogue is a separate later task; no rewriting of the catalogue or existing prototype.

## 3. Approved artwork package — LOCAL ORIGINAL, do not touch

The user approved the ZIP **`Pixtee golf assets(1).zip`** as the canonical Pixtee art pack (a prior upload also used `Pixtee golf assets.zip`). Approved sprite sheets, golfer directions, side-on bird frames, terrain, flowers, water, crowds, sponsorship boards, course assets and FX must be sourced from the actual approved ZIP, **not** regenerated from preview contact sheets. GitHub `art/previews/` and `art/source/` contain valuable older material preserved in the portrait archive but do not by themselves establish the correct final approved binary revision.

**Verified approved art backup (12 October 2026):** the exact `Pixtee golf assets(1).zip` was located in the ChatGPT Library, materialized, ZIP-tested and independently copied into **`/Pixtee Canonical Backups/2026-10-12/Pixtee golf assets(1) - CANONICAL FROZEN.zip`**. Both files have **20,820,759 bytes**, identical SHA-256 **`2043e9c678b9904311778d17a1f341d577344e6d49a3a9d48ce6c3aef3b5239e`**, matched byte-for-byte, and all **16 ZIP entries passed CRC verification**. Full [canonical SHA-256 and asset-content record](PIXTEE_APPROVED_ASSETS_CANONICAL_SHA256_2026-10-12.json). **This is a genuine second retained Library copy; no original ZIP bytes were changed.** Store the original untouched; local offline second-media backup remains advisable. If supplementary animation frames are needed, create separate derivative assets, never silently replace approved originals.

## 4. Safely copy local Work references (tool added; needs execution on Windows)

[Copy-only local canonical backup script](../../tools/backup_pixtee_frozen_work_assets.ps1).

**Run in the ongoing Windows Work session** with the actual original approved-asset ZIP location:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\backup_pixtee_frozen_work_assets.ps1 `
  -CourseFolder "C:\Users\86185\Documents\Codex\2026-10-11\can\outputs\pixtee-hole-01-reference-2026-10-12" `
  -AssetPaths "C:\ACTUAL_LOCATION\Pixtee golf assets(1).zip"
```

The approved ZIP already has a separate SHA-256-verified Library backup; running the Work-side script is still necessary to capture **the exact newer frozen Hole 1 folder**. The script **never moves, renames, edits or overwrites** the originals. It makes a new independent archive under `C:\Users\86185\Documents\Codex\pixtee-canonical-backups\PIXTEE_CANONICAL_FROZEN_YYYYMMDDTHHMMSSZ\` and creates a per-file source-to-destination SHA-256 verification log `PIXTEE_CANONICAL_LOCAL_SHA256_MANIFEST.json` covering *all* files inside the frozen Work folder and any supplied asset ZIP/directory. The existing source hash records in the original Work folder remain untouched. The tool checks for concurrent source changes while copying and aborts on checksum discrepancy.

**Important:** without `-AssetPaths`, it can make a **course-only** snapshot but exits with a warning/nonzero status; **that is NOT an approved full-asset backup**. No local execution or resulting SHA-256 checksum has occurred in this ChatGPT/GitHub conversation. A second external/cloud copy of the resulting archive is recommended, because a backup on the same PC is not enough for disaster recovery. If Work updates the local course, create a *new* reference revision/different path and have the owner approve it; do not revise this frozen folder.

## 5. Canonical ownership hierarchy / no-touch contract

1. **Frozen Work Hole 1 `index.html` + Work `COURSE_PRODUCTION_STANDARD.md`:** approved Hole 1 visual and production specification. Most recent owner-frozen version prevails over old prototype; its hash must be recorded before intake.
2. **Owner-approved `Pixtee golf assets(1).zip`:** master artistic source. Approvals refer to exact files and bytes/hashes, not generic filenames or similarly named revisions.
3. **GitHub archived recovered core commit `1203f8c`:** physics, input/sample, collision, scoring, replay, research constants, data tables and recovered engine contract. No alternate Godot/GDScript/Kotlin physics.
4. **GitHub archived Pixtee portrait commit `7b849d2`:** useful prior source/history and art previews, subordinate to the owner-approved ZIP on art.
5. **Commercial engine handover, course FX standard and M1 checklists:** acceptance/integration criteria. These instruct work, but do not supersede owner-approved frozen visuals or fundamental native physics source evidence.

Never let a new player FX shader change world simulation, ball terrain lookup, PRNG or replay. Sponsor boards remain on-course, not on footpaths or in the HUD, and noncolliding. Approved top-down art, ball-to-camera world scale, wind direction and Fold adaptation remain stable.

For any correction:
- Duplicate source to `work/<description>`, `feature/<...>` or new dated version; do not edit archive/approved original.
- State original file SHA, proposed file SHA, visual before/after, reasons, gameplay effect and test result.
- Obtain explicit owner approval for art/course visual replacement; no automatic acceptance from passing CI.
- Record the approved new revision and **retain old hashes and all original bytes** so one command can restore.

## 6. Verified recovery procedure / fallback

For engine: restore from GitHub **full immutable commit** rather than whatever the current working branch happens to contain:

```sh
git fetch origin
git switch -c restore/pixtee-engine-2026-10-12 1203f8c440cc96f56e18c3a50336a632ab10efaf
# Code and known data are restored here; verify:
python3 pixtee-godot/tools/verify_reference_data.py
python3 tools/verify_pixtee_milestone_one_inventory.py
```

For old Pixtee portrait art/source history:
```sh
git switch -c restore/pixtee-portrait-2026-10-12 7b849d29db2a7e37ac039e51c39a80dce0106e0c
```

**Do not restore over a dirty working copy.** Stash or commit current work first, or use a fresh Git clone before checking out a baseline. For frozen Work Hole 1 and ZIP, restore only from the separately SHA-verified local archive (after Work generates it). No current GitHub branch is a proved byte-for-byte substitute for those locally frozen originals.

**Commercial-quality status remains OPEN:** saving these baselines protects progress but does not certify a completed native Android APK, three playable holes or rights clearance. Continue build/testing on **new feature branches** and keep both archival branches untouched.

---

**Checkpoint summary:** GitHub engine, portrait and canonical documentation snapshots ✅. Approved final 16-file asset ZIP and a second persistent Library copy verified byte-for-byte with SHA-256 ✅. Canonical source lock and restore instructions ✅. **The frozen 12 October Work HTML Hole 1 folder remains LOCAL-ONLY / NOT INDEPENDENTLY VERIFIED HERE**; the copy-only checksum backup tool is ready for Work execution. Do not claim the entire project is independently backed up until that exact folder and its hashes have been captured. Never infer `all files safe` until its `PIXTEE_CANONICAL_LOCAL_SHA256_MANIFEST.json` has been produced and checked.
