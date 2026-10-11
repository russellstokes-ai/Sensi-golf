# Pixtee Golf — top-down camera and scale calibration gate

**Owner direction (2026-10-10):** Match the observable camera perspective,
on-course golfer scale, terrain proportions, and ball/green presentation of
Sensible Golf as closely as legally permissible. Original physics/gameplay
comparison is separate from visual camera projection. A closer playable view
was requested; the illustrated concept's character/person style was requested
to be a little cuter. **No art asset is approved yet.**

## Baseline visual evidence and limitations

The original game appears in public screenshots with a fixed top-down
perspective, part of the hole visible and a substantial gameplay HUD/sidebar:
- https://retro.gg/game/sensible-golf/349
- https://www.mobygames.com/game/2530/sensible-golf/screenshots/
- https://www.lemonamiga.com/game/sensible-golf

These references are only comparisons. Do NOT import, trace, recolour,
sharpen, or republish the original artwork or exact course bitmaps.

**Important:** Original executable world units, resource tile size, and shot
coordinates are not the same thing as screen pixels. No released original
screenshot has yet been paired with authoritative original-game camera
projection variables or exact native device-pixel scaling. Do not describe
the current candidate as measured pixel-perfect parity.

## Executed camera model in Pixtee

- Logical game view 360 units wide; Android Canvas scales uniformly to device.
- Physics world remains 300 x 510 units; neither flight nor hazards nor clubs
  depend on camera scale.
- Old viewport represented the whole 300 world units horizontally and revealed
  almost all 510 vertically on tall portrait screens; feedback: **too small**.
- New reference-camera candidate: uniform zoom 1.9x relative to
  the earlier phone-world projection. The base 360 / 300 = 1.2
  logical screen units per world unit, so actual worldScale = **2.28**.
  The visible width is 360 / 2.28 ≈ **158 world units**, height at
  360x760 is 760 / 2.28 ≈ **333 world units** (versus 510 total).
  This crops rather than shrinking the game world.
- Camera follows the actual ball's world coordinates with 52 world units of
  anticipation toward the green, clamping gracefully at world edges. HUD,
  meter and pause remain screen-space controls.
- World-to-screen and screen-to-world projections are reversible and use
  the **same X/Y scale**. Golfer PNG sprite and ball are both drawn at that
  worldScale; they must never remain screen-sized while terrain zooms.
- No automatic stretch to fit unusual Fold viewports. Wider screen layouts
  should be addressed in a separate approved camera/layout decision.

## Asset-size review gate — candidate ratios, not final dimensions

With ~158 world units visible across the playfield, an
11.5-world-unit golfer projects to about **26 logical pixels high** on a
360-wide preview, or about **7.3% of gameplay width**. This is a
**visual review hypothesis**, not an exact recovered original
sprite size. Approve only after direct original-to-Pixtee screenshots with
the same subject (golfer beside ball, green and bunker) at matching apparent
zoom and playback state.

Review these fixed framing situations:
1. Tee shot on a par 4: golfer must be visible above gameplay controls;
   hazards are readable while the distant green need not be visible.
2. Mid-fairway approach: camera follows the ball without jumping across frames.
3. Near-green chip and putt: show usable green and slopes without stretching.
4. Ball near any edge or water: clamp view and keep ball visible.
5. Standard tall handset and compact Fold-like viewport: controls remain
   tappable; green/ball model ratio stays unchanged.

## Art approval

The owner must explicitly approve:
- camera scale and golfer/ball visual ratio in actual-phone screenshot,
- cute-but-classic original golfer/spectators,
- original terrain, course scenery and sponsorship board scale,
- animation frames and HUD/menu artwork.

Do not add any illustrated proposal to Android production assets just
because a preview was shown. All approved individual files require the
owner approval reference and SHA-256 in
`spec/pixtee/ART_REVIEW_REGISTER.json`, and CI + Gradle release validation.
