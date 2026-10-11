# PIXTEE GOLF — SPRINTS 1–5 IMPLEMENTATION + OWNER ART REVIEW

## Approved north star
The owner approved the lush high-quality top-down Pixtee Golf concept image,
including live course-backed wooden-button menu, world-space sponsor boards,
classic close camera, 1990s arcade scale, friendly golfers and spectators,
very polished natural landscapes, and faithful observable Sensible Golf gameplay.
The owner explicitly rejected all programmer placeholder sprites.

User feedback: all illustrated swing poses are good EXCEPT the follow-through
club orientation. Fixing only its club rotation is necessary, but every
intermediate pose needs continuous motion. Request intermittent photographer
camera flashes and occasional birds crossing the course.

## Sprint 1 — ENGINE AND SCALE (IMPLEMENTED FOUNDATION)
- Independent 70Hz Kotlin golf simulation, 13 clubs, repeatable holes.
- World-fixed golfer address pivot while ball follows independent physics.
- 1.9x closer ball-following portrait camera candidate (not exact proven SG parity).
- Three-click shot semantics, fairway/rough/green/sand/water and penalty behaviour.
- NEW: dedicated visual tick counters in PixteeCore: backswingTicks,
  downswingTicks, postContactTicks, independent of shot physics.
- Frame-zero contact coordinate is the exact physical launch coordinate.
- Tests added for pose timings, camera projections and contact registration.

STILL NEEDED: measured club-by-club/terrain-by-terrain parity with external
observable original behaviour. Do NOT call parity achieved or exact.

## Sprint 2 — CORE ART ASSET PIPELINE (INTEGRATED, ASSETS PENDING)
- Approved PNG loader exists, never invents or substitutes sprites.
- Normal release art is SHA-256 locked to individual owner sign-offs.
- NEW: approved-only terrain shader tiling with five art slots:
  terrain_rough, terrain_fairway, terrain_green, terrain_sand, terrain_water.
- NEW: approved-only UI bitmap loader with four slots:
  ui_pixtee_logo, ui_wood_button, ui_hud_panel, ui_sponsor_board.
- NEW: a separately marked `src/debug/assets/art/review` intake path can
  show genuine candidate PNGs in *debuggable* builds only. Review art can never
  authorise production release and does not appear by default.
- Total production requirement now: 9 golfer frames, 10 scene/ambient
  sprites, 5 terrain tiles and 4 interface bitmaps = **28 PNG slots**.
- No production PNGs have been signed off or bundled. This is a hard
  blocker, not a cosmetic detail.

## Sprint 3 — ENVIRONMENT (BEHAVIOUR IMPLEMENTED, ART PENDING)
- Shared world renderer is used by both the playing round and live menu.
- Existing real hole geometry gives fairway, green, bunker and water masks.
- Dynamic ambient cycle: water ripples, grass sway, spectator movement.
- NEW: short photographer flash approximately every 18 seconds, for designated
  real spectator points only; never a constantly flashing light.
- NEW: pairs of bird sprites cross occasionally with changing wing frames.
- Both the flash and birds are disabled by the existing course-motion toggle.
- No bird silhouettes or camera-flash placeholders are generated.
- Expected actual artwork: bird_wings_up, bird_wings_down,
  spectator_photographer, camera_flash.
- No modifications to ground collision or ball steering.

STILL NEEDED: production-grade textured environments, authored trees,
flower/rock/water edging, and distinctive scenic layouts across 35 courses.

## Sprint 4 — TITLE SCREEN AND HUD (RENDERING INTEGRATED, ART PENDING)
- Existing live title course has 3 isolated real-simulation golfers,
  independent of player saves, XP and season results.
- NEW: ui_pixtee_logo overlays menu title when approved art exists.
- NEW: ui_wood_button and ui_hud_panel replace development-only Canvas
  fills when approved bitmaps exist.
- NEW: ui_sponsor_board supports approved artist-led board frames, retaining
  sponsor text and any paid disclosure in-world.
- 42x16 world-unit sponsor boards (up from 35x13).
- Minimap reflects current real hole fairway, water, bunkers, cup and ball.
- Interactive button hitboxes remain unchanged.

STILL NEEDED: final on-device title/HUD/wood panel graphics and sizing;
the currently executing builds do not resemble the approved concept.

## Sprint 5 — CHARACTERS AND SWING (TIMELINE BUILT, ART PENDING)
- NEW: independent eight-step swing sequence:
  ADDRESS -> TAKEAWAY -> BACKSWING -> TOP -> DOWNSWING -> IMPACT ->
  FOLLOW THROUGH -> FINISH; PUTT as a separate ninth pose.
- Actual golfer swing phase clocks drive which approved bitmap is shown.
- Impact pose is reserved for contact, not used throughout downswing.
- Follow-through begins AFTER impact, with a separate finish frame.
- All 9 frame PNG exports must use a common 128x64 RGBA8 transparent canvas,
  the same registered feet at pixel (40,60), and impact clubface point (92,60).
- 52 source pixels correspond to 12 world units between the golfer's feet
  and exact physical ball centre. No independent frame centring or stretching.
- NEW: Python export validator decodes actual PNGs, checks RGBA, dimensions,
  feet pixel opacity and impact clubface pixel, run in CI with unit tests.

CRITICAL REVIEW GATE: the submitted follow-through CLUB ORIENTATION from the
sample was rejected; actual source frame must have the shaft above/around
the lead shoulder in the correct grip and direction, not hanging down or
reversed. Pose silhouettes and ease/continuity must pass real device-scale
review. A correct anchor alone is NOT enough.

## User approval gates — no implied signoff
1. **Actual swing animation:** submit the actual exported 9 transparent PNGs,
   sprite atlas contact sheet, real-game-scale preview and 70Hz animation
   recording. Inspect follow-through club angle and physical ball contact.
2. **Environment:** actual 5 terrain textures with natural edge transitions,
   trees, bushes, spectators, photographer/flash and two-frame birds.
3. **Title/HUD/boards:** actual image sources + live Android recording,
   verifying wood buttons, legibility, logo, 42x16 boards and user touch.
4. **Course variety:** actual original hole scenery and collision/camera
   view at comparable scale, not a conceptual overview image.
5. **Signature:** owner approval of exact file revisions and their SHA-256
   before assets enter `src/main/assets/art/production`.

## Test and release status
Source code changes go through GitHub Actions workflow `Pixtee Android Portrait`.
CI now adds tests for pose order/contact registration, intermittent ambience,
PNG export validator, existing gameplay physics, menu interactions and Android
emulator touch tests. Release MUST remain blocked until owner-approved actual
production assets exist, with all 28 required file hashes.

**Not a completed visual sprint pack yet:** these sprints implement the engine
systems and asset interfaces, but actual artwork has not been fabricated or
silently extracted from a concept image. No art should be called approved
merely because code and Android tests pass.
