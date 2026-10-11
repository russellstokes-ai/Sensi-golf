# Pixtee Golf — executed sprints, 10 October 2026
Branch: feat/pixtee-native-portrait-framework, draft PR #11.
Never edit the original Sensible Golf Android port in the root project.

## Source facts / scope
The owner approved a polished pixel-art composite as the canonical visual
**direction**, demanded approval of EACH actual art file, and prohibited
placeholder sprites. They also approved closer traditional camera framing,
friendlier "cuter" characters, a live course-backed title screen, slightly
larger in-world advertiser boards and true original-like golf play scale.

**Do not misrepresent the original game's observed physics as proven identical
in Pixtee.** The independent Kotlin engine is still being measured against
known shot/lie/cup/roll/collision behavior. Original art and courses are not
licensed for use in the Pixtee production assets.

## Sprint 1 — gameplay / impact coordinate integrity
- `PixteeCore.golferWorldX/Y` holds the golfer at the actual shot origin
  through FLIGHT and ROLL instead of teleporting after the ball.
- Gameplay renderer projects golfer and ball separately using the same
  camera scale.
- New test: `PixteePhysicsFidelityTest.golferRemainsAtLaunchPointWhileBallTravels`.
- This fixes "golfer chases ball" positioning, but final sprite clubhead
  **pixel-perfect contact registration** remains art/rig approval work.
- No flight coefficients, surface friction or hazards were modified.

## Sprint 2 — approved visual contract
- Locked concept quality and details in `PIXTEE_CANONICAL_VISUAL_2026-10-10.md`.
- Existing `ART_REVIEW_REGISTER.json` SHA-256 + explicit owner approval
  remains the only path into production artwork.
- Rejected programmer sprite sheet remains deleted.
- No new production PNG or fake characters added.
- No image-generation collage is treated as a production sprite atlas.

## Sprint 3 — real-time menu engine
- `MenuAttractMode.kt`: three isolated `PixteeCore` golfers each run genuine
  power/accuracy/flight/roll sequences in the course backdrop.
- Menu simulation advances at the fixed 70 Hz gameplay tick; rendering
  requests approximately 30 Hz in an active menu, with an existing motion
  toggle supporting a static accessibility mode.
- Menu uses same world-course renderer, separate from the human game state.
- Developer title/menu geometry still requires the production-grade artwork.
- Ad board geometry increased from 35x13 to 42x16 world units, visual-only.
- New JVM tests check deterministic animated golfers, independence from player
  scores/saves, no state corruption and bounded ball coordinates.
- New Android smoke test waits for live menu simulation seconds to advance
  and navigates to actual course selector using touch.

## Sprint 4 — faithful gameplay course overview
- Replaced generic hard-coded fake minimap with projection from each actual
  current hole's pin, tee, fairway curve, sand patches, water patches and
  live ball coordinates.
- Small HUD map never affects physics coordinates or collisions.
- Unit tests cover 35-course geometric sampling, projector reversibility
  and no mutation of game state.
- The 31-pixel minimap needed 0.001 world-unit numerical tolerance for the
  Float reverse projection; this was corrected after a CI test failure.
- Sponsor inventory placement remains physics-neutral.

## Verification / release discipline
- `Pixtee Android Portrait` is the authoritative CI workflow.
- First combined run 38076977214: compile worked; 71/72 unit tests passed;
  one minimap floating-point test was overly strict.
- Corrected in commit `8f36af6c06d5f6df071b3b501e3c007a55a2a6bd`.
- The subsequent `38077133977` build and unit test stage passed;
  Android emulator interaction tests were in progress when this worklog
  was written. Do NOT mark emulator tests passed until job conclusion.
- A compiled debug APK is an **engine-validation artifact only** until all
  user-approved individual art assets are actually in place and checked.
- The release art validation deliberately fails for unapproved assets.

## Remaining mandatory production gates (not finished)
1. Export pixel-perfect **production** golfer, club/ball contact rig and
   all character poses; owner approves each exact file, then SHA lock.
2. Genuine original course terrain art, crowd/spectator motion and backgrounds
   to the approved high standard, with individual approvals.
3. Proper title/logo/wooden button/menu HUD assets, animated menu and
   cute spectators all visible in emulator; no placeholder renderer.
4. Pixel-level screenshot comparison with original gameplay to establish
   camera/model ratios; 1.9x is a candidate, not proven exact.
5. Numerically verified golf ball/club/terrain/cup/roll parity matrix across
   13 clubs, 35 courses, hazards, greens, putts and edge cases.
6. Better distinctive 35-course content; deterministic seed variants do not
   equal 630 independently hand-balanced holes.
7. Fold closed/open interaction, actual user-device QA, performance and
   accessibility review, compliant ad integration, final signed release.

No art approval may be inferred from a successful build or emulator suite.
