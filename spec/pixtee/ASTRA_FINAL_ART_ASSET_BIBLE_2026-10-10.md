# PIXTEE GOLF — ASTRA FINAL PRODUCTION ART BIBLE & ASSET REGISTER
**Version:** 1.1 — Expanded wildlife, impact and advertising coverage • **Reference audit:** 10 October 2026 • **Owner baseline:** approved premium pixel-art montage and true overhead Sensible Golf gameplay scale • **Branch:** `feat/pixtee-native-portrait-framework`

**ART COMPLETENESS UPDATE:** See §11 and [the full biome/impact manifest](ASTRA_AMBIENCE_AND_IMPACT_MANIFEST_v2.json): 21 region-appropriate bird species with harmless comic ball-hit sequences, subtle flowers/grass, detailed crowds/photographer, mandatory sand/water/tree FX and boards facing **tee and green**. **§11 supersedes any older minimal lists or 384-frame golfer target; final proposed golfer programme is 576 frames.**

**Owner's instruction:** Create **EVERY original Pixtee art asset needed for a finished Android golf game**, ready for actual production integration—not placeholder art or static posters. Preserve Sensible Golf's verified gameplay and **relative visual world proportions**, use original authored art with more detail and richer animation; do not change the view, scale, course length, collision maps or physics to suit the art. Standard portrait Android phones are the **primary** experience. Closed/open Fold must also work. **Do not generate an angled camera, fake full-hole-on-screen scene, conventional power slider or inaccurate course-size layout.**

## 0. NON-NEGOTIABLE: measured vs proposed numbers

Tag every reference measurement and export target correctly:

- **[ORIGINAL-FILE VERIFIED]** Historical Sensible Golf v1.014 data: original `GOLFWIN.EXE` SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`, original `GOLF.EPF` SHA-256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`.
- **[ORIGINAL-FILE VERIFIED]** Every original source course map has **34×114** tiles, each **16×8** game-world units ⇒ **544×912 world units**. Fine collision grid **272×456**, cell **2×2** world units. 72 source course resources (not 72 Pixtee courses). The detailed green raster size varies **304–416 world wide** and **240–272 world tall**. These are *world* dimensions and binary-map resolutions, **not** physical display pixels.
- **[ORIGINAL-FILE VERIFIED]** `GOLFS1.MCH` = 72 individual **16×21 px** padded animation frames (actual painted bbox width varies 7–13, height 16–19); `GOLFB1.MCH` = 48 individual **32×32 px** frames (painted width 15–21, height 29–31). Other 15 MCH resources total **874 source frames**. The game's 19 LBM resources are measured at **320×200** or **320×257 px**. **Which source frame is the playable golfer and its actual rendered screen-size must still be independently verified from draw calls**; a frame rectangle does **not** prove world size, painted silhouette, camera or collision box.
- **[CURRENT CODE VERIFIED]** `PixteeSwingRig.kt`: existing production golfer source canvas **128×64**, feet anchor **(40,58)**, club contact **(54,58)**, spacing **3.5 game-world units** and **4 source px/world**. `ProductionPixelArt.kt` currently REJECTS all golfer PNGs not exactly 128×64. Its full padded frame covers **32×16 world units**, but its nominal painted visual height goal **11.5 world units** does *not* mean the whole padded frame is only 11.5 high. Do not confuse those values.
- **[PROPOSED FINAL EXPORT — CODE CHANGE REQUIRED]** Exactly double *the current Pixtee rig*: **256×128 RGBA golfer sprite canvas**, feet pivot **(80,116)**, impact clubface **(108,116)** ⇒ contact difference 28 px = 3.5 world units at **8 art pixels/world**. Same **32×16 world-unit padded frame**, same actual shot-ball contact coordinate. Target about **92 painted source px high** (11.5 world × 8); transparent padding is intentional and does not affect gameplay. This is a Pixtee art production engineering target, **NOT an assertion Sensible Golf originally used 256×128 sprites**. Engineer must update `PixteeSwingRig.kt`/`ProductionPixelArt.kt` atomically and add a verified contact test before accepting the final files. Until then, the compatibility export is only **128×64** using the original 40/58 and 54/58 anchors.
- **[PROPOSED FINAL EXPORT]** Other **world art** uses a convenient **8 authoring pixels/world unit** (16×8 original-equivalent tile footprint ⇒ 128×64 Pixtee artwork), independent of on-screen zoom. UI artwork uses **4 pixels per 360-width logical UI unit**. Neither number changes collision sampling. Final pixel sizes below are **explicit requested export targets** rather than unproven original source sprite sizes.
- **[PENDING SOURCE CALIBRATION]** Original renderer's exact pixel-to-world conversion, visible golfer/ball/contact proportions, individual tree collision boxes/height, original meter red/yellow pixel distribution, and original effects frame timings **have not been proven**; Astra must flag them for comparison rather than inventing claims of equality.

**Verified source files:** [Original frame/asset audit](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38090116988/artifacts/11683403911), [complete 72-course and collision audit](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38090500528/artifacts/11684037276), [numerical source physics reference](ORIGINAL_PHYSICS_RESEARCH.json), [full original-file source ledger](ORIGINAL_BINARY_HARD_DATA_MASTER_2026-10-10.md), and [phone/fold world scale and geometry rules](SCREEN_RESOLUTION_AND_WORLD_SCALE_2026-10-10.md).

## 1. Where Astra MUST save the art — exact repository locations

**Production source-of-truth Git repository:** [`russellstokes-ai/Sensi-golf`, Pixtee development branch `feat/pixtee-native-portrait-framework`](https://github.com/russellstokes-ai/Sensi-golf/tree/feat/pixtee-native-portrait-framework). Do **not** commit the original Sensible Golf binaries, tiles, sprites, fonts, or music. Do **not** commit art to restoration `main`.

| Use | Save to (relative to repository root) | Contents |
|---|---|---|
| **Editable masters** | `art/source/<category>/<asset>.aseprite` or `.psd`/`.svg` | Layered editable original artwork; keep frame timeline, pivots and colour palette version; organized category folders |
| **Review packs** | `art/previews/<batch>/<name>.png` | Real-size gameplay crops **and** enlarged pixel-view contact sheets, preview GIF/APNG where helpful; no approval inferred |
| **Android unapproved development previews** | `pixtee-android/app/src/main/assets/art/review/<asset_id>.png` | Flat one-PNG-per-ID filenames currently recognized by the loader. Add `REVIEW_ONLY.txt` with exactly `PIXTEE_ART_REVIEW_ONLY_NOT_APPROVED_V1` **only in debug builds** |
| **Android approved/production art** | `pixtee-android/app/src/main/assets/art/production/<asset_id>.png` | **FLAT directory**, `<id>.png`, because the CURRENT `ProductionPixelArt.kt` loader opens `art/production/$id.png` and has a hard-coded ID allowlist. For new assets first update the allowlist/manifest loader. |
| **Approval and hashes** | `spec/pixtee/ART_REVIEW_REGISTER.json` | Per-asset revision, exact filename, SHA-256, approval, authored export size, in-world painted and collision footprint, reviewer, date; **never invent owner approval** |
| **Runtime manifest (NEW; engineering task)** | `pixtee-android/app/src/main/assets/art/production/assets.manifest.json` | Explicit ID, image path, pixel WxH, world WxH or UI logical WxH, 8px/world or UI-4×, transparent pivot, animation frame count, atlas reference, intended usage and collision-tag |
| **Production gate** | `pixtee-android/app/src/main/assets/art/production/APPROVED_v1.txt` | Contains exactly `PIXTEE_PRODUCTION_ART_APPROVED_V1` only **after all required art is explicitly approved and tested**. Do not generate it prematurely. |
| **Original course design files** | `art/source/courses/c01...c35/` | 35 completely original themes, vegetation sets, tile variations, thumbnails, scenery design and 18 distinct hole plans each |
| **Playable hole geometry** | `pixtee-android/app/src/main/assets/courses/c01/h01.json` ... `c35/h18.json` | **630 independently authored, play-tested hole definitions** with original collision masks, greens/slope, tree objects and tee/cup anchors; engineering must implement/validate file loader. Not original Sensi layouts. |
| **App icons / splash** | `pixtee-android/app/src/main/res/mipmap-*/` and `res/drawable*/` | Android launcher/adaptive icon and splash resource variants derived from approved Pixtee originals |
| **App-store marketing** | `art/marketing/play_store/` | 512 square icon, 1024×500 feature graphic, authentic 1080×2400 phone screenshots from tested final app; don't fake gameplay screenshots |
| **Sound/audio (separate from PNG art)** | `pixtee-android/app/src/main/assets/audio/<name>.ogg` | New/licensed swing, contact, ground bounce, bunker, water, bird/crowd, cup, reward cues, with event-tied playback; do not reuse original audio |
| **This master brief** | `spec/pixtee/ASTRA_FINAL_ART_ASSET_BIBLE_2026-10-10.md` | Every category, source comparison, export dimension, delivery, approval and QA requirement |

**Important:** If Astra saves a new approved asset as `art/production/golfer_idle.png` at repo root, Android **will not load it**. The loader expects the **Android module asset path above**. Likewise, placing new art in category subdirectories of the Android production folder requires an explicit loader change first. Editable masters and flattened runtime PNGs are separate artifacts.

## 2. Canonical appearance, world geometry and screen sizes

- Visual benchmark: the **approved Pixtee Golf premium pixel-art montage**; rich green terrain, depth from texture and shadows, warm branded wood/gold HUD, polished original golfer and original character designs. No crude placeholder sprites or icon-font stand-ins. Title/menu can show a lively on-course simulation, never a static fake gameplay screenshot.
- Camera: **true straight-down orthographic Sensible Golf-style**, ball-following, close **partial-hole** crop. Do **not** show tee/green/full long hole all on one phone screen. Do **not** tilt, use perspective shrinking, isometric art or zoom out merely to display new trees/ads.
- Classic original course grid is **544×912 world**, while CURRENT Pixtee prototype still uses **300×510** world. The old-to-new **uniform rebasing candidate** is `912/510=1.7882352941`; old 300-width maps to 536.47 new world (3.76 units horizontal margin either side of 544). Never independently scale horizontal `544/300` and vertical `912/510`: that distorts geometry, movement and collisions.
- **Source artwork density is NOT camera zoom.** Under current unrebased candidate `CourseViewport`, 360 logical px wide / 300 world wide × zoom 1.9 = **2.28 UI logical px/world**; golfer **11.5 visible world-unit target =26.22 logical screen px**. A 720px-wide phone would draw ~52.44 physical pixels, a 1080px-wide phone ~78.66 pixels and a 1440px-wide phone ~104.88 pixels; those are candidate Pixtee visual proportions, **not measured original Sensible Golf frame dimensions**.
- **Full padded golfer canvas is 16 world units tall** and under the same camera occupies **36.48 logical screen px**. Do not confuse with the intended *visible* 11.5-world character silhouette. Transparent padding and pivot remain identical in every frame; art creation must keep the painted silhouette centered around the approved intended scale, not scale based on each pose's paint bbox.
- Every authored tree, hazard boundary, ball center, clubface and target X must retain stable world positions across 720×1600, 1080×2340, 1080×2400, 1440×3120 phones and Fold closed/open. Gameplay must be **display-resolution independent** and tick-based (~70.017 original logical ticks/s).
- Original MAPI collision sampling is **2×2 world units**, distinct from **sprite rectangle/padded alpha bounds** and actual physical collision geometry. New Pixtee collision maps, hazard polygons and any height-dependent tree response must be authored/implemented separately and tested against reference observed behaviour.
- **Tile architecture:** at 8px/world a **single full 544×912 world map would be 4352×7296 PNG pixels (~127 MB of uncompressed RGBA)**. Do **not** produce **630 giant full-hole bitmap PNGs**. Make recyclable 128×64 texture tiles / independently authored course masks, biome-themed atlases, streaming scenery chunks and data-driven hole layouts. Distinct 630 playable hole arrangements are authored in course data files.
- Sponsor boards: four small non-colliding world signs per hole, two tee and two green; current `drawSponsorBoard()` physically uses a **42×16** body and `ui_sponsor_board` draw rectangle **44×20** world, so 8px/world export = **352×160** pixels. Sign is world-space, never HUD ad; real paid creatives may only use a separately approved programmatic in-game provider and compliant attribution.
- **WHACK-O-METER:** single **HORIZONTAL** combined power-and-accuracy meter. From left, moving cursor goes **right** and second tap captures power; cursor travels back **left** and third tap captures strike accuracy/hook/slice. The pixel strips/red/yellow original zone geometry, cursor motion and lie adjustments require direct executable/graphics proof. **Astra may author neutral frame/track/pointer outlines but MUST NOT mark final meter colours or physics complete using made-up guesses**.

## 3. EXISTING Android asset IDs — all 31, exact filenames for migration

Every ID below is directly declared in current `ProductionPixelArt.kt`. All final-target PNG dimensions are *Pixtee proposed exports* and must be separately owner-reviewed; special engineering notes highlight current incompatibilities. **Production folder uses the basename exactly as printed**. All golfer images require 256×128 rig update.

| ID / family | Final export size (px) | World footprint / UI size | Implementation and evidence |
|---|---:|---|---|
| `golfer_idle.png` | 256×128 | 32×16 frame; painted height target 11.5 | [IMPLEMENTATION] upgrade 128×64 loader to 256×128, pivot 80,116 |
| `golfer_takeaway.png` | 256×128 | same pivot and footprint | full swing pose 2 |
| `golfer_backswing.png` | 256×128 | same pivot and footprint | full swing pose 3 |
| `golfer_top.png` | 256×128 | same pivot and footprint | full swing pose 4 |
| `golfer_downswing.png` | 256×128 | same pivot and footprint | full swing pose 5 |
| `golfer_impact.png` | 256×128 | same pivot and footprint | clubface at 108,116; ball at +3.5 world |
| `golfer_follow.png` | 256×128 | same pivot and footprint | full swing pose 7 |
| `golfer_finish.png` | 256×128 | same pivot and footprint | full swing pose 8 |
| `golfer_putt_ready.png` | 256×128 | same pivot and footprint | putting distinct from full swing |
| `golfer_putt_back.png` | 256×128 | same pivot and footprint | putting backswing |
| `golfer_putt_impact.png` | 256×128 | same pivot and footprint | putter contact exact |
| `golfer_putt_finish.png` | 256×128 | same pivot and footprint | putting followthrough |
| `tree_round.png` | 384×320 | 37 high (current candidate) | PNG aspect ratio implies 44.4 wide; hitbox unverified |
| `tree_pine.png` | 256×352 | 42 high (current candidate) | PNG aspect ratio implies 30.55 wide; hitbox unverified |
| `spectator_idle.png` | 96×96 | 12 high (current code) | compare apparent height to golfer; spectator size not approved |
| `spectator_wave.png` | 96×96 | 12 high (current code) | consistent feet/pivot; only limb moves |
| `spectator_photographer.png` | 96×96 | 12 high (current code) | camera separate effect |
| `camera_flash.png` | 48×48 | 5 high (current code) | decorative only |
| `bird_wings_up.png` | 64×64 | 7 high (current code) | world scene; no collision |
| `bird_wings_down.png` | 64×64 | 7 high (current code) | world scene; no collision |
| `flower_yellow.png` | 32×32 | 4 high (current code) | decorative |
| `flower_pink.png` | 32×32 | 4 high (current code) | decorative |
| `terrain_rough.png` | 128×64 | 16×8 terrain tile | requires 8 art px/world shader mapping |
| `terrain_fairway.png` | 128×64 | 16×8 terrain tile | seamless and world-anchored |
| `terrain_green.png` | 128×64 | 16×8 terrain tile | nonvisual collision source remains separate |
| `terrain_sand.png` | 128×64 | 16×8 terrain tile | bunkers retain original-style lie physics |
| `terrain_water.png` | 128×64 | 16×8 terrain tile | animated variants separate; no collision by art pixel |
| `ui_pixtee_logo.png` | 1024×512 | responsive logo | production title and splash variants later |
| `ui_wood_button.png` | 256×96 | resizable UI (nine-slice required) | existing stretch drawing must be replaced with 9-slice |
| `ui_hud_panel.png` | 432×964 | 108×241 logical HUD canvas | existing draw (2,38)-(110,279), 4x UI sampling |
| `ui_sponsor_board.png` | 352×160 | 44×20 world extent | existing RectF =44×20; no gameplay collision |

**Release gate:** every one of these 31 IDs must be present as an approved production PNG for its expected role. Existing terrain and UI sprite loader itself needs modernization before claiming these files render identically to the source game.

## 4. Additional world sprites and effects required for a finished game

Original independent Pixtee art, using **8 source pixels per world unit** as a *density target*, with fixed world-space footprints where shown. Numbers labelled **candidate** need visible reference comparison and owner approval, not invented Sensible Golf provenance.

| ID / family | Final export size (px) | World footprint / UI size | Implementation and evidence |
|---|---:|---|---|
| `ball_white.png` | 48×48 | 5 world diameter within transparent canvas | Ball visual size is Pixtee prototype, not measured original ball size; match before approval |
| `ball_roll_f00..f07.png` | 48×48 each | 5 world diameter | roll animation is visual only; tick position comes from physics |
| `ball_shadow_soft.png` | 48×24 | world-anchored under airborne ball | shadow offset keyed to physical height |
| `ball_tee.png` | 48×48 | 5 world ball + tiny tee | ball sits proud visibly |
| `ball_fairway.png` | 48×48 | 5 world ball | flat grass lie |
| `ball_semi_rough.png` | 48×48 | 5 world ball | graded grass surrounding ball |
| `ball_rough.png` | 48×48 | 5 world ball | partial grass occlusion |
| `ball_very_rough.png` | 48×48 | 5 world ball | heavier occlusion |
| `ball_sand.png` | 48×48 | 5 world ball | partially recessed into sand |
| `ball_green.png` | 48×48 | 5 world ball | green contact with small grounded shadow |
| `ball_tee_peg.png` | 32×48 | world anchor at ball foot | separate peg for tee-off |
| `ball_contact_spark_f00..f03.png` | 64×64 each | visual contact | only on actual impact tick, never artificial trajectory change |
| `ball_flight_trail_f00..f05.png` | 64×64 each | optional motion effect | disable under reduced motion |
| `ball_bounce_dust_f00..f05.png` | 96×64 each | ball-event visual | multiple actual bounces; no fixed fake count |
| `cup_hole.png` | 64×32 | 8×4 world candidate | actual cup detection from game simulation |
| `flag_pole.png` | 96×160 | 12×20 world candidate | base anchored to cup, not pixel collision |
| `flag_cloth_f00..f07.png` | 96×64 each | 12×8 world candidate | subtle repeat only when visible |
| `green_slope_arrow_f00..f03.png` | 64×64 each | 8×8 world candidate | rotate in 16 raw source-data directions, magnitude 0..15 controls display; sprite appearance unverified |
| `target_x.png` | 64×64 | on-course aiming location | render through world camera; hit area >=48dp independent |
| `aim_dots.png` | 32×32 | world-located dots | direction follows shot aim heading |
| `terrain_semi_rough.png` | 128×64 | 16×8 tile | new layer/terrain ID |
| `terrain_very_rough.png` | 128×64 | 16×8 tile | new layer/terrain ID |
| `terrain_fringe.png` | 128×64 | 16×8 tile | green/skirt visual variant |
| `terrain_tee.png` | 128×64 | 16×8 tile | tee box visual |
| `terrain_out_of_bounds.png` | 128×64 | 16×8 tile | visual style only; hazard from terrain model |
| `edge_fairway_rough_[16mask].png` | 128×64 each | 16×8 world tile | 16 tile masks/rotations; new assets; no original raster reuse |
| `edge_green_fringe_[16mask].png` | 128×64 each | 16×8 world tile | 16 transition masks |
| `edge_bunker_grass_[16mask].png` | 128×64 each | 16×8 world tile | 16 transition masks |
| `edge_water_bank_[16mask].png` | 128×64 each | 16×8 world tile | 16 transition masks |
| `water_ripple_f00..f07.png` | 128×64 each | 16×8 world tile | tile-anchored animation; no physics impact |
| `water_splash_f00..f11.png` | 128×128 each | 16×16 world candidate | spawn on actual water-contact event |
| `sand_puff_f00..f11.png` | 128×96 each | 16×12 world candidate | spawn on sand touchdown/contact |
| `rough_particles_f00..f05.png` | 64×64 each | 8×8 world candidate | only on actual rough contact |
| `grass_tuft_01..06.png` | 64×64 each | 8×8 world candidate | decorative |
| `bush_low_01..04.png` | 192×128 each | 24×16 world candidate | collider only when proven |
| `tree_birch.png` | 320×320 | 40 world height candidate | new style, tree hitbox unverified |
| `tree_palm.png` | 320×352 | 44 world height candidate | new style, tree hitbox unverified |
| `tree_flowering.png` | 384×320 | 37 world height candidate | new style |
| `tree_autumn.png` | 384×320 | 37 world height candidate | new style |
| `tree_shadow_soft.png` | 384×160 | 48×20 world candidate | decorative |
| `tree_hit_leaves_f00..f11.png` | 192×192 each | 24×24 world candidate | must be triggered by proven collision branch; source response unverified |
| `tree_hit_trunk_f00..f07.png` | 192×192 each | 24×24 world candidate | impact feedback only after real collision |
| `rock_small_01..04.png` | 96×80 each | 12×10 world candidate | decorative unless world data defines obstacle |
| `rock_large_01..04.png` | 192×128 each | 24×16 world candidate | review collision separately |
| `bridge_wood.png` | 512×192 | 64×24 world candidate | must line up with authored collision masks; requires bridge interaction rules |
| `bridge_stone.png` | 512×192 | 64×24 world candidate | world object, no arbitrary physical obstacle |
| `fence_rope.png` | 256×64 | 32×8 world candidate | decorative by default |
| `out_of_bounds_marker.png` | 96×96 | 12×12 world candidate | not authoritative collision |
| `tee_marker_blue.png` | 64×64 | 8×8 world candidate | alternate colours without physics change |
| `tee_marker_white.png` | 64×64 | 8×8 world candidate | same anchor |
| `tee_marker_red.png` | 64×64 | 8×8 world candidate | same anchor |
| `golf_bag.png` | 128×192 | 16×24 world candidate | static scenery |
| `spectator_clap_f00..f07.png` | 96×96 each | 12 world height (existing candidate) | cosmetic only |
| `spectator_cheer_f00..f07.png` | 96×96 each | 12 world height (existing candidate) | cosmetic only |
| `spectator_turn_f00..f03.png` | 96×96 each | 12 world height (existing candidate) | cosmetic only |
| `spectator_seated_f00..f03.png` | 96×96 each | 12 world height (existing candidate) | cosmetic only |
| `butterfly_f00..f05.png` | 64×64 each | 8×8 world candidate | ambient and optional |
| `cloud_shadow_soft.png` | 512×256 | 64×32 world candidate | world-tile decorative, no sun/wind physics |
| `sun_glint_f00..f05.png` | 128×64 each | 16×8 world candidate | small ambient pond glint |

**Interaction/animation contract:** water splash when the real terrain resolver emits water collision; bunker puff and ball-in-sand graphic for actual sand; tree leaves/shake **only if the original reference branch says a tree was struck** (still being recovered); successive ball bounces from actual height/velocity contacts; green arrows from slope direction/magnitude; real ball placement on tee, fairway, rough, green, bunker. Purely decorative water/flowers/crowds/birds must not modify physics ticks or RNG.

## 5. Complete player animation production plan (MORE frames than original prototype)

Final enhanced **8 facing directions** = `n,ne,e,se,s,sw,w,nw` with identical feet/pivot and clubface contact across frames. **Four-direction coverage is a minimum; 8 directions and 24-frame full-swing are the production target, not claimed counts from Sensible Golf originals.** These extension frames need a manifest-driven atlas loader; the current app only knows the 12 single pose IDs from section 3.

| Animation family | Filenames / source sequence | 256×128 frames per direction | 8-direction total | Event sync |
|---|---|---:|---:|---|
| Long-club full swing | `golfer_swing_<dir>_f00..f23.png` | 24 | 192 | Actual three-tap events; frame tagged `impact` must coincide with true launch tick |
| Putter swing | `golfer_putt_<dir>_f00..f11.png` | 12 | 96 | Ground shot, no driver-style airborne arc |
| Walk to next lie | `golfer_walk_<dir>_f00..f07.png` | 8 | 64 | Visual movement after real ball stops; doesn't move the ball |
| Idle/read stance | `golfer_idle_<dir>_f00..f03.png` | 4 | 32 | Subtle loop; foot registered and grounded |
| Address / setup | `golfer_address_<dir>_f00..f03.png` | 4 | 32 | Registered first stance |
| Watch the shot | `golfer_watch_shot_<dir>_f00..f05.png` | 6 | 48 | Per completed shot |
| Celebrate | `golfer_celebrate_<dir>_f00..f07.png` | 8 | 64 | Birdie, eagle, trophy feedback |
| Disappointed | `golfer_disappointed_<dir>_f00..f05.png` | 6 | 48 | Shot/hazard reactions |
| **TOTAL NEW SEQUENCED FRAMES** |  | **72 per direction** | **576** | Separate from 12 compatibility pose exports |

Deliver 576 **individually named** RGBA PNG frames plus packed texture atlases and manifest with deterministic frame ordering and pivot metadata. At 256×128 RGBA, 576 uncompressed frames would consume ~72 MiB just in raw frame pixels; **pack/crop transparently with pivot metadata, group atlases (prefer <=2048×2048), load animation families on demand, and budget GPU memory**. Do **not** trim/pivot recenter the actual world character or spoil clubface alignment. Enhanced intermediate frames must be visually approved, not guessed from the classic game's sprite count.

Additional costume variants: at minimum red/blue/white default and unlocked clothing/hats/trousers/skin-tone options, designed as recolour/masked parts or composited layers sharing this one skeletal registration contract. **Do not** duplicate 576 full RGBA frame sets for every outfit unless memory planning explicitly justifies it. 13 club drawings/icons must align with the independently authored player and verified gameplay club mapping.

## 6. Menus, career, HUD, sponsorship and brand — complete production UI assets

**UI art sizes below are proposed exports at 4× the 360-wide logical UI grid** (except explicitly labelled brand/store/app outputs). Adaptive layouts draw art into destination rectangles; source PNG size is not a physical screen size. All icons/buttons need untouched text-safe margins, scalable corners, normal/pressed/disabled/focus states, and high-contrast accessibility variants.

| ID / family | Final export size (px) | World footprint / UI size | Implementation and evidence |
|---|---:|---|---|
| `ui_whack_track.png` | 1152×176 | 288×44 reference logical px | Horizontal single bar; swing power-out accuracy-back; final colours BLOCKED pending exact original data |
| `ui_whack_pointer.png` | 48×112 | 12×28 logical px | pointer moves both ways; no static slider |
| `ui_whack_accuracy_overlays.png` | 1152×176 | 288×44 logical px | separate zone tint layer; red/yellow boundaries NOT confirmed |
| `ui_club_icon_00..12.png` | 192×192 each | 48×48 logical px | 13 clubs; numbers/labels from verified mapping only |
| `ui_aim_left.png` | 192×192 | 48×48 logical px | transparent hit region to left of world-space X |
| `ui_aim_right.png` | 192×192 | 48×48 logical px | slow hold opposite direction |
| `ui_minimap_frame.png` | 128×440 | 32×110 logical px | HUD compositing; visual only |
| `ui_wind_off.png` | 128×128 | 32×32 logical px | source classic default; wind optional |
| `ui_wind_on.png` | 128×128 | 32×32 logical px | only if optional wind mode accepted |
| `ui_scorecard_panel.png` | 1280×1920 | 320×480 logical px | scroll/reflow layout; typography via UI not baked text |
| `ui_tournament_bracket_panel.png` | 1280×1920 | 320×480 logical px | adaptive panel artwork |
| `ui_options_panel.png` | 1280×1920 | 320×480 logical px | buttons accessible |
| `ui_career_map_frame.png` | 1280×1920 | 320×480 logical px | career overlay; no full phone screenshot baked |
| `ui_stats_panel.png` | 1280×1920 | 320×480 logical px | data-driven widgets |
| `ui_reward_panel.png` | 1280×1920 | 320×480 logical px | celebration overlay |
| `ui_pause_panel.png` | 960×1280 | 240×320 logical px | keyboard/accessibility parity |
| `ui_button_primary_9slice.png` | 256×96 | 64×24 logical px base | nine-slice spec; preserve corner pixels |
| `ui_button_secondary_9slice.png` | 256×96 | 64×24 logical px base | nine-slice spec |
| `ui_button_pressed_9slice.png` | 256×96 | 64×24 logical px base | nine-slice spec |
| `ui_toggle_on.png` | 256×128 | 64×32 logical px | settings toggle |
| `ui_toggle_off.png` | 256×128 | 64×32 logical px | settings toggle |
| `ui_icon_settings.png` | 192×192 | 48×48 logical px | navigation |
| `ui_icon_pause.png` | 192×192 | 48×48 logical px | navigation |
| `ui_icon_stats.png` | 192×192 | 48×48 logical px | navigation |
| `ui_icon_trophies.png` | 192×192 | 48×48 logical px | navigation |
| `ui_icon_career.png` | 192×192 | 48×48 logical px | navigation |
| `ui_icon_courses.png` | 192×192 | 48×48 logical px | navigation |
| `ui_icon_play.png` | 192×192 | 48×48 logical px | navigation |
| `ui_profile_frame.png` | 512×512 | 128×128 logical px | rewards cosmetics only |
| `ui_level_ring.png` | 512×512 | 128×128 logical px | no physics bonuses |
| `ui_progress_bar_track.png` | 1024×96 | 256×24 logical px | rewards/progression |
| `ui_progress_bar_fill.png` | 1024×96 | 256×24 logical px | rewards/progression |
| `ui_medal_bronze.png` | 512×512 | 128×128 logical px | career ranks |
| `ui_medal_silver.png` | 512×512 | 128×128 logical px | career ranks |
| `ui_medal_gold.png` | 512×512 | 128×128 logical px | career ranks |
| `ui_medal_platinum.png` | 512×512 | 128×128 logical px | career ranks |
| `ui_trophy_cup_small.png` | 512×512 | 128×128 logical px | badge/icon |
| `ui_trophy_cup_large.png` | 1024×1024 | 256×256 logical px | result animation |
| `ui_achievement_icon_[key].png` | 320×320 each | 80×80 logical px | at least all specified achievement IDs; each unique |
| `ui_tour_emblem_amateur.png` | 512×512 | 128×128 logical px | 5 career tiers |
| `ui_tour_emblem_regional.png` | 512×512 | 128×128 logical px | 5 career tiers |
| `ui_tour_emblem_national.png` | 512×512 | 128×128 logical px | 5 career tiers |
| `ui_tour_emblem_pro.png` | 512×512 | 128×128 logical px | 5 career tiers |
| `ui_tour_emblem_world.png` | 512×512 | 128×128 logical px | 5 career tiers |
| `reward_fireworks_f00..f15.png` | 1024×1024 each | 256×256 logical px | end frame held; reduced-motion option |
| `reward_confetti_f00..f15.png` | 1024×1024 each | 256×256 logical px | end frame held; reduced-motion option |
| `reward_trophy_shine_f00..f11.png` | 512×512 each | 128×128 logical px | finish marker visible |
| `course_select_thumb_c01..c35.png` | 1024×768 each | responsive thumbnail | 35 original course themes; 35 covers, not original layouts |
| `course_select_icon_c01..c35.png` | 256×256 each | responsive tile/icon | 35 selectable course emblems |
| `app_icon_master.png` | 1024×1024 | vector master+transparent PNG | also 512×512 Play Store export; Android adaptive layers separate |
| `app_adaptive_foreground.png` | 432×432 | Android launcher layer | res/mipmap-anydpi-v26 + adaptive layers |
| `app_adaptive_background.png` | 432×432 | Android launcher background | avoid logo crop |
| `app_splash_brandmark.png` | 1024×1024 | Android splash drawable | do not delay gameplay |
| `app_store_feature_graphic.png` | 1024×500 | Play Store promotional image | not in-game asset |
| `app_store_screenshot_1080x2400.png` | 1080×2400 | normal portrait phone | must be captured final playable build, not fantasy mockup |

Additional mandatory UI *states* (not a single flattened screenshot): settings with Course Boards On/Off, music/sound, accessibility/reduce-motion, controls onboarding for 3-tap swing and target X, resume/pause confirmations, scorecard for 3/9/18, match play and stroke play brackets, career tier/reward unlock, wardrobe equipment picker, challenges, stats graphs with dynamic live data, achievement locked/unlocked/earned versions, offline save/resume, empty/error/loading states, and one complete screen family for every menu mode already in Pixtee source. **Do not bake dynamic text or scores into bitmaps**. Make reusable 9-slice panels, borders, buttons, scalable icon families and tokenized typography with licensed in-app font assets.

Sound (separate non-image handover): golfer swing whoosh/clubface strike, tee ping, lie contact rough/sand, bounce count, water splash, tree impact, putt roll, holed cup, crowd applause, results/trophies, menu transitions, gentle ambient birds/water and music themes; licensed original compositions only. Timing attached to real engine events and mute/reduce-motion controls.

## 7. The entire course-art workload — 35 courses × 18 authored holes

Pixtee's planned total is **35 distinct original courses / 630 distinct holes**. Historical original archive's **72** source hole-resource numbers are reference facts, **not** a Pixtee course count or license to copy original terrain layouts. Do not generate 630 screenshots or 630 source-game-map clones.

**For EACH of course `c01...c35` produce:**

1. `art/source/courses/cNN/theme_palette.json` (one original theme palette), `course_theme.aseprite`, `course_cover.psd/png`, `course_emblem.svg/png`, and reusable 128×64 world-tile family with grass tones, 16-mask edges, rough, fringe, green, bunker/water transitions, 3+ vegetation variants, fixed world space cast shadows and occasional environmental scenery. Each course must have distinct aesthetic and palette—not just a global hue filter.
2. Runtime `assets/courses/cNN/h01.json` ... `h18.json` containing original hole geometry and **draw layers and collision layers separated**: par, world size `544×912` (or rigorously validated compatible geometry), 4-position tee starts if wanted, cup coordinates, boundary/water polygons, detailed greens/arrow directions, 2-world-unit terrain classification grid or compatible exact resolver, trees with **separately classified** obstacle hitboxes/height only after reference evidence, authored decorative scenery, four reserved non-colliding board slots, camera follow and minimap scale.
3. 18 distinct hole compositions with tee/cup hierarchy, fairway/bunker/water locations, clean putting approach, no impossible collision routes, intentional difficulty, and evidence that none substantially duplicates an original Sensible Golf hole. Keep aspect/world scale from original-world geometry contract.
4. A hole validation bundle per course: tee+approach+bunker+water+rough+green+putting screenshots from *the actual engine at normal phone scale* and playable automated trajectory tests. Check camera never shows whole 400-yard hole from tee, no decor influences physics, water/sand edges visually align with collision polygons.

**Do not put original GOLF.EPF extracted course grids or MAPI banks in Pixtee assets.** Direct numeric measurement governs scale and testing, not copying the original artistic hole topology. Build assets with a signed legal/IP review before any literal reuse of original code, raw tables or media.

## 8. Colour, registration, layering, accessibility, file quality

- A Pixtee-specific shared palette set: lush greens, nuanced sandy creams, blue water, darker navy HUD, warmer gold and natural wooden buttons, approved concept's friendly characters and polished 16-bit premium texture. Original source Welly pixel red/yellow boundaries **PENDING genuine binary/palette recovery**, so no invented hex values for it.
- Source art: 32-bit transparent RGBA PNG, sRGB, lossless, no lossy JPG for gameplay, no accidental resampling blur on final crisp pixel exports. Editable layered originals + palettes/timeline retained. Enforce exact image dimensions with script/CI.
- All animated frames use identical transparent canvas registration; lower-body foot anchor, physical ball lie coordinate and clubface contact never drift more than target **0.5 output art pixel at fixed frame world pose**. This is a Pixtee QA tolerance proposal, not a measured classic tolerance.
- Hard requirement: image/export dimensions **are visual only**; collision shapes are separate gameplay data. A 384×320 tree image does **NOT** mean a 48×40-world collision box; real tree interactions and hit tests require further original executable traces. Golfers, spectators and 4 in-world ad signs are not automatic obstacles just because sprites overlap.
- Avoid z-order changes and shadow mismatch: terrain base > rough/fairway masks > green fringe > water/sand > world-scenery shadows > grass/rocks/signs > on-ground ball/shadow > golfer/tree sprites > airborne ball > UI HUD/WHACK-o-meter; sort world objects by footpoint if appropriate, **never introduce 3D perspective**.
- 48dp Android minimum effective interactive hit regions; UI buttons and target-X steering areas can have larger invisible touch hitboxes than their painted PNG outlines. The physical DPI/window insets, not PNG pixel size, determines touch usability.
- Avoid full-screen screenshots baked as gameplay/menu backgrounds (the main menu should show live AI golf); avoid repeating the same 3 trees and same spectators endlessly. No static fake gameplay placeholders.

## 9. Astra handoff and production execution order

**Astra, implement these in the repository in this order, saving each file and testing it before moving on.** Do not change the already approved camera concept, typography words or interactions except targeted corrections needed for asset formats and tested normal-phone behavior.

**Stage 0 — prove asset pipeline and size arithmetic.** Update render support for 256×128 golfer frames, pivots (80,116)/(108,116), 8px/world tiles, UI4×, sprite atlas metadata, and 9-slice controls **before exporting 576 gameplay frames**. Keep current 128×64 legacy variant as explicitly temporary debug compatibility until 256×128 replacements work. Verify 3.5-world ball/contact distance, 16-world padded height, 11.5-world candidate painted figure and correct crop on phone. No changes to game physics or camera angle. Original collisions, meter colours/timing, object hitboxes and source renderer placement remain separate evidence gates.

**Stage 1 — golfer and ball.** Author/redline 12 engine IDs, 8 directions, all 576 final frames, correct lie-ball/ball shadow/tee/cup/flag and impact. Attach a real 1080×2400 Android captured preview and enlarged sprite sheet for every approved batch; no unseen-source-sheet 'approved' claims.

**Stage 2 — course tiles and scene.** Export seamless biome tiles, 16-mask transitions and water/bunker sprites at exact sizes, tree/hazard visual reactions only when gameplay events trigger; test surfaces align original-style fine collision grid and course painter on normal phone and Fold.

**Stage 3 — UI and horizontal Welly.** Build all menu/HUD/button/target/minimap textures and art; retrieve original meter color/tick samples before final zones; **do not invent another power-only bar**. Test user input on actual game at phone and Fold ratios.

**Stage 4 — all 35 themed courses and 630 holes.** Unique palettes/content, built from original Pixtee art, automated seeded validation/replay, cosmetic sponsorship boards at tee+green; ensure no hole has blank assets or gameplay placeholder.

**Stage 5 — career, rewards, stats, wardrobe, sounds, marketing.** All screen states, 5 tours, progression badges, matching animated icon sizes, app icon/splash, achievements and screenshot deliverables.

**Stage 6 — release-level proof.** Run GitHub build, manifest validator, native emulator gameplay, normal portrait Android **720×1600/1080×2340/1080×2400/1440×3120**, Fold closed/open, reduced-motion/high contrast, low-memory, repeated round and screenshot A/B tests. Only then request owner approval per asset pack and populate `ART_REVIEW_REGISTER.json`.

## 10. Acceptance gate — must pass before calling final art DONE

| Gate | Pass condition |
|---|---|
| **Exact file dimensions** | Every PNG uses its table'd *final-target* dimension or a separately documented owner-approved revision. Source masters, runtime flat filenames and atlas manifests agree byte-for-byte (SHA-256) |
| **No wrong golfer scale** | 256×128 padded pixels, feet (80,116), club (108,116), same **32×16** world-frame as old 128×64; 11.5-world candidate visible silhouette subject to screenshot signoff; all directions register contact |
| **Source fidelity** | Actual Sensible Golf file-derived reference maintained; all 72 original-course resource measurements, 874 source sprite dimensions and fine 2×2 collision reference; unverified renderer sizes/hitboxes not mislabelled exact |
| **Collision/physics invariance** | Identical game trajectories before/after switching bitmap resolutions and Android phone pixel count; any tree/water/sand visual changes leave original-style physics/collision untouched |
| **Camera** | Strict top-down ball-follow partial-hole crop on standard portrait phone, no full-hole on-screen shrinking, no view angle change |
| **Meter** | One horizontal meter with outgoing-power/return-accuracy timing; authentic red/yellow colour mapping pending measured reference; never a separate power bar |
| **Coverage** | No missing sprite poses/lie looks, environments/terrain edges, UI states, 35 unique course art packs or 630 authored playable holes; 4 board slots per hole |
| **Delivery** | Everything in correct GitHub paths; editable masters, Android production PNGs, matching manifest, approval hashes, preview packs, build/test evidence |
| **Owner** | Owner approves each defined pack and the canonical gameplay montage equivalence; no approval auto-generated |

**Stop rather than guess:** If any original-source sprite role, hitbox, tile render scale or meter colour/physics is not verified, label it `PENDING_REFERENCE`, capture original program evidence, and keep the asset in review. Final product means measured behavior and owner-approved consistent art, never an untested set of generated images.


---

## 11. CANONICAL EXTENSION — ALL COURSE AMBIENCE, SPECIES AND IMPACT ANIMATIONS (OWNER DIRECTIVE)

**Status:** mandatory production coverage, export sizes proposed at 8 authored pixels per world unit. This extension **supersedes the minimal generic two-frame bird, static flowers/grass and single-person crowd concepts** in earlier rows, but keeps their legacy IDs as first-build compatibility aliases. It also makes the previously understated ball-to-tree, ball-to-water and ball-to-bunker feedback explicit. Older rows are NOT the complete animation inventory.

**Game fidelity boundary:** Sensible Golf's original file-derived physics/collision logic remains the gameplay reference. Cosmetic new wildlife, particles, animated flowers, waving spectators, signs and reactions **must not advance/consume the physics RNG** or mutate the ball's acceleration, velocity, spin, collision codes, scores, flight timing or direction. Optional rare bird-hit jokes are an explicitly **new Pixtee feature**, NOT falsely described as original Sensible Golf behaviour. The original game's effect frame dimensions/timings still need renderer/disassembly verification. Every original-game gameplay animation category discovered later MUST be added to the register and measured before parity is claimed.

### 11.1 Bird species matched to location: complete required wildlife artwork

The current code's `PixteeCourse.theme = courseIndex % 10` is only a **palette rotation**, not reliable ecology. **Astra/engineering MUST use an explicit `wildlifeBiome` and weighted `speciesSpawnSet` per course** (recorded in `assets/courses/cNN/theme_palette.json` and authored course metadata). Bird types, calling sounds, visual sizes, feather colours and flight style must be appropriate to the course environment. Do not render tropical parrots on frozen Scottish-style greens or seabirds in inland deserts.

| Environment | Required original Pixtee bird species | Behaviour and effect personality |
|---|---|---|
| PARKLAND | robin, blackbird, sparrow | small chirping fly-through; robin can hop and dart |
| WOODLAND | blue-tit, woodpecker, crow | agile flutter; woodpecker near trees, crow glides |
| HEATH / MOOR | skylark, kestrel | high quick arc / occasional graceful hover |
| LINKS / COASTAL | gull, tern, oystercatcher | gull lazy circling, tern quick darting over water |
| LAKE / MARSH | mallard, heron, kingfisher | duck flap, slow heron glide, fast kingfisher along stream |
| DESERT | falcon, dove | rare high soaring falcon; soft dove wingbeat |
| HIGHLAND / CLIFF | eagle, raven | slow wide glide, soaring thermals |
| TROPICAL / LAGOON | parrot, kingfisher | colour accents and energetic short flyovers |
| AUTUMN / VALLEY | magpie, crow | dashes between tree canopies |
| FROST / GLACIER | ptarmigan, raven | rare bird movement and mostly long gliding shots |

**Coverage rule:** all **21 distinct species** named across that table require unique silhouettes/colour palettes and impact/recovery frames; shared bird **skeleton templates may be reused** but no single gull skin covers every biome. Birds are sometimes alone and occasionally two together if appropriate; never a guaranteed flyover every shot. Season/weather variants are variants, not a new game-physics mechanic. A curated per-course `wildlifeBiome` mapping MUST cover all 35 course IDs, including Coastline/Pebble Cove/Seabright (COASTAL), Willow Marsh (MARSH), Desert Bloom/High Mesa (DESERT), Frostwood/Glacier Point/Northlight (FROST), Blue Lagoon/Coral Key/Lotus Springs (TROPICAL/LAGOON), Lakewood/Summit Lakes (LAKE), Pine Crest/Redwood Park (WOODLAND), and all others. Defaults chosen by explicit authored ecology, not numeric `theme` index.

**Per-species exact export convention and filenames:**

| Bird asset family (replace `<species>`) | Exact PNG canvas per frame | Frames / species | World footprint target | Trigger |
|---|---|---:|---|---|
| `bird_<species>_fly_f00..f07.png` | 64×64 (small), 96×96 (medium), or 128×96 (large) | 8 | 8×8, 12×12, or 16×12 world | world-space flyover, not fixed screen overlay |
| `bird_<species>_hit_f00..f09.png` | same size as species' flight canvas | 10 | **exact same visual/world anchor** | rare ball-to-bird event |
| `bird_<species>_recover_f00..f07.png` | same as species' flight canvas | 8 | same footprint | comical mid-air wobble, then safe fly-away |
| `bird_feathers_f00..f09.png` | 128×128 | 10 shared | 16×16 world decorative effect | feather puff with 2-3 species-tinted particles |
| `bird_confused_star_f00..f05.png` | 64×64 | 6 shared | 8×8 world decorative effect | whimsical star orbit, optional |

Species size classes: **small** robin/blackbird/sparrow/blue-tit/woodpecker/skylark/tern/oystercatcher/kingfisher/dove/magpie/ptarmigan; **medium** crow/gull/mallard/kestrel/parrot/raven; **large** heron/falcon/eagle. Some are a zoologically simplified stylized scale; review final visible height on a normal portrait phone. Where a species is larger in nature, treat its size class above as a readability **proposal**, not an assertion about the original game. Supply transparent registered canvases and distinct light/dark silhouette values. The species set lists **21 unique species**; generate **21 complete animation families**. That is **21×26 = 546 authored frames**, plus 16 shared hit particles. Flight direction can be flipped only when asymmetry/lighting passes review; otherwise author left/right facings and note frame multiplication in `assets.manifest.json`.

**Humorous bird strike — Pixtee-exclusive Easter egg:** if (and only if) a real existing ball-flight position overlaps the actual *world-space* bird interaction volume including height, trigger the bird's 10-frame flip/spin with a brief safe scatter of feathers, surprised blink/chirp, then 8-frame recovery/flyaway. It is a lighthearted cartoon near-miss/bonk, never gory, never a dead bird. The ball **continues on its verified original-game physics path unchanged**. Use a purely visual trigger filtered out of authoritative physics collision/cup/hazard handling and use a separately seeded *cosmetic* RNG so identical three-tap shot produces identical ball history with birds on/off. Debounce the encounter once per bird and shot. Do not randomly force a bird under the ball or count a normal 2D screen overlap (its flight-height must intersect). Add accessibility/reduce-effects option for feathers/flash and limit frequency, including a per-round cooldown. **The current `CourseAmbient.birdFlyover` uses screen-ish coordinates (-22..322) despite being drawn under world transform: refactor bird placement to actual camera-independent course world coordinates before declaring finished.**

### 11.2 Birds, flowers and foliage never remain static

| Family | Exact PNG export | Frames / variant | Loop rule and limits |
|---|---|---:|---|
| `grass_tuft_small_01..03_f00..f05.png` | 48×48 | 6 × 3 | tiny sway ±1–2 authored pixels; asynchronous, never uniform |
| `grass_tuft_large_01..03_f00..f05.png` | 64×64 | 6 × 3 | gentle tip bend; anchored base, no tile drift |
| `flower_yellow_patch_01..02_f00..f05.png` | 64×64 | 6 × 2 | slight sway, unchanged root |
| `flower_pink_patch_01..02_f00..f05.png` | 64×64 | 6 × 2 | slight sway, unchanged root |
| `flower_white_patch_01..02_f00..f05.png` | 64×64 | 6 × 2 | 6-frame gentle loop |
| `flower_blue_patch_01..02_f00..f05.png` | 64×64 | 6 × 2 | 6-frame gentle loop |
| `flower_red_patch_01..02_f00..f05.png` | 64×64 | 6 × 2 | 6-frame gentle loop |
| `reeds_marsh_01..03_f00..f07.png` | 96×96 | 8 × 3 | reed sway tied to biome, no collision |
| `bush_low_01..04_sway_f00..f03.png` | 192×128 | 4 × 4 | nearly imperceptible leaf motion; not ball physics |
| `tree_round_sway_f00..f03.png` | 384×320 | 4 | anchored trunk / rustling crown |
| `tree_pine_sway_f00..f03.png` | 256×352 | 4 | anchored trunk |
| `water_ripple_f00..f07.png` | 128×64 | 8 | separate from splash; tile anchored |
| `flag_cloth_f00..f07.png` | 96×64 | 8 | weather visuals; wind-off original fidelity mode remains non-force |

These include **purely cosmetic animation** at deterministic frame sampling; flora and sky movement do not advance gameplay time, RNG or change surface classification. Static originals (`flower_yellow.png` etc) remain compatible until animated API shipped. All optional motion respects reduced motion.

### 11.3 People — complete crowds, individual poses and photographer WITH FLASH

Every person remains legible at the intended tiny top-down camera scale, with natural skin, clothing, body shapes and diverse ages. A 96×96 RGBA source canvas corresponds to a **12×12 world canvas** at 8 pixels/world; painted silhouette must remain close to the current candidate **12-world-unit vertical height**. Never paste full-scale street photographs into pixel scene.

| Family | Export canvas | Count and frames | Event |
|---|---|---|---|
| `spectator_idle_male_01..03_f00..f03.png` | 96×96 | 3 variants ×4 | animated breathing/shift |
| `spectator_idle_female_01..03_f00..f03.png` | 96×96 | 3 ×4 | animated breathing |
| `spectator_idle_child_01..02_f00..f03.png` | 96×96 | 2 ×4 | subtle movement |
| `spectator_clap_01..04_f00..f07.png` | 96×96 | 4 ×8 | good shot / holed |
| `spectator_cheer_01..04_f00..f07.png` | 96×96 | 4 ×8 | birdie/eagle/ace, event-driven |
| `spectator_wave_01..04_f00..f07.png` | 96×96 | 4 ×8 | occasional, never simultaneous wall |
| `spectator_turn_head_01..04_f00..f03.png` | 96×96 | 4 ×4 | ball/crowd attention |
| `spectator_seated_01..04_f00..f03.png` | 96×96 | 4 ×4 | seated by spectators’ area |
| `spectator_photographer_idle_f00..f03.png` | 96×96 | 4 | camera lowered / readiness |
| `spectator_photographer_raise_f00..f05.png` | 96×96 | 6 | raises camera to eye |
| `spectator_photographer_shoot_f00..f03.png` | 96×96 | 4 | shutter animation |
| `spectator_photographer_lower_f00..f05.png` | 96×96 | 6 | return to idle |
| `camera_flash_f00..f03.png` | 48×48 | 4 | bright 1–2-frame burst + fading flash; honour reduced flash |
| `crowd_cluster_small_01..04.png` | 160×128 | 4 | 2–3 people reusable group |
| `crowd_cluster_medium_01..04.png` | 224×160 | 4 | 4–5 people |
| `crowd_cluster_large_01..04.png` | 320×192 | 4 | 6–8 people |

Crowd clusters are **render composites of still-animated individuals** where applicable; final implementation must either animate each person through slots or export small group sequences. Draw with depth order keyed to the footpoint in a true straight-down course. Photographer's flash must be tied to the actual shutter pose, not an independent bright spot; current `CourseAmbient.photographerFlash()` hard-codes a 120ms flash every ~18 sec, which is **current prototype timing, not original Sensible Golf evidence**. No camera flashes during user interface transitions or reduced-flash settings.

### 11.4 Bunker, water, trees, rough — all impact reactions are MANDATORY

**The following list is a minimum final-game set, not a statement these frame counts are extracted from original files.** The historical source's collision/terrain transition logic must decide *when* an effect plays. Astra authors original high-detail Pixtee animation frames; gameplay code emits distinct `BallImpactEvent(kind, worldX, worldY, tick, shotId, contactIndex, velocity, lie)` snapshots for the effect renderer without allowing the renderer to alter physics.

| Physics contact / event | Required asset filenames (one transparent PNG per frame) | Exact final export canvas | Frames | Source/reference and behavioural requirement |
|---|---|---|---:|---|
| First ball hits **bunker sand** | `fx_bunker_ball_impact_f00..f11.png` | 128×96 | 12 | Only emitted by confirmed SAND collision; sand puffs out and settles; anchored at true impact |
| Sand landing bounce on subsequent contact | `fx_bunker_ball_bounce_f00..f07.png` | 96×80 | 8 | Real second/subsequent impacts, never fake bounce count |
| Sand lie when resting | `ball_sand.png`, `fx_sand_settle_f00..f05.png` | 48×48; 96×64 | 1 + 6 | Ball partially seated in sand as classified by collision resolver |
| Bunker shot leaving sand | `fx_bunker_explosion_f00..f11.png` | 160×128 | 12 | When the club launches FROM sand, sand cloud behind the true outgoing ball, never a different launch vector |
| Ball touches **water** | `fx_water_entry_f00..f13.png` | 128×128 | 14 | Impact/splash at exact water collision point; wait for hazard state and original 35 stop case |
| Water ring / settling | `fx_water_ripple_ring_f00..f11.png` | 128×128 | 12 | Expanding fading ring, no fake ball ground bounce |
| Water ball disappearance | `fx_water_sink_f00..f07.png` | 64×64 | 8 | Only when original hazard state removes/recovers ball; never leave white ball floating |
| **Tree trunk** contact | `fx_tree_trunk_hit_f00..f07.png` | 192×192 | 8 | Only after actual object collision; trunk thump/chips |
| **Tree canopy / branch** contact | `fx_tree_canopy_hit_f00..f11.png` | 192×192 | 12 | Layered branch shake with leaf scatter |
| Tree leaves scattering | `fx_tree_leaf_scatter_f00..f11.png` | 192×192 | 12 | Different leaf colour variants by tree type/season |
| Branch motion / recover | `fx_tree_branch_wobble_f00..f07.png` | 256×192 | 8 | Anchor trunk untouched, crown oscillates and settles |
| **Rough / grass** impact | `fx_rough_grass_impact_f00..f05.png` | 64×64 | 6 | Grass blades fly only on terrain-contact event |
| **Fairway** contact | `fx_fairway_turf_tap_f00..f03.png` | 64×48 | 4 | Tiny near-invisible turf response, real ball bounce |
| Ball hit solid obstacle / wood fence | `fx_obstacle_impact_f00..f07.png` | 128×128 | 8 | Only after proven object collider category; no invented rebound |
| Repeated airborne touchdown/bounce dust | `ball_bounce_dust_f00..f05.png` | 96×64 | 6 | Replay as many contacts as physics records |
| **Hole/cup** capture | `fx_cup_capture_f00..f11.png` | 96×96 | 12 | Cup moment is actual original-style terminal result, no visual-only 'hole in' cheat |
| Green slope indication | `green_slope_arrow_f00..f03.png` | 64×64 | 4 | Direction/magnitude from original-like terrain slope descriptors; not generic static arrows |

**Distinct impact outcomes are mandatory**: bunker ball landing, sand-lie ball, sand shot cloud, water strike/splash/ripple/sink, tree trunk vs canopy hit + leaves + wobble, fairway/rough contact, every genuine bounce, hole capture. Record effect event IDs in replay so frame and sound trigger reproducibly; animation FPS must not modify original shot state. If original game has additional effects or special branches, add them as new test-backed rows; no statement of “all original animations recovered” until the full renderer/event trace is inventoried.

**No missing-effects fallback**: if an effect animation is not approved/exported, do not quietly display a programmer-art puff and pretend release parity. Production gate fails and QA lists its exact missing asset/family. Placeholder geometry is permitted only in separately labelled engine diagnostic tests, not shipping visual reviews.

### 11.5 Advertising boards: must face tee and hole, NOT randomly face the camera or away

**Four fixed non-colliding signs per hole:** `tee-a`, `tee-b`, `green-a`, `green-b`. They must be **positioned near the tee-off zone and green, outside shot corridor, and their front faces must face toward the associated playable area**, so they are actually visible as the golfer prepares to hit/putt.

| Board variant | Proposed PNG export | Role/face target | Required viewpoint |
|---|---:|---|---|
| `board_tee_left_front.png` | 352×160 | tee-a: face the teeing ball/address point | readable true-overhead front at tee |
| `board_tee_right_front.png` | 352×160 | tee-b: face teeing ball/address point | same, not mirrored typography |
| `board_green_left_front.png` | 352×160 | green-a: face playable green / cup and common approach direction | readable at ball-follow green view |
| `board_green_right_front.png` | 352×160 | green-b: face playable green / cup and common approach direction | same |
| `board_wood_back.png` | 352×160 | alternate rear face when seen from behind | no accidental ad impression |
| `board_post_shadow.png` | 352×80 | non-colliding world-space shadow | decorative |
| `board_house_pixtee.png` | 352×160 | offline/unfilled house sign fallback | **not** paid advertisement |
| `board_slot_mask.png` | 320×112 | blank area for provider-approved creative/labels | never print baked advertiser logos |
| `board_corner_highlight.png` | 64×64 | world-scene polish | no physics |
 
**Geometry contract:** Each sign has `position=(x,y)` and `frontNormal = normalize(targetPoint - position)` in the world XY plane. For tee signs `targetPoint=tee`; for green signs `targetPoint=cup/green approach focus`. This is **an authored 2D plan-view orientation cue**, not a tilted 3D billboard; ensure screen-space projection is legible in the straight-down camera, with proper depth/front/back variants if visible from the reverse side. Place front-face artwork so the normal play camera sees the branding and mandatory SDK attribution without sideways/mirrored copy. Assess visibility/cutout/minimap/shot corridor at **1080×2400 normal portrait phone** first, then compact phone and Fold open/closed. Never alter camera zoom to increase ad visibility. Retain original 42×16 body world units and 44×20 outer artwork bounds (352×160 art @8px/world) until source gameplay scale acceptance; no hitbox at all.

**Advertising product contract:** provider-approved programmatic in-game ad system only; merely drawing Pixtee placeholder logos on signs does not earn revenue. Advertiser creative is filled and measured by licensed network renderer; any contrast/attribution requirements override decorative design only after review. **No billboard-as-HUD bars and no collision with ball.**

### 11.6 Expanded golfer direction/pose completeness and original animation coverage

The existing brief's 8 direction set (`n ne e se s sw w nw`) is MANDATORY in every relevant golfer action and frames share identical 256×128 canvas, pivot **(80,116)**, impact-contact **(108,116)** for its reference facing with facing-specific, explicitly registered contact anchors for all rotated facings (do NOT falsely reuse the same contact point for left/back side shots). Proposed full enhancement sequences:

| Required asset family | Frames/direction | 8-direction count | Export |
|---|---:|---:|---:|
| `golfer_address_<dir>_f00..f03.png` | 4 | 32 | 256×128 |
| `golfer_swing_<dir>_f00..f23.png` | 24 | 192 | 256×128 |
| `golfer_putt_<dir>_f00..f11.png` | 12 | 96 | 256×128 |
| `golfer_walk_<dir>_f00..f07.png` | 8 | 64 | 256×128 |
| `golfer_idle_<dir>_f00..f03.png` | 4 | 32 | 256×128 |
| `golfer_watch_shot_<dir>_f00..f05.png` | 6 | 48 | 256×128 |
| `golfer_celebrate_<dir>_f00..f07.png` | 8 | 64 | 256×128 |
| `golfer_disappointed_<dir>_f00..f05.png` | 6 | 48 | 256×128 |
| **Total** | **72** | **576 complete animation frames** | Not the old 384-frame minimum |

The earlier 384 count is **superseded by 576** because address and shot reaction/emotion animations are now mandatory. The 12 current engine pose IDs remain compatibility mapping aliases; updated 8-direction/576-frame atlas manifest and animation state machine must be implemented. Swing impact, bunker shot, water hit, tree hit, putt, hole completion, idle, walk and celebrations are all **event-driven**. One pixel-art template or flipped frame set only qualifies when facing silhouette/handedness, foot anchor and clubface genuinely align. The original Sensible Golf animation frame-by-frame timing is not fully extracted; **no made-up 'exact original frame count' claims allowed**.

### 11.7 Specific directories and mandatory verification

- Editables for all new ambience and effects: `art/source/world/ambient/`, `art/source/world/wildlife/`, `art/source/world/spectators/`, `art/source/world/foliage/`, `art/source/world/impacts/`, `art/source/world/boards/`, plus `art/source/golfer/8dir/`. Group by course/biome as needed.
- Review: `art/previews/<batch>/`; debug runtime flat-ID PNGs: `pixtee-android/app/src/main/assets/art/review/<asset_id>.png`; **approved** runtime flat-ID PNGs: `pixtee-android/app/src/main/assets/art/production/<asset_id>.png`; expandable atlases can use `assets/art/production/atlases/` **only after implementing a manifest-aware loader**. New IDs are NOT accepted by current `ProductionPixelArt.REQUIRED_SPRITES` allowlist; Astra must update code/loader and demonstrate visible sprites (otherwise files are invisible).
- Machine-readable per-animation export specs in **[ASTRA_AMBIENCE_AND_IMPACT_MANIFEST_v2.json](ASTRA_AMBIENCE_AND_IMPACT_MANIFEST_v2.json)**. Current core [ASTRA_ART_EXPORT_MANIFEST_v1.json](ASTRA_ART_EXPORT_MANIFEST_v1.json) covers the original 31 IDs **only**; do not conflate that with complete production coverage.
- QA for every animated set: exact per-frame export pixel canvas, frame counts, world footprints, named event trigger and animation clock, transparent padding, stable pivot, screen-captured animation/impact on normal portrait phone, graceful Fold layout, no occlusion of the ball/WHACK-O-METER, correct ambience for **each course**, visible board fronts facing tee/green, no gameplay physics deltas with cosmetic layers on/off.
- **All 35 original authored courses × 18 holes (630 unique holes)** need selected biome, bird species, foliage, crowds, four sign slots, impact event visibility, tree type and authentic slopes. Do not count 630 random seeded course arrangements as fully artist-authored/polished layouts. The classic source physics/extracted dimensions remain frozen independent of this new art.
