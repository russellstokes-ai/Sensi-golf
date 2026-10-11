# Pixtee — original Sensible Golf recovered data bank (ENGINEERING ONLY)

This directory holds exact byte-for-byte copies of **previously authenticated**
source-derived research data, not artist-generated estimates.

1. `ORIGINAL_PHYSICS_RESEARCH.json` is copied exactly from
   `spec/pixtee/ORIGINAL_PHYSICS_RESEARCH.json`: 13 raw club entries,
   13 × 10 lie selectors, 11 accuracy bounds, 11 curvature profiles,
   77 terrain descriptors, 70.017 Hz timer constants, distance/putt,
   hazard, cup and collision-lookup constants.
2. `ORIGINAL_ROUND_REFERENCE.txt` is copied exactly from
   `engine/tests/fixtures/original_round_all_eighteen.txt`: 18 recorded
   reference hole traces and original course resource identifiers. It is
   **evidence**, never a Pixtee commercial course design.
3. `python3 pixtee-godot/tools/verify_reference_data.py` checks both
   against their original repository sources AND verifies the C++ source
   tables agree with the original binary extraction research.
4. The `sensigolf_core` **C++ engine remains the sole runtime physics**.
   The Godot application can query the research manifest for developer
   verification, but **must not** implement its own formula from these
   records or silently normalize/replace raw samples.

**Legal gate:** These copies are research/engineering fixtures only.
The original EXE/EPF, proprietary artwork/music, MAPI/MAPM/MAPS course
payloads and raw sound data are **not** bundled. Commercial rights to
ship source-derived numerical tables and restored engine code remain
unreviewed; before an actual product release either obtain rights or
replace them with independently validated lawful implementations.

The recovered meter's full tick scheduler, visual colour windows,
extreme-accuracy handling, specials 9/10, full tree-collision activation
and some graphics/sound timing are **not** recovered. They must not be
fabricated or claimed complete.
