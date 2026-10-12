# Pixtee — Frozen Work Hole 1, Menu and Approved Art Intake Contract
**12 October 2026 · Safe pre-integration handoff · New working branch: `feat/pixtee-m1-course-intake-safe`**

## What we can finish before the actual Work course and menus arrive

The recovered native engine and owner-approved visual files must remain separate until the **exact** Work-version `index.html`, `COURSE_PRODUCTION_STANDARD.md`, `COURSE_CATALOGUE_50.csv`, checksum list and menu source are accessible. No engineer should reconstruct the course or menus from screenshots and call them canonical.

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
