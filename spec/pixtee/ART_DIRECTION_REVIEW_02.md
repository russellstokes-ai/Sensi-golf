# Pixtee artwork / scale — review round 02 (OWNER APPROVAL PENDING)

Date: 2026-10-10
Status: **UNAPPROVED PROPOSAL — NOT IN GAME ASSETS**

## Owner feedback captured
- Previous crude ten-sprite batch REJECTED and deleted; NO PLACEHOLDER SPRITES.
- Later visual direction positively received: lush, clear, slightly enhanced classic
  2D top-down golf pixel art.
- Original course and player proportions were too small in the full-hole
  phone view. The most recent tighter on-course crop was positively received.
- Requested **as close as possible to Sensible Golf camera, player size,
  course proportions, physics, and gameplay**. Independently author artwork
  and original course layouts unless copyrighted assets are licensed.
- Golfer and people should be a LITTLE **cuter**: more friendly character
  and spectator appeal, less stiff or blocky, while tiny/legible at true scale.
- All final art must be explicitly owner-approved, every revision separately.

## Current implementation (camera/projection only)
- Candidate zoom = 1.9x versus former full-hole-fit viewport.
- Follows actual ball in X/Y; look-ahead 52 world units toward the green.
- World model/hazard physics untouched.
- Production sprite source resolution is decoupled from drawn world size:
  prototype numerical sizes are golfer 13.5, spectator 12, round tree 37,
  pine tree 42, flowers 4 world units high; **NONE of these art dimensions
  has yet received an explicit approval**.
- Independent reference comparisons and behavior tests in
  CAMERA_SCALE_REFERENCE.md and CourseViewportTest.kt.
- Debug builds with missing production sprite assets remain visual-incomplete
  ENGINE tests only; no substitute sprite is drawn.

## Approval workflow (strict)
1. Show 1 real-phone mockup at the tighter camera, 1 natural-size golfer/ball/
   tree comparison, golfer animation breakdown, spectators and terrain art.
2. Ask for separately explicit sign-off for camera ratio, model ratio and
   character style. No implication that approving a concept covers its
   unreviewed PNG/atlas exports.
3. Prepare a genuine exact-source production asset candidate, display at
   in-game size as well as enlarged crop, and seek approval for **that exact
   file revision**.
4. Only AFTER approval, add its SHA-256 and owner reference to
   ART_REVIEW_REGISTER.json and transfer to production.
5. Run Android render tests on closed/tall/Fold-like portrait views; visually
   inspect shot/putt scale and sprite registration.
6. No silent substitution or implicit approval by a successful compile.

## Final acceptance targets
- Retain straight-down 2D classic gameplay.
- Golfer not dominating the fairway; ball and golfer visually proportional,
  anchored consistently on every animation frame.
- A screenshot of a long hole should show only a cropped region and retain
  usable camera context, not all 18 holes or the entire current hole.
- Objects occupy meaningful and appropriately comparable sizes relative
  to fairway, green, and hazards.
- Scenery/crowd remain decorative with no collision changes.
- Art approval is separate from legal review, gameplay test acceptance,
  and release eligibility.

No production PNG files were added by this art direction review.
