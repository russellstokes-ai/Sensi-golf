# Pixtee Godot migration: saved progress checkpoint (2026-10-11)

## Source control protection
- Working branch: `feat/pixtee-godot-recovered-core`.
- Draft review: PR #12, **not merged**.
- Sensible Golf original restoration `main`: untouched.
- Existing Pixtee native-portrait framework: untouched.

## Completed in this milestone
1. A new Godot 4.5 project exists at `pixtee-godot/`.
2. The native bridge links directly to all existing recovered C++ `engine/src/*.cpp` and exposes the real `ClassicHoleSession` + `ClassicMobileControls` through a `PixteeGameplay` GDExtension class.
3. Exposed 4096-step raw aiming, 13 club indices, 3-click sample capture and C++ tick/snapshot methods.
4. Original fixed timer is retained: 936/65536 seconds per simulation tick.
5. Independently authored Lakewood Hole 1 course terrain is converted to the existing classic MAPM/MAPI lookup contract. Not ready for art-scale final collision, green MAPS, slopes or full game completion.
6. A C++ adapter smoke test and a full native core test workflow exist.
7. Two stale inherited test expectations were corrected **only on this isolated branch**, without any change to recovered engine algorithms: descriptor index 52 maps to V ROUGH D2 (not semi rough), and code-8 CupTerminal with a nonzero cup distance is not automatically HoleScored.
8. Latest native smoke CI at commit da38d53: **SUCCESS**, all 18 CTests pass plus adapter smoke. Previous passes include original golden-master flight/putter/hazard/collision workflows.
9. Godot C++ build workflow is running as an independent gate. A successful native CTest does NOT prove GDExtension binary compatibility or rendering.
10. Master reuse/gap document: `GODOT_REUSE_AND_PARITY_MASTER_2026-10-11.md`.

## Remaining release blockers (no invented substitutes)
- Actual Godot GDExtension compiler and Godot editor headless startup validation.
- Android arm64 / Android emulator export and run.
- Original raw WHACK-O-METER scheduler, exact colour/window mapping and horizontal visual sampling.
- Fine spatial collision tile banks, dedicated greens, verified slope and cup interactions.
- Unsupported 9/10 specials, extreme accuracy, exact tree-strike activation/animation, original rules where not proven.
- Integrating the user's approved Pixtee art archives unchanged as canonical sources, exporting correct pivots/frames and event-driven animation.
- Original-scale ball-following camera, HUD, world-space ads, minimap and portrait/Fold layout.
- End-to-end human playable Hole 1 with no fake scoring, no placeholder art and full parity comparisons.
- Review of distribution rights for any recovered reference code and original extracted numerical tables before commercial publishing.

**No APK or customer-facing playable build has been produced by this milestone.**
