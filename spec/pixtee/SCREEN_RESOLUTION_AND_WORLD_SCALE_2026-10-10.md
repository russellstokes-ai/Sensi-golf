# Pixtee Golf — Source-Derived Screen Resolution & Gameplay Geometry Contract
**Owner decision:** replicate *apparent sizes and real mechanics*, improve detail through additional pixels, **never tilt camera or fit the full course on a phone screen**. Designed for standard portrait Android phones first; Fold closed/open is additional adaptive support.

## Actual measured original game facts — NOT screenshot estimates

- Selected Sensible Golf PC v1.014 hashes: `GOLFWIN.EXE` SHA256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`, `GOLF.EPF` SHA256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`.
- Original game bitmaps `320×200` and `320×257` pixel rectangles (resource file BMHD dimensions, **not proven final gameplay viewport**).
- Every original course map: **34×114** original tile cells; each tile covers **16×8 gameplay world units** ⇒ **544×912 world units** per original main resource (all **72/72** source resources).
- MAPI collision precision: **8×4** binary subcells per tile ⇒ **272×456** cells on main map, each **2×2** world units. This is **terrain geometry**; object/tree hitboxes are separate, not recoverable from sprite frame dimensions alone.
- Original green map dimensions: width **304..416**, height **240..272** gameplay world units depending on 72 original map files, with detailed green coordinate transitions and putt slope nibble encoding.
- Source extraction **PASSED** [all-object/original-course artifact #11684037276](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38090500528/artifacts/11684037276). Includes 72 original per-course tile/green sizes, terrain collision descriptor histograms, slope direction/magnitude distributions, original tee/cup coordinate metadata, **277** file names/hashes, **874** MCH individual frame bounds, 13 club physics rows/13x10 lie selectors/11 accuracy profiles/77 terrains. Source reproducible from `tools/extract_sensible_complete_dimensions_and_collision_reference.py`.
- Sprite inventory **PASSED** [#11683403911](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38090116988/artifacts/11683403911): 15 original MCH sources + 19 LBM bitmaps, with **padded width/height and painted bounding rectangles** for each frame; transparent/blank frame slots retained. `GOLFS1.MCH` has **72 × 16×21px** frames; `GOLFB1.MCH` **48 × 32×32px**; frame naming alone does not establish which is rendered as the playable golfer.
- Verify original rendering-to-world source coordinate transform, entity anchors, collision hierarchy, and frame animation timing **from EXE calls and emulator captures** before claiming pixel-perfect original onscreen size. Frame size does NOT establish an object hitbox.

## Current Pixtee prototype viewport — actual code, not proposal

`PixteeCore.WIDTH=300`, `HEIGHT=510` in prototype, *not the original 544×912 logical world*. `PixteeActivity.kt` draws into a **360-unit wide** logical UI space, with **logicalHeight=actualWindowHeight*360/actualWindowWidth**. Native rendering multiplies these logical units by `actualWindowWidth/360`. Current provisional ball-following camera zoom is `1.9` and world width is `300`.

- `CourseViewport.worldScale=(360/300)*1.9 = **2.28** logical screen pixels per Pixtee world unit`.
- Visible world width = `360 / 2.28 = **157.895** Pixtee world units`, independent of screen DPI.
- Current candidate golfer world-height `11.5` ⇒ **26.22 logical px** on a 360-unit wide layout ⇒ **7.283%** of gameplay canvas width. This is a **Pixtee candidate**, not directly proven original onscreen golfer size.
- Original source 21px (if the relevant 16×21 resource is the on-course golfer) against original 320px source rectangle is 6.563% of that file width; not a measured display proportion until original renderer mapping is known. Do not auto-adjust 26.22 down to 23.625 on this provisional ratio alone.
- For a source art frame `16×21`, 2× art canvas `32×42`; 3× `48×63`; 4× `64×84`. **These are artist pixels only, not world dimensions or scaled collisions.** The source 128×64 Pixtee swing sheet includes padding and pivot anchors; use its painted content/world registered feet position, not the entire canvas, for visible size.

## Calculated normal-phone and Fold/window-size matrix

Input examples are **window-pixel sizes**, illustrative Android phone resolutions, *not claims about a particular model's dimensions*. Inset-adjusted window may differ from hardware panel; track density and cutouts separately.

| Target | Window px W×H | Pixtee logical W×H | Native px / logical | Candidate golfer screen px | Visible Pixtee world H (1.9 zoom) |
|---|---:|---:|---:|---:|---:|
| Compact standard phone | 720×1600 | 360×800 | 2 | 52.44 | 350.88 |
| Standard portrait phone | 1080×2340 | 360×780 | 3 | 78.66 | 342.11 |
| Tall portrait phone | 1080×2400 | 360×800 | 3 | 78.66 | 350.88 |
| Shorter portrait phone | 1080×2160 | 360×720 | 3 | 78.66 | 315.79 |
| Hi-res normal phone | 1440×3120 | 360×780 | 4 | 104.88 | 342.11 |
| Illustrative unfolded wide window | 2208×1840 | 360×300 | 6.133 | 160.82 | 131.58 |

**This is what the current Pixtee renderer WOULD do**; numbers reflect screen pixels and not physical millimetres or display-density-independent pixels. On normal phone portrait variants: wider/high-res devices have equal *relative golfer-to-fairway scale* and height differences only expose more/less longitudinal terrain. **Do not rescale course length to match physical screen height**.

In a short/wide/unfolded window the current static `drawMeter` y=450 is **offscreen** on a logical 360×300 window; static minimap `y=38..151` and gameplay overlays overlap play. This is a proven architectural adaptability gap, not permission to redesign the approved course. Use **HUD-only adaptive reflow** with game camera still overhead and world units unchanged. Fold CLOSED behaves as a tall normal phone.

## Exact rendering and asset scaling formulas

Let `P_x,P_y` be ball/tree/golfer coordinates in **game world units**, and `C_x,C_y` camera top-left, `S` a **single identical** logical-screen-per-world multiplier. Let `N=windowWidthPixels/360`.

```text
S = 360 / designedWorldWidth * cameraZoom
screenLogicalX = (worldX-cameraLeftWorld)*S
screenLogicalY = (worldY-cameraTopWorld)*S
screenDevicePxX = screenLogicalX*N
screenDevicePxY = screenLogicalY*N
drawnDevicePxHeight = entityWorldHeight*S*N
visibleWorldW = 360/S
visibleWorldH = (360*windowHeight/windowWidth)/S
```

Never apply independent X/Y stretching or multiply original physics positions by `N` or the sprite-source art detail factor. For zoom, preserve world height/radius/collision bounds; render using a camera matrix. For crop and HUD, favour moving UI panels, not moving the ball or enlarging the course to fill the screen. The white target X and ball use same world-to-screen matrix and the X hit target maps touch through the inverse matrix.

**Pixel art fidelity:** create *new, original* artist sprites at sufficient detail (typically 3× or 4× authored source density), use opaque bounding boxes to measure visual proportions, align art with registered physical feet/contact pivots. The source image can be resampled to any Android native resolution while the physical projected world size stays invariant. Verify visual nearest-neighbour / dither / sampling quality with screenshots. Do not tile pixel bitmaps in raw unscaled device-pixel dimensions if course is mapped in world units: terrain repeats must be world anchored to prevent drift on zoom.

## Gameplay/physics: what MUST and MUST NOT change

1. **No screen resolution changes to gameplay**: the same start world point, club index, raw power `0..105`, accuracy tick (63 straight), 4096-unit heading, original physics delta/drag/bounce, MAPI surface/collision code, slope/tick model, tree object/hazard contacts, RNG state and cup results must produce **identical world trajectories** on 720px, 1080px, 1440px, folded/unfolded and 60/90/120Hz displays. Game core keeps original logical timer **936/65536s** and deterministic event capture, independently of rendering FPS.
2. **Prototype physics still approximate**: `PixteeCore.kt` invents `sin(π·position)` power and fractional accuracy, `Club.yards` and loft/ball flight; it caps XY, uses one cosmetic bounce and broad ground types. It is NOT original parity. Replacing that with an independently authored, original-reference-calibrated deterministic simulator is the true gameplay work, **not** an aspect-ratio adjustment. Do not copy original proprietary code/tables without legal rights.
3. **Original gameplay world mapping**: original main map = 544×912 logical world. Prototype = 300×510 *different arbitrary world*. To switch source-native world geometry without distortion, uniformly rebase existing Pixtee authored coordinates by `s=912/510=1.7882353`; width maps to `536.47` leaving `(544-536.47)/2≈3.76` world-unit side margin. Transform points **and** shapes/radii/velocity parameters/slope coordinate bases together; do **not** independently stretch X by 544/300 (1.81333) and Y by 912/510 (1.78824). Longer-term design all new Pixtee courses directly in 544×912 world units, preserve original course-size scale but with original *Pixtee* terrain designs.
4. **Keep camera framing** during a world-unit rebase: if golfer worldHeight `11.5` becomes `20.565`, preserve the current 26.22 logical px by changing logical pixel/world scale to `S'=2.28/1.7882353≈1.275`. With world width 544, derived zoom must be `S'*544/360≈1.927`, **not** blindly keep `1.9`. This is an algebraic equivalence to avoid an accidental visual zoom change; actual final scale needs original game camera screenshot calibration.
5. **Visual/reference collision separation**: render bounds and painted bboxes are not hitboxes. Terrain sampling at 2×2 game units, water 35 hazard stop vs sand code 7 normal lie, fine-grained greens, actual tree contact zones/height response pending further trace. Colliders must stay fixed when artist increases pixels or client changes DPI; don't make sponsor boards or spectators collide.
6. **Controls**: preserve 3-tap combined power/accuracy reverse-travelling **horizontal** WHACK-O-METER; target X in world with touch left/right steering (slow hold), club-tap cycle; pointer position/timing must be fixed game tick samples not UI animation deltas. Minimum physical touch target **48dp**, assessed using Android `density` not guessed from native 720/1080 pixels. Onboarding explains; no instructional captions on course.
7. **Safe areas and HUD**: Android WindowInsets/density/font-scale, camera cutouts and bottom navigation areas must be accounted for in visible content. Render course edge-to-edge behind controls without occluding ball at contact and meter; do not pretend `360x760` is a standard phone. Profile by available window *aspect ratio* / width class, not model name, arbitrary physical pixel count, or whether folded.
8. **Animation sizes/timing**: original collision/simulation tick cadence remains independent; sprite frames (swing, splash, tree hit, bounce, green arrows, ball lie) keyframe from simulation events and source-bitmapped frame cadence after original binary recovery. FPS/resolution must not change time-to-impact or ball trajectory.

## Engineering implementation order and acceptance tests

| Gate | Needed change | Regression proof |
|---|---|---|
| R1 | Lock numerical source measurement, exact per-frame bbox, full 72 game source course units and MAPI collisions | Source SHA256 + extraction CI #38090500528 **PASS** |
| R2 | Create explicit `WorldGeometry` game world coordinates/collision-space separate from `ArtDensity` source pixel resolution and viewport transforms | Same object size/world position when source art changed 2×→4× |
| R3 | Normal portrait phone matrix (320–440dp window widths, 19:9–22:9), safe insets, 48dp touches, camera ball follow and 3-click meter visible | 720×1600 / 1080×2340 / 1080×2400 / 1440×3120 in Android emulator screenshots |
| R4 | Short-window/fold/tablet HUD-only reflow, meter/minimap pinned, same top-down world scale | 360×300/360×360 logical tests and unfolded screenshot |
| R5 | Game world 544×912 scale migration (if validated against original zoom and current Pixtee artwork) | No geometry stretch; 2×2 course collision sample; source-to-Pixtee trace comparisons |
| R6 | Replace provisional physics/meter with captured binary-reference-calibrated behavior | Launch/contact/rest, club/lie, slope/putt, tree/hazard/cup, shot timing and RNG exact target trace tolerances |
| R7 | Original-referenced object frame/render anchors and effect animations with approved Pixtee original art | On-device original-vs-Pixtee matched-world-size screenshot A/B with measured deviations |

**Lock:** Do not modify the approved art direction, course viewpoint, phone HUD language or golfer proportions during pure scaling work. Current dimensions and new conversion are engineering recommendations/acceptance criteria, NOT evidence that all gameplay/UI changes have already been made.

Machine-readable exact calculations / example phone matrix: [SCREEN_RESOLUTION_AND_WORLD_SCALE_2026-10-10.json](SCREEN_RESOLUTION_AND_WORLD_SCALE_2026-10-10.json).
