# PIXTEE GOLF — CANONICAL VISUAL BRIEF
## Approved creative direction; source production art still pending

**Owner decision 10 October 2026:** The pixel-art concept sheet showing
a richly detailed PIXTEE GOLF title menu over a golf course, game view with
advert boards, cute character/spectator studies and richly textured
environments was explicitly approved as the visual target:
"This is gold we want to make a game that is this"; "the art is great lock this in".

### What was approved
1. Art direction and quality: lush, readable, richly textured, premium 16-bit/
   polished pixel-art. Match the *look and presentation standard* of the
   approved concept; do not revert to programmer glyphs or placeholders.
2. Main menu: title and approved-style wooden menu buttons overlay a **living
   course**, with real simulated AI golfers actively playing in the background.
3. Gameplay view: same straight-down perspective as classic Sensible Golf,
   equivalent relative player/course/terrain sizes, and a close, ball-following
   crop. Avoid full-hole-scale zoom-out. Current numeric zoom 1.9 remains
   provisional pending exact side-by-side phone screenshot confirmation.
4. Characters: classic tiny golf-world proportions, a touch cuter, rounder,
   friendlier and more expressive without visual clutter.
5. Impact pose: clubface and ball must meet exactly at the launch frame; do
   not rely on static art mockups that leave the ball away from the club.
6. Sponsors: in-world, slightly larger than earlier board prototype, located
   around tee and green, integrated into the landscape and not HUD banners.
   Board geometry must not alter physics/collisions.
7. Surface/terrain: independent, original course designs with comparable
   classic balance; richly articulated bunkers, grass, water, trees, flowers,
   rock edging, spectators, bridges and small environmental details.
8. 35 original courses x 18 holes; gameplay scale/ball/club/collision
   behavior to be measured against original observable Sensible Golf.
   No claim of byte-exact parity without measurement.

### What was NOT approved
- No extracted individual production PNG, animation atlas, soundtrack,
  exact pixel contact point, course tile or final asset hash.
- The previously rejected ten-sprite glyph batch remains rejected.
- The original game's protected sprites/bitmap courses are not licensed by
  approving this concept, nor does approval permit copying their images.
- The broad visual direction is approved. **Every individual production art
  sheet and changed revision still requires explicit owner approval**.
- Image-generation poster/composite art is a *creative benchmark*, not a
  pixel-perfect importable atlas. Do not crop a poster into game sprites
  without making and reviewing actual production assets.

### Production art package — review and acceptance gates

**Gate A — playable golfer, impact and ball**
- All 4 facing directions; stance, address, backswing, contact, follow-through,
  putting and walking.
- Frame set registration: same feet pivot and head silhouette; the clubface tip
  at CONTACT must coincide exactly with the ball world coordinate.
- Original golf clothes with unlockable styles, readable at ~26 logical screen
  pixels tall on a 360-wide reference phone. Native PNG pixel dimensions may
  be larger; rendering is constrained in world units.
- Two renders required per candidate: real game scale alongside ball/green
  and enlarged individual frame sheet.

**Gate B — animated course population and scenery**
- Cute readable spectators: idle, turn, applaud, wave, consistent tiny scale.
- Trees (round/needle/flowering), bushes, grasses, flowers, flags, golf bags,
  tee boxes, wildlife only where thematically appropriate, water edge detail.
- Idle environmental animation must work at 30 FPS and respect reduced motion.

**Gate C — menus and gameplay interface**
- Live menu composition: PIXTEE GOLF logo, wooden buttons, backdrop course,
  animated golfers, world-space ads; title remains legible on compact phones.
- Welly-o-meter visually matches the approved classic semicircle feel. Exact
  timing and contact cues validated with real emulator interaction tests.
- Course selector, scorecards, career, achievement and statistics art should
  be a consistent family with the title/menu.

**Gate D — 35 courses**
- A separate visual theme plan for each course; 630 seeded hole prototypes
  are NOT equivalent to 630 hand-polished, balanced holes.
- Distinct course silhouettes, fairway/hazard choices, unique feature
  placement and valid tee-to-green playable routes.
- Approval by course/theme batch before applying tiles widely.

**Gate E — sponsor placement**
- Sizes, typography, paid disclosure, distance from tee/cup, visibility across
  ball-follow camera and compact Fold layouts.
- No collision, physics interference, full-screen video, tracking or
  unapproved external advertising creativity.

### Deliverables before production release
- Owner approval references per exported art file.
- SHA-256 validation of every approved production PNG/atlas in
  `spec/pixtee/ART_REVIEW_REGISTER.json`.
- Emulator-verified close game crop, real ball-to-club contact, 4 playable
  facing directions, animated menu, different course themes.
- Continuous 18-hole scoring tests and physics/putting/terrain comparison
  report with **measured deviations**, not unverified claims of exactness.

### Implementation note (10 October)
The engine and demo menu may be tested without these images, but debug builds
without approved art are **not** visual previews or a complete game.
Production art release gate must remain closed.
