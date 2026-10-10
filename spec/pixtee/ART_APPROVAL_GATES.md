# REJECTED ART — enforce no placeholder sprites (10 October 2026)

The owner rejected the full ten-sprite glyph batch as unacceptable, and has
explicitly required **NO PLACEHOLDER SPRITES, FULL STOP**.

Enforced in active Pixtee development:
- The glyph atlas and its SVG contact sheet were deleted from the branch.
- Only genuinely authored, owner-approved production PNGs may be drawn as
  golfer, tree, spectator or flower sprites.
- If an image is absent or not approved, the renderer draws no sprite and
  never creates a dummy box/silhouette/pixel glyph in its place.
- Debug builds are ENGINE TESTS ONLY until the art has been approved.
  They are not visually acceptable or distributable preview APKs.
- Release Gradle builds are blocked if a required sprite or explicit owner
  approval marker is missing.
- No approved production sprite assets currently exist in the project.
  Approval must be obtained before any production visual claim.

This restriction does not prevent work on ball physics, controls, gameplay,
course geometry, scoring, saved games, progression or automated testing.
It does prevent labelling a grey-box or assetless build as a finished game.

---

# Pixtee Golf — visual asset approval gates

**Status: ALL VISUAL ART UNAPPROVED.** The project owner signs off each batch before it is locked as the production canonical asset. Unapproved colours, Canvas primitives, deterministic geometry and any preview images are temporary scaffolding, not final art.

## Approval batch 1: gameplay pixel sprites (priority now)
- Golfer front/back/left/right tiny in-world poses, standing/aiming/winding up/contact/follow through, female/male/customisation styling, clothing and club accessories.
- Ball shadow, small lift, roll, cup, flag, tee markers, golf bag.
- Tiny spectators with optional two-frame idle/wave motions. Trees, grass, flowers and optional scenery motion.
- Deliver a labelled sprite sheet **AND** a natural-size composite view over a real course and phone portrait. Enlarged previews alone are insufficient.
- Check visual legibility, per-frame silhouette, real-world ball size, consistent pixel dimensions, terrain occlusion, and Fold closed/unfolded.
- Preserve mobile-friendly look close to observable 1990s top-down golf, but every shipped sprite must be original/licensed and legally distinct.

## Approval batch 2: terrain and course treatments
- Present one same-layout comparison per palette/biome, with fairway stripes, fringe, putting green contours, bunker edges, water, tree families, scenery shadows, collision-safe tee/green boards.
- Show 3 visually different playable holes side by side for each archetype (parkland, woodland, heath, links, marsh, desert, highland, coastal, autumn, frost).
- Approval covers **visual style**, not course gameplay balance. Course geometry needs separate par, hazard, tee/green and playtesting gates.
- The 35-course roster currently uses deterministic prototype layouts. That is NOT 630 final, hand-polished designs; unique authored landmarks and difficulty balance come after art direction approval.

## Approval batch 3: HUD, menus and effects
- Original-design semicircle Welly-o-meter at actual screen scale. Submit timing mockups: backswing, power peak, accuracy/late/hook/slice.
- Scorecards, menus, career, season calendar, trophies, sponsor board styles, sound identity.
- Animation sheets + a short in-game comparison at the approved camera scale; transitions must not obscure ball or ads.

## Asset status and version control
- Proposed -> revision requested -> approved canonical -> implemented -> on-device verified.
- Record each reviewed item with asset ID, preview, exact palette/animation frames, reviewer decision and canonical version tag.
- **No silent substitution** of approved assets; later changes return to review.
- Approved assets go into the game separately from collision data. Motion can be disabled for accessibility; all artwork must look correct without animation.
- No final asset may be described as approved without an explicit approval message from the owner.
- Current temporary Canvas sprites and palette data are **unapproved prototypes**.
