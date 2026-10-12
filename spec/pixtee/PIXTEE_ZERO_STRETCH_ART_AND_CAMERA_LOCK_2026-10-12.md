# PIXTEE GOLF — ZERO-STRETCH ART, CAMERA AND PHYSICS-ALIGNMENT LOCK
**12 October 2026 | Owner explicit directive | MUST NOT REGRESS**

> **DO NOT STRETCH ANY GRAPHICS.** NO horizontal-only scaling, vertical-only scaling, aspect-ratio distortion, UI/image stretching, deformed course sprites, reflowed textures, blurry texture filtering, or distorting approved art to match a viewport or physics map. This restriction is non-negotiable and overrides convenience-based resizing ideas elsewhere in engineering notes.

## 1. Protected references
- Original owner-approved `Pixtee golf assets(1).zip`, SHA256 `2043e9c678b9904311778d17a1f341d577344e6d49a3a9d48ce6c3aef3b5239e`.
- Original owner-approved `pixtee-lakewood-open.zip`, SHA256 `82c0df6831241e31ad0c83a4302df145349afb9b63503487a914b964d05c7ff3`.
- Newer `pixtee-production-standards-v1-1.zip`, SHA256 `1544ac4782b64d2a2b968a581f31b9cf7b37b6e0cf4583dd665e21d1d81ffe17`.
- Both original ZIPs are preserved outside the working repository, and the engine/archive branches are immutable by owner policy. Never modify them.

## 2. Render and camera rules
1. Render the authored course geometry **without changing its width:height proportions**. The HTML Lakewood visual world is **544 × 912 authored units**. Never fit that entire world into a phone screen by distorting its dimensions.
2. The default playable camera shows a *crop* of the course and follows the authoritative native ball position. Adjust the viewport / camera rectangle to closed Fold, open Fold and ordinary portrait phone; do **not** adjust world dimensions or physics when the aspect ratio changes.
3. Any game-world zoom must be **uniform** (same factor in X and Y). For pixel-perfect assets use integral pixel steps when necessary and nearest-neighbour filtering; avoid subpixel jitter and introduce separate clean HUD compositions rather than stretched bitmaps.
4. A shader can animate artwork (e.g. subtle preapproved grass bend, flowers, water/flag frames) without changing the source asset or its overall dimensions. Never deform approved static objects or mask collisions to make visuals fit.
5. The golfer, birds, spectators, sponsor boards, water edges, sand, grass, trees, ball and visual target X must remain visually faithful at the approved size and proportions. UI images are not resizable wallpaper: use correct-size assets and reposition/clip/recompose the menu/HUD around screen sizes.
6. Preserve visual anchor coordinates, approved camera viewpoint, course layout, paths and sponsor placement; no graphics on pathways and no unnecessary bushes.

## 3. Critical correction: do not apply 0.947 world scaling to a mismatched physics map

An earlier provisional green-fit proposal used `0.947` as a **native gameplay coordinate conversion**. Although a uniform transform does not *geometrically stretch pixels*, it risks shifting tee, hazards, trees, green boundaries and cup relative to the frozen HTML course. **That 0.947 candidate is NOT approved as production. DO NOT use it for visible sprites or silently incorporate it into gameplay resources.**

Instead:
- Inspect and preserve the exact authored green polygon and original engine's MAPS lookup coordinate window, transition marker requirements and playable world units.
- Build a reversible **coordinate-mapping contract** whose sampled physical surfaces correspond to the visually identical positions. If the original green detail window is smaller than the full visual green, **record this as an open incompatibility**, do not compress / clip / scale the approved green or substitute arbitrary physics.
- Green MAPS detailed mode may require a separate engine-compatible representation or revised artistic *physics annotation*; proposed physical data must be validated on the original image/visual outline at tee, cup, green rim and all transitions, with original C++ fixed-tick core unchanged. Ask for owner approval before changing any visual, world collision boundaries or gameplay green mapping.
- Physical occupancy and graphical object positions must agree; collision masks are authored separately but tested against same world point sampling. No dynamic adjustment of ball physics to make art fit.

## 4. Fail-closed acceptance gates before any playable APK or course signoff

- **PIXEL-01:** Exact source PNG / HTML SHA-256 integrity matches; no source file or approved sprite changed by import.
- **PIXEL-02:** Same X:Y magnification and proportions for every sprite and course element. No inherited node scale anisotropy; no shader aspect-ratio warp or nine-patch stretch used on pixel-art world assets.
- **PIXEL-03:** Overlay visual screenshots at original authored resolution and game camera crop; no distortion or stretched geometry at any point.
- **PIXEL-04:** Tested normal portrait phone and Fold closed/open; same world geometry and collision independent of resolution. Difference is only camera crop and HUD composition.
- **PIXEL-05:** Native ball, tree, hazard, tee, green, cup and sprite positions coincide within authored game-world resolution and native 2×2 fine MAPI sampling; no secret `0.947` workaround.
- **PIXEL-06:** Native 70 Hz fixed-tick physics and deterministic replay identical at different rendering refresh rates and zoom levels.
- **PIXEL-07:** VFX (foliage, water, sand, grass, birds, spectators, lighting, flag) remain appropriate, clipped to course masks, do not distort artwork or affect real collisions.
- **PIXEL-08:** Unresolved green-map world compatibility keeps the course **NOT_PRODUCTION_APPROVED**, even if an attractive Godot screenshot renders correctly.

**Blocking rule:** if an asset won't fit a layout, change the viewport/crop, layout surrounding it, or supply an owner-approved correctly authored derivative. Never stretch it.
