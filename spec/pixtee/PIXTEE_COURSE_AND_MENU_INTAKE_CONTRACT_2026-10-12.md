# Pixtee — Frozen Work Hole 1, Menu and Approved Art Intake Contract

> **OWNER LOCK: DO NOT STRETCH ANY GRAPHICS.** Mandatory [zero-stretch course, sprites, camera, Fold and collision-alignment rules](PIXTEE_ZERO_STRETCH_ART_AND_CAMERA_LOCK_2026-10-12.md). The provisional 0.947 native geometry transform is **NOT approved**; never scale visual art, clip the green or introduce a hidden collision mismatch. Preserve the original 544×912 layout, crop/scroll the camera and resolve the MAPS integration difference explicitly.
**12 October 2026 · Safe pre-integration handoff · New working branch: `feat/pixtee-m1-course-intake-safe`**

## Work course and production standard RECEIVED — verified canonical intake, 12 October 2026

**The user supplied two real ZIPs in this conversation, and we examined their actual bytes.** Do not confuse these with earlier local `pixtee-hole-01-reference` or older Library `Pixtee-Golf-Lakewood-Hole-1.html`:

| Incoming frozen ZIP | SHA-256 (whole original uploaded ZIP) | Contents | Status |
|---|---|---|---|
| `pixtee-lakewood-open.zip` | `82c0df6831241e31ad0c83a4302df145349afb9b63503487a914b964d05c7ff3` | 43 regular files: self-contained HTML with embedded imagery, `course-layout.json`, `course.js`, `asset-manifest.json`, eight previews, 16 wind frames, foliage/cart/source image files, internal older standard and catalogue | **Exact second copy saved in Library and byte-identical; design only, not Godot gameplay** |
| `pixtee-production-standards-v1-1.zip` | `1544ac4782b64d2a2b968a581f31b9cf7b37b6e0cf4583dd665e21d1d81ffe17` | 3 files: **v1.1** `COURSE_PRODUCTION_STANDARD.md`, updated `COURSE_CATALOGUE_50.csv`, `MOUNTAINS_LAYOUT_AND_EFFECTS_ADDENDUM.md` | **Exact second copy saved in Library and byte-identical; current future course-production authority** |

Saved Library originals (do not overwrite):
- `/Pixtee Canonical Backups/2026-10-12/Work Course and Standards/pixtee-lakewood-open-ORIGINAL.zip`
- `/Pixtee Canonical Backups/2026-10-12/Work Course and Standards/pixtee-production-standards-v1-1-ORIGINAL.zip`
- `/Pixtee Canonical Backups/2026-10-12/Work Course and Standards/pixtee_canonical_course_standards_integrity_2026-10-12.json` — all **46** inner-file SHA-256 hashes, zip sizes and CRC proofs, with source/backed-up ZIP exact comparisons.

The [source intake JSON with full statuses](PIXTEE_LAKEWOOD_OPEN_AND_COURSE_STANDARD_V1_1_INTAKE_2026-10-12.json) is mandatory for all future engine integration. The actual HTML's `HOLE_01_REFERENCE_STATUS.json` says `VISUAL_REFERENCE_APPROVED_FOR_HANDOFF`, `GODOT_INTEGRATION_AND_ACCEPTANCE_REQUIRED`, `NOT_COMMERCIALLY_READY`. Its `course-layout.json` is a **544 × 912 visual-unit**, par-4 candidate named **Willow Creek**, with 445 cosmetic placements including 17 trees, four advertising boards and a shoreline-connected creek/pond. Their positions are *not* automatically native gameplay physical colliders.

**Conflict resolved without modifying originals:** the course ZIP contains an **older** 22,654-byte production standard and a 9,054-byte catalogue; the separate v1.1 ZIP provides the **newer** 35,168-byte standard, 10,014-byte 50-course catalogue and mountain effects addendum. Keep the old files byte-for-byte inside the HTML archive for freeze integrity, but use v1.1 as the **authoritative rules for all NEW courses**. The changed C31–C35 mountain family includes Cloudline Ridge, Alpine Loch and Frostgate Pass; no mountains, snow, cliff or fog effect may silently add original golf physics that the C++ core doesn't support.

**Freeze caveat:** `REFERENCE_README.md` points to a `REFERENCE_SHA256.csv` that is **not actually present** in this uploaded ZIP. Our separately generated intake SHA-256 manifest proves exactly what was uploaded and backed up, not a historical hash match to an unavailable earlier Work freeze. The HTML package's existing internal `HOLE_01_REFERENCE_STATUS.json` documents the actual handover status.

**Architecture decision:** The approved `index.html` is the visible/cosmetic design authority, but `course-layout.json` is **not a compiled MAPM/MAPI/MAPS resource**. The current native `make_lakewood_hole_1()` fixture is 19×64 original tiles (304 × 512 world units), and its geometry has not been verified equivalent to the **544 × 912** HTML. The Gold/Green native collision and trees/flag/cup must be separately authored; do not alias the two courses or change native physics to fit the HTML image. Build a reversible reference-world→native-world transform, rasterise the *approved original polygons* at native 2×2 MAPI collision precision, and validate 4 tee slots, source-compatible cup/green/slope and hazard topology before claiming playability. Sponsor boards, water motion, birds, flowers, grass, and idle actors are separate render-only placement records.

**Menus:** no final menu archive was contained in these two ZIPs; menus remain a later independent frozen intake.

---

## What was prepared before the real Work course arrived

**Update:** We now have the exact uploaded Work `pixtee-lakewood-open.zip` and independent v1.1 standards ZIP, separately verified and preserved in Library. The menus remain outstanding; earlier intake code is still a safe extraction-directory tool, not an automatic conversion or a substitute for the received ZIP manifest. No engineer should reconstruct the course or menus from screenshots and call them canonical.

This branch adds an independently testable **read-only intake audit**, not a new course, replacement artwork, alternate physics or fake menu. The validator intentionally marks `html_course_converted_to_godot=false` and `three_playable_holes_completed=false` after a successful pass.

### 1. Source precedence — FROZEN, never edited

1. [Immutable-by-policy canonical baseline](PIXTEE_CANONICAL_SOURCE_AND_RECOVERY_BASELINE_2026-10-12.md), [SHA-verified original approved 16-item asset ZIP manifest](PIXTEE_APPROVED_ASSETS_CANONICAL_SHA256_2026-10-12.json).
2. Exact original Work frozen reference folder `C:\Users\86185\Documents\Codex\2026-10-11\can\outputs\pixtee-hole-01-reference-2026-10-12\`; **the Work folder is not available from GitHub and is not currently copied by this audit**.
3. Work-produced menus, when provided, become separate immutable source inputs, with their own hash list and owner review. Treat unspecified menus as not provided.
4. Godot C++ reference engine [snapshot](https://github.com/russellstokes-ai/Sensi-golf/tree/archive/pixtee-engine-pre-integration-2026-10-12), [source-code/archive](https://github.com/russellstokes-ai/Sensi-golf/tree/archive/pixtee-canonical-verified-assets-2026-10-12): recovered numerical and gameplay authority.

**Never edit original snapshots or the approved visual reference to make data load.** Conversion should create wholly separate Godot/manifest output paths on an integration feature branch and retain original hashes for traceability.

### 2. Tool prepared for when source arrives

[Source and approved-art hash intake](../../tools/pixtee_frozen_course_intake.py) performs these real checks:
- All files in the frozen Work Hole 1 source tree are enumerated and SHA-256 hashed, including nested graphics, data and existing checksum files; no ignored dependencies or symlink substitution.
- The expected three Work files must exist; the 50-course CSV must be valid and nonempty; whether it actually has 50 data rows is reported, not asserted without inspection.
- Exact approved `Pixtee golf assets(1).zip` SHA-256 **`2043e9c678b9904311778d17a1f341d577344e6d49a3a9d48ce6c3aef3b5239e`**, ZIP byte size, 16 member names and each member CRC must match the independently saved canonical manifest.
- Optional Work freeze manifest `{"files":[{"path":"index.html","sha256":"..."}]}` must include **every** file (including internal dependencies); changed and missing entries fail closed. A Work-provided independent hash list is needed to verify the *previously frozen* bytes, not merely to hash a new copy after the fact.
- Optional menu file or directory is inventoried, but source visual approval and menu-to-game action wiring remain open.
- An output JSON record is written to a **new independent destination** outside the input trees, using exclusive file creation; existing reports are never overwritten.
- This tool does **not** convert HTML, invent hole geometry, parse/render the frozen course, create a Godot resource or claim commercial gameplay readiness.

### 3. Example read-only Windows handoff command

From a clone containing this new intake branch, replace `D:\SAFE\...` with the *actual* canonical ZIP/menu paths:

```powershell
python tools/pixtee_frozen_course_intake.py `
  --course-dir "C:\Users\86185\Documents\Codex\2026-10-11\can\outputs\pixtee-hole-01-reference-2026-10-12" `
  --approved-zip "D:\SAFE\Pixtee golf assets(1).zip" `
  --menu-path "D:\SAFE\PixteeFinalMenus" `
  --output "D:\pixtee-intake-audits\hole1-menus-intake-001.json"
```

`--menu-path` is optional until menus arrive. `--work-freeze-manifest "D:\SAFE\pre-existing-hashes.json"` is optional but **required for independent earlier-frozen hash verification**. Do not synthesize an old hash list from changed files or claim this is a true frozen-source comparison if the original checksum list isn't available.

### 4. Integration work approved after a passing intake

- Parse **actual** Work standard and source before choosing how its world scale, camera, textured terrains, swaying grass, soft water motion, flag, sponsor boards and 50-course catalogue map into Godot.
- Create a separately versioned course **visual layer** without re-rendering, recolouring or modifying the approved artwork ZIP. Preserve HTML layout/visual samples against a before/after screenshot check.
- Author independent, physically valid **MAPM/MAPI/MAPS** geometry, tee/cup/flag data and verified tree collision object manifest from the real course's intended design — **not guessed from a screenshot**, and not copied from the original game's copyrighted course maps.
- Connect native `PixteeRoundRuntime` to the actual playable course; ensure launch → bounce/land → sand/water/OOB → real tree/flag/cup → score → next hole behaves correctly.
- Reuse the one Godot effect system ([living course standard](PIXTEE_MILESTONE_ONE_WORLD_FX_AND_FEEL_STANDARD_2026-10-12.md)) without affecting original CPU physics/PRNG; production effects require owner review.
- Menus should dispatch to **real** game states: New Round, Continue, course selection, controls/settings and score/stats, with no fake menu actions; actual menu visuals and ownership remain pending source delivery.
- Perform desktop native + Godot ABI + Android arm64 + Fold portrait/unfolded tests; don't claim a release from this audit.

### 5. Tests added now

[8 isolated intake regression tests](../../tools/tests/test_pixtee_frozen_course_intake.py) for unchanged input files, complete independent hashes, missing course standard, changed ZIP, menu hashing, writing inside original, overwriting approved art and symlinks.

[GitHub workflow](../../.github/workflows/pixtee-frozen-course-intake.yml) runs these tests and existing reference-bank and engine-completeness audits. **CI with synthetic fixture bytes is not a test of the real Work course, the real menus, all 3 holes or the Android app**.

### 6. Native 3-hole round stop — NEW isolated product layer

**Confirmed architectural gap:** `ClassicHolePlan`, `ClassicRoundSession` and `ClassicGameSession` are source-accurate **18-hole** controllers. Do not change them simply to finish our first three-hole preview, and never pretend the original Windows game finished a round after three.

On this separate feature branch, [`PixteeThreeHoleRound`](../../pixtee-godot/native/src/pixtee_three_hole_round.hpp) wraps the original `PixteeRoundRuntime` and applies a strictly product-owned **three-hole milestone stop**:
- Accept **exactly three distinct course resource IDs** and real per-hole par metadata; reject duplicate or invalid input before starting.
- Register only the three approved authored native course resources. Course bytes cannot be replaced after a hole starts.
- Delegate real shots, swing and physical collisions, scored holes, strokes, par, hazard acknowledgements and fixed native ticks to the existing untouched recovered core.
- After exactly three separately scored holes, report **`preview.complete()==true`** and forbid playing a fourth. The underlying original native round correctly stays at `holes_completed=3`, `round_complete=false` because its source contract remains 18-hole.
- The internal 18-byte original plan is padded for constructor compatibility only and the additional resource requests are **never exposed as playable courses**. These are not 15 phantom products; the real three Work courses remain pending.
- Preserve original native deterministic stable shot replay serialization. **3-hole restore replay is not exposed yet**, since safe atomic verification of the restored three-hole product boundary is unfinished.

[Native three-hole regression](../../pixtee-godot/native/tests/three_hole_preview_gate_tests.cpp) exercises actual native cup scoring on three artificial CI-only small numeric fixtures, metadata checks, invalid/duplicate resource rejection, impossible early commits, and attempted fourth-hole access. **These fixtures are not production-design courses or a playable Android/Godot scene.** Live three-hole assets, Work HTML conversion, native Godot binding and save/restart remain separate acceptance gates.

### 7. Handoff boundary

**At this moment:** the canonical art ZIP is separately backed up with matching hashes; the recovered engine is archived; the Work Hole 1 and newer menus are not supplied to this GitHub branch. Intake tooling is designed not to interfere with parallel Work production.

**Next step when files arrive:** verify exact frozen checksums, inspect Work source and standard, reconcile collision/world/camera/FX metadata, then create the *first playable Godot Hole 1 as a derivative* of the frozen design. No further Sensible Golf numeric discovery is needed to prepare the importer.
