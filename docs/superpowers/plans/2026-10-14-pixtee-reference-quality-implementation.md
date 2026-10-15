# Pixtee Reference-Quality Implementation Plan

> For implementing agents: use `superpowers:executing-plans` if available. Work through the numbered tasks in order, one bounded change at a time. Do not delegate or reread the complete repository routinely. This document supplies the design; implementation is the next model's job.

**Goal:** Make the existing one-hole game look and move like the owner's three reference pictures, with readable mobile controls, a living course and real native wind affecting airborne balls.

**Architecture:** Preserve the recovered C++ gameplay engine and existing Godot 4.5 presentation. Improve the existing renderers, introduce a small shared presentation state, and reuse the approved art and Living World package. Add wind as an explicitly authored C++ extension to the existing shot path, with a zero-wind fidelity path; never create another ball simulation.

**Tech stack:** Godot 4.5, GL Compatibility renderer, GDScript presentation, C++ GDExtension, fixed native tick 936/65536 seconds. Build and review Windows first. No APK until the owner accepts readiness.

**Specification:** This document plus the three owner images in the accompanying `references/` folder. Earlier factual checkpoint: `spec/pixtee/PIXTEE_ASTRA_FINAL_POLISH_HANDOFF_2026-10-14.md`. The later explicit request for ball-affecting wind supersedes older instructions that normal Pixtee play must have no wind. Keep the zero-wind fidelity/test mode.

## 0. Read this before changing anything

### Working baseline

- Repository: `https://github.com/russellstokes-ai/Sensi-golf`.
- Last verified implementation: `cb19300d7cc7abc113b1323e571c11d36e1824a0`.
- Feature branch: `feat/pixtee-living-world-polish-2026-10-14`; draft PR16.
- Restoration branch: `restore/pixtee-pre-astra-polish-2026-10-14`.
- Passed native/Godot/render run: `https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38593208863`.
- Local checkout is under `work/Sensi-golf`. Its Git HEAD is stale `f0e3f258`; its working files contain newer remote code. **Do not reset it, discard its changes, or bulk-push its old history.** Read `work/REMOTE_CHECKPOINT.json`, fetch the actual remote head and preserve all newer work before editing. Make an isolated feature branch from that actual state if needed.
- All paths below are repository-relative unless stated otherwise. Paths marked **create** are proposed additions, not existing APIs.
- Existing tests passing does not mean the visuals are accepted. Current captures are significantly flatter than the references. All commercial release gates remain open.

### Non-negotiable constraints

1. Preserve approved course geometry, 445 placements, native 544×912 coordinates, collision boundaries and original art files. Keep archive/canonical branches and original ZIPs immutable. Add derived files with provenance and hashes.
2. Keep sprites uniformly scaled and nearest-filtered. Never squash a character, resize each frame to its opaque bounds, stretch the course to the screen, or introduce a hidden physics-map scale.
3. The camera crops/scrolls a true top-down course. Portrait and Fold/wide windows share the same world geometry. UI layout may adapt independently.
4. C++ owns ball movement, wind force, collisions, gameplay RNG, hazards, strokes and score. Godot reads snapshots/events. Cosmetic animal paths, smoke, leaves and ripples may use independent cosmetic state.
5. Do not trigger impact sounds/particles by guessing that a sprite looks near the ball. Use confirmed native events. Nearby animals may react to the location of a confirmed impact.
6. Both male and female golfers remain right-handed through all aim directions. Never mirror the whole golfer to turn left.
7. Keep native meter timing and accuracy boundaries. The red zone means centred accuracy while retaining the power selected by the second tap; it does not force every shot to maximum power. Rough/bunker yellow AND red regions are smaller.
8. Retain the approved menu logo/button artwork. No placeholder controls, invented replacement canon menus, debug labels or unfinished features exposed as working buttons.
9. Use the supplied audio. Three themes play only on title/pause, with 60-second gaps and a random nonrepeating pause opener. Nature ambience uses the established 75% level before the user's volume setting. No gameplay music or unsupplied frog/duck sound substitutes.
10. Do not recreate the scene as a reference-image background. The references specify finish, depth and atmosphere; the actual course must remain playable.

### Visual interpretation of the three references

| Reference | Reproduce | Do not infer |
|---|---|---|
| Clear summer, `10_37_04 AM` | Saturated but readable grass, dark cool foliage shade, crisp warm sun, grounded long shadows, coherent gold/green UI, clear shot focus | Permission to change hole geometry, remove approved objects or copy painted baked shadows into gameplay |
| Mist, `11_03_12 AM` | Soft irregular mist across peripheral trees, visible shafts through foliage, clear central ball/player, moving cloud shade | A uniform white veil, constantly obscured green or grey desaturation of the whole course |
| Golden/autumn, `11_03_20 AM` | Warm light, dark contrasting shade, sparse recognisable leaves, luminous restrained HUD trim | Permission to recolour canonical trees orange wholesale or add heavy fullscreen bloom |

Build summer and morning mist first. Golden hour uses warm lighting over approved assets. Autumn colour variants are optional new derived art only after the main look succeeds.

### Definition of done for EVERY task

- [ ] Implement the stated behaviour, including pause/reduced-motion and restart handling.
- [ ] Run its targeted checks. For behaviour changes, add a regression that fails before the fix. For appearance changes, capture and inspect the real render; do not replace visual review with string-search tests.
- [ ] Compare against the supplied reference at phone viewing size and at 100% pixels. Inspect motion, not just a still.
- [ ] Confirm no gameplay fingerprint change for the same native inputs unless explicitly working on the enabled wind extension.
- [ ] Commit only the intended files and record evidence in `spec/pixtee/visual-polish-progress.md` (**create**). Record task ID, commit, checks, capture/clip paths and unresolved defects. Do not label a task done because code exists.

Review the five high-risk cases throughout: actor behind/in front of a tree; pause during an effect or fade; return from OOB/water; a changed weather/quality setting during a round; and narrow/wide viewport resizing. Their required checks appear below.

## 1. Asset and code map — reuse these first

| Responsibility | Existing files/assets |
|---|---|
| Scene, UI, routing and transitions | `pixtee-godot/scripts/LakewoodGame.gd`, `LakewoodWorld.gd`, `PixteeShotTransition.gd`, `PixteeMenuAttract.gd` |
| Environment | `PixteeLivingWorld.gd`, `PixteeCourseAtmosphere.gd`, `PixteeFoliageBreeze.gd` in `pixtee-godot/scripts/` |
| Weather | `pixtee-godot/addons/pixtee_living_world/scripts/PixteeWeatherDirector.gd`, `PixteeWeatherVisuals.gd`; `pixtee-godot/shaders/pixtee_weather_atmosphere.gdshader` |
| Water | `pixtee-godot/shaders/pixtee_subtle_water.gdshader`; package `assets/premium/river/tileable_flow/flow_*.png` and manifest entry `river_tileable_flow` |
| Confirmed impacts and ball shadow | `pixtee-godot/scripts/PixteeNativeWorldFX.gd` |
| Birds/ducks/caddy | `PixteeLayeredBirds.gd`, `PixteePondDucks.gd`, `PixteeCaddy.gd`; `characters/ducks-v2.png`, `ducks-v2.json`, `caddy-contact-v1.json` |
| Player | `characters/male-cute-v2.png`, `female-cute-v2.png`, `atlas-v1.json`, `face-regions-v1.json`; `PixteeOutfit.gd`, `PixteePlayerProfile.gd`; player rendering in `LakewoodWorld.gd` |
| HUD | `PixteeClubCard.gd`, `PixteeMiniMap.gd`, `PixteeAimTouch.gd`, `PixteeWhackMeter.gd`, `PixteeVerifiedGreenArrows.gd` |
| Existing creature layers | `addons/pixtee_living_world/PREMIUM_ANIMATION_MANIFEST.json`, `scripts/PixteePremiumLayeredSprite.gd`; `assets/premium/{rabbit,squirrel,frog,dragonfly,caddy}/` |
| Course sources | `pixtee-godot/courses/lakewood-hole-1/course-layout.json`, `visual/`, `native-green-v1/` |
| Menus/audio | `pixtee-godot/ui/supplied/`, `PixteeCommercialAudio.gd`, existing owner audio manifest |
| Native integration | `engine/src/classic_shot_model.cpp`, `classic_hole_session.cpp`, `engine/include/sensigolf/`; `pixtee-godot/native/src/pixtee_round_runtime.*`, `pixtee_round_bridge.*`, `pixtee_native_session.cpp` |

Manifest facts: `rabbit_layered_idle` has base/ears/shadow; `squirrel_layered_idle` has base/tail/shadow; `frog_breathe` has body/rock-ripple/throat layers; `dragonfly_heading_00` through `_07` supply directional body/wings/shadow. `frog_hop` is a whole-pose sequence marked as requiring visual approval. Rabbit/squirrel pose sheets are whole-pose assets, not automatically layered locomotion. No verified squirrel-climb sequence exists in this baseline. Do not call idle sprites sliding up a trunk a finished climb animation.

The package's standalone river banks and waterfall are not Lakewood geometry. Reuse water-only frames masked to Lakewood; do not paste a waterfall or replace the river to use an available asset.

### New-asset policy

Before creating art, list the missing pose/mask in the progress file, inspect the existing package and reuse compatible material. Create only the missing item. Preserve canvas size, anchors, palette and pixel density across a sequence. Save source, prompt/tool provenance, source SHA and output SHA. Keep original masters unchanged. Use transparent separate layers for body/limbs/tail/wings and metadata for pivots. Test a whole loop before accepting any generated sheet.

Do not introduce a voxel engine, 3D renderer or isometric camera. Layered 2D art, shadow projections and height metadata supply the required depth. Tiny voxel-like debris may be 1–2 world-unit shaded particles; voxels are optional, not a deliverable.

## 2. Task A — shared presentation state and real depth ordering

**Modify:** `LakewoodWorld.gd`, `PixteeLivingWorld.gd`, `PixteeCourseAtmosphere.gd`, creature renderers. **Create:** `pixtee-godot/scripts/PixteePresentationState.gd`, `pixtee-godot/scripts/VisualPolishCI.gd`.

**Proposed interface:** `PixteePresentationState extends RefCounted`; fields `time_seconds: float`, `paused: bool`, `reduced_motion: bool`, `reduced_flashes: bool`, `quality: int`, `view_rect: Rect2`, `wind_toward: Vector2`, `wind_mph: float`, `sun_direction: Vector2`, `sun_strength: float`, `cloud_cover: float`, `fog_density: float`. `advance(delta: float)` advances cosmetic time only when neither paused nor reduced motion. Initialise one instance per displayed world. Controllers consume it; do not create another gameplay clock or RNG.

- [ ] Inventory existing duplicate clocks/cloud/leaf/shadow emitters. Give each effect one owner. Replace the old implementation when introducing its replacement; do not stack three mist systems or two ball shadows.
- [ ] Use floating-point logical positions for cosmetic motion. Pixel-snap only render positions, consistently relative to the camera. Do not round an actor's stored path position every frame: `PixteePremiumLayeredSprite` currently rounds its own position and can create hopping at small speeds.
- [ ] Define render bands: ground; masked water; ground lighting/shadows; ground decals/arrows; depth-sorted objects/actors; airborne actors/ball as height requires; restrained atmosphere; aim marker; HUD; menus; whole-screen shot fade. Use world foot/contact Y for ground-object ordering, not sprite centre. Keep HUD/menu CanvasLayers outside environmental tint and fog.
- [ ] Split tree rendering into a grounded trunk/base and canopy only where overlap requires it. Reuse complementary masks or derived layers on the same canvas. A squirrel behind a trunk must be hidden correctly; a duck must disappear behind foreground reeds, not draw over them.
- [ ] Preserve zero/half/full visual quality. Avoid per-frame image reads, manifest parsing, texture loads, material recreation or node creation. Cache manifests/textures and pool particles.

**Accept:** An actor can move behind then in front of a tree without suddenly changing size or being drawn over its trunk. Pause freezes all environmental movement; reduced motion removes motion without flashing to a different pose. Switching quality restores hidden actors correctly. Capture the same world point in 360×760, 1080×1920 and 1200×1000; world aspect remains uniform.

## 3. Task B — grounded shadows and directional light

**Modify:** shadow drawing in `LakewoodWorld.gd`, `PixteeNativeWorldFX.gd`, `PixteeCaddy.gd`, wildlife renderers. **Create if useful:** `pixtee-godot/scripts/PixteeWorldShadows.gd` and a shadow-only shader.

**Shared direction:** incoming sun from screen upper-right; cast shadows lower-left, starting normalized direction `(-0.6, 0.8)`. The light direction must not rotate when the camera moves. Weather interpolates sun strength over 3 seconds, not object positions.

- [ ] Give golfer, caddy, spectators, photographers, sponsor boards, trunks and ground animals a contact shadow attached to their existing verified foot/base anchor. Player starting ellipse radii 3.5×1.2 world units, alpha 0.22–0.30. No detached upright duplicate of a person.
- [ ] Add a separate projected cast silhouette from an alpha mask. It is a shadow-only projection anchored at the feet; never change the visible sprite's scale/aspect. Initial length: player/caddy 0.65×body height in summer, 1.2× in golden hour; trees 0.45×display height and 0.8× respectively. Initial opacity 0.16–0.24, cool green-black. Fade toward the tip, with a 1–2 world-unit soft edge.
- [ ] Remove the current displaced upright tree-shadow copy when the projected shadow replaces it. Do not darken overlaps indefinitely: combine related canopy shadows in a bounded mask before applying opacity. Ground should remain recognisably green in shade.
- [ ] Keep a tiny foot shadow even in overcast/mist; reduce the cast shadow with sun strength. Feet must not slide when breathing.
- [ ] Ball shadow stays at the native world ground position while the ball sprite rises by native height. Use the existing MAPS-to-world conversion. Radius may grow slightly from 1.2 to 2.3 world units and opacity fall from 0.32 to 0.10 with height; cap both. Never derive height from a fake parabola.
- [ ] Modulate separate leaf/canopy highlight masks by sunlight and cloud cover, initially 3–7% luminance. Preserve outlines and base palette. Do not apply a glossy highlight to every sprite pixel or recolour faces.

**Accept:** Freeze a golfer facing each of eight directions: foot/shadow gap ≤0.5 world unit. Ball and its shadow reunite on a native ground contact. Tree/crowd shadows point in one direction. Mist retains grounded contacts. No shadow darkens HUD or hides the cup. Compare summer and golden-hour captures with the references.

## 4. Task C — cloud shade, sunlight shafts and mist

**Modify:** `PixteeWeatherDirector.gd`, `PixteeWeatherVisuals.gd`, `PixteeCourseAtmosphere.gd`, `pixtee_weather_atmosphere.gdshader`. Reuse existing weather presets. All values here are initial art-tuning values, not physical constants.

| Effect | Initial settings | Boundaries |
|---|---|---|
| Cloud shade | 2–3 irregular fields, 120–240 world units across; drift 0.25–0.65 world units/sec; opacity 0.07–0.14 | Combined darkening ≤0.22; soft edges 18–35 units; world-anchored |
| Sun shafts | 2–4 broad broken beams from upper-right; width 18–40 units; alpha 0.03–0.08 | Irregular mask; slow modulation 12–25 sec; no animated diagonal stripe wallpaper |
| Morning mist | 2 near-ground fields plus 1 faint foreground field; widths 70–160 units; drift 0.4–1.2 units/sec | Typical alpha 0.04–0.10, combined ≤0.18; peripheral concentration |
| Summer haze | One very faint warm veil near tree margins | Alpha ≤0.025; no fog covering the fairway |
| Golden light | Warm shaft tint around `#FFE5A0`; cooler shadows | No fullscreen orange filter; maintain white ball and clothing readability |

- [ ] Replace visibly repeated circles/regular sine bands with reusable soft noise/masks, already supplied if available; otherwise create a small procedural noise texture. Do not invent new landscape art.
- [ ] Anchor masks to course coordinates; scrolling camera reveals the same cloud/mist patch. Wrap outside view with a crossfade, never reset at screen edges.
- [ ] Use separated back/foreground mist planes with different speeds, very restrained parallax. Atmospheric softness is allowed; sprite textures remain nearest-filtered.
- [ ] Reduce foreground fog/shafts near the active ball and cup using a soft visibility mask: radius 10 units clear centre, returning to full effect by 28 units. Apply to fog/light overlays only, not collision geometry or physical shadows.
- [ ] Blend presets through the existing director. Honor pause and reduced motion in interpolation. No unsynchronised shader `TIME` should keep moving while paused; use the shared time uniform.
- [ ] Rain/drizzle use small slanted streaks, pond ripples and restrained wet glints. Spawn ground rain splashes only where the visual surface mask permits; rain particles are cosmetic, not native ball impacts. No new thunder/audio without supplied recordings.

**Accept:** Record 30 seconds of mist and summer at a fixed camera, then pan. No screen-locked fog, regular stripes, opacity pumping or layer restart. Ball/cup/aim remain visible. A weather change has no effect on native ball physics unless Task W explicitly changes the native wind state before a round.

## 5. Task D — river flow, pond reflections, ripples and sparkles

**Modify:** Lakewood water polygon creation, `pixtee_subtle_water.gdshader`, package water-only animation usage; add a small pooled water FX helper if needed.

- [ ] Derive a water mask from the existing `water` and `river` polygons in `course-layout.json`. Keep the bank edge stationary. Derive a 2–4 unit bank-distance fade for moving/reflected detail. The complete visible water boundary must remain aligned with native hazard mapping.
- [ ] Use the authored `flowLine` to assign river flow direction locally. For each river mesh vertex, use the tangent of the nearest flowLine segment; interpolate through neighbouring segments so bends do not pop. Cache this at load. Never make the entire bent stream flow in one global direction.
- [ ] Keep the existing water base. Overlay water-only flow at initial opacity 0.10–0.18. River detail speed 0.6–1.4 world units/sec; pond drift 0.08–0.20. Sample the flow texture with consistent world texel density; do not stretch it to fit polygons. Hide tile repetition with a second low-opacity phase at a different offset, not by distorting banks.
- [ ] Reflections: start with cool sky/cloud tint and faint silhouettes from nearby bankside trees/reeds. Use a separate water-clipped reflection mask, alpha 0.06–0.14, disturbed by tiny ripple offsets only within water. Reflection-only projection is permitted; visible source sprites remain unchanged. Do not reflect every tree on the course or the HUD. A full reflected viewport is unnecessary.
- [ ] Sparkles: seed positions inside water, choose a few based on sun strength, smoothly brighten then fade for 0.4–1.0 seconds. 3–8 visible at Standard quality, maximum 14 at High. Each core 0.5–1 world unit with a faint halo up to 2. No random white noise flashing every frame; none over grass/banks.
- [ ] Native water impact: one expanding ring set and one droplet burst per event ID; ring lifetime 0.8–1.4 seconds, radius 1→10 units. Duck wakes are separate cosmetic rings: 0.6–1.0 second lifetime, tiny trailing arcs, proportional to swim speed, capped at 12 visible wake pieces total.
- [ ] Keep animation frame canvases aligned and banks static. Check shader modulation: preserve vertex tint/alpha when sampling a texture; do not multiply the source texture into itself or discard crossfade alpha.

**Accept:** Watch a river bend, pond edge and native splash for 30 seconds each. No seams crawling over grass, flashing tile rows or detached wake. Pause freezes all movement. Repeat one impact ID: no second burst/sound. Low quality retains the same water boundary and colour.

## 6. Task W — real native wind and a truthful wind display

**This is a gameplay task, not a cosmetic shader.** The inspected baseline exposes no verified ball-wind force/state in the bridge; weather currently supplies cosmetic wind. Older specs explicitly describe wind as an optional extension. Do not label a newly authored force as recovered original wind.

**Inspect/modify:** `engine/src/classic_shot_model.cpp`, `engine/include/sensigolf/classic_shot_model.hpp`, hole/session state, `pixtee_round_runtime.*`, `pixtee_round_bridge.*`, `pixtee_native_session.cpp`. **Create:** `engine/include/sensigolf/authored_wind.hpp`, `engine/src/authored_wind.cpp`, `engine/tests/authored_wind_tests.cpp`; register them in the actual CMake/SCons source lists.

### Required native contract

- [ ] Define `AuthoredWindState`: enabled flag, direction-toward raw 0..4095, speed in tenths of mph, versioned response coefficient. Normal new Pixtee rounds start with a gentle 6 mph toward raw 1536 (upper-right); the existing fidelity/tests explicitly set disabled/zero. Keep wind constant during a shot and initially throughout this one-hole round. Do not add random gust physics in this first implementation.
- [ ] Expose `set_authored_wind(direction_raw, speed_mph_x10, enabled)` through runtime/bridge. Permit it only before the first shot of a new round. Validate integer ranges, reject invalid values and mid-shot mutation. Use 0–150 tenths mph for the initial product range. Future changes can widen it deliberately.
- [ ] Export snapshot fields `wind_enabled`, `wind_toward_raw`, `wind_speed_mph_x10`, `wind_model_version`. The HUD and foliage read these fields; they never create forces. The arrow means **where the wind blows toward**. Native convention: 0 down, 1024 right, 2048 up, 3072 left. Label speed in mph, rounded for display only.
- [ ] Record wind configuration and any residual force state in native save/replay commands and fingerprints. Restore identical wind before replaying a shot. Bump compatibility identifiers as required; reject incompatible sessions clearly, never silently reinterpret an old shot with new wind. Keep wind-enabled score records distinguishable from historical calm records.

### Force integration — use the existing shot solver

- [ ] Keep the original `recovered::step_clear_air(FlightState&)` unchanged. Add an authored wind helper called from the existing native shot path before its airborne advance. Do not move the ball after collision resolution and do not run a second trajectory.
- [ ] Apply horizontal acceleration only on ticks that use the airborne flight path. Never accelerate a putt or grounded roll, during hazard recovery, after a score, or while native simulation is paused. Gravity, vertical launch, source drag, spin, bounce and collision responses stay in their existing code paths.
- [ ] `FlightState` stores horizontal force and direction, not a ready-made Cartesian velocity. Use the recovered integer trig/projection helpers to obtain X/Y force components, add the wind impulse with checked 64-bit intermediate arithmetic, then resolve back to native horizontal magnitude/direction using deterministic integer math. Retain fractional/angular quantisation residuals so light wind is not rounded away every tick. Clear/rebase residuals on launch and native contact/deflection as required; serialize them. Do not use platform-dependent floating-point `atan2` as gameplay authority.
- [ ] Native MAPS coordinates have a different scale: verify world/green conversion and scale the force consistently when crossing that boundary. A green-mode transition must not double or halve the apparent wind suddenly.
- [ ] Start calibration at **96 native horizontal-force units per tick per mph**, with speed tenths handled by integer remainder accumulation. This is an authored tuning seed, not a recovered or real-world physical conversion. Tune only this versioned coefficient within 32–192 after running the measured fixture below. Do not change gravity/club tables to hide poor wind response.
- [ ] Calibration fixture: flat obstacle-free authored terrain, same seed, Driver, power 105, accuracy 63, aim 2048. Compare calm and 8 mph perpendicular winds toward 1024/3072 at first ground contact. Target cross-track drift of 5–10% of calm airborne carry, symmetric left/right within 1 world unit or 2%, whichever is larger. At 15 mph, drift must increase and remain under 22% of calm carry. Headwind reduces carry, tailwind increases it, with no reversed motion or discontinuity. A shorter-airtime low shot should accumulate less absolute drift. These are game-feel acceptance targets, not claims of physical aerodynamics.
- [ ] Exact zero-wind fast path: skip all vector reconstruction/residual updates so the complete original native trace is byte-for-byte unchanged. Zero wind must pass the old golden/source tests.

### Tests and wind presentation

- [ ] Native tests: zero-wind exact parity; both crosswind signs; head/tail direction; monotonic 4/8/15 mph response; grounded putter unchanged; repeated replay identical; save midflight/restore identical; contact and green-mode transitions finite and continuous; paused wind tick/state unchanged; invalid/mid-shot setters rejected.
- [ ] Bind the resulting native vector into `PixteeWeatherDirector.set_native_wind_visuals(..., verified=true)`. Set verified only for a snapshot from the actual core. Weather presets may change fog/rain/light; they must not silently alter ball wind. Wind selection in Settings applies to the **next round**, clearly labelled.
- [ ] HUD wind needle and number update from the same current snapshot. At 6 mph toward raw1536 the arrow points upper-right, never toward the source of the wind. Calm shows a small calm icon and `0 mph`. Never display the reference's example 7/8 mph as a hardcoded number.
- [ ] Add one native integration test that reads the HUD data and verifies the corresponding ball drift direction. Use explicit zero wind for old three-shot score fixtures; add a separate full playable wind round test instead of weakening their assertions.

**Accept:** A player can deliberately aim against visible wind and observe repeatable drift. Changing cosmetics/quality cannot change it. Complete native AI, when integrated, must evaluate the same wind state. The existing authored menu shot sequences are not proof of AI capability.

## 7. Task E — coherent wind, leaves, grass and particles

**Modify:** `PixteeFoliageBreeze.gd`, `PixteeCourseAtmosphere.gd`, `PixteeLivingWorld.gd`, `PixteeNativeWorldFX.gd`. Consume Task W's native wind vector and Task A's presentation state.

- [ ] Trees: fixed trunk/base, canopy rotation around its natural attachment, initial amplitude 0.3–0.9 degrees at 6 mph, maximum 1.5 degrees. Reeds/flowers/grass rotate around roots 1–3 degrees, maximum 4. Flags use existing cloth frames. No UV liquefaction, scaling or whole-tree rocking at the roots.
- [ ] Use a shared slow gust envelope for visual amplitude, period 5–9 seconds, with position-based phase offsets. It may enrich foliage movement but must not claim a new ball-force gust. Native wind direction controls lean and drifting effects; not all plants oscillate in lockstep.
- [ ] Falling leaves: reuse actual leaf artwork or create a tiny pointed leaf/stem mask. Never plain rectangles/squares. Source from actual tree canopy positions; 1 leaf per visible tree every 12–35 seconds, maximum 8 active at Standard. Lifetime 4–7 seconds, drift 2–6 units/sec with wind, gently rotating, then settle/fade over 0.8 seconds. Maintain constant sprite size. At High, at most 2 occasional slightly larger foreground leaves at screen margins; never across the ball/HUD.
- [ ] Add 6–12 subtle pollen/dust motes at Standard only where shafts/sunny margins make them visible. Size 0.4–0.9 world unit, low alpha, slow drift. Avoid filling the playable fairway with glitter.
- [ ] Upgrade confirmed impact bursts: grass 5–8 thin blades (0.35–0.55 sec), rough 7–10 darker blades (0.45–0.7), sand 10–16 grains plus one small dust puff (0.5–0.9), water per Task D, tree 4–8 leaf pieces, bird 3–6 feathers. Direction/spread comes from the actual event data if present; otherwise use restrained cosmetic spread around its confirmed position, without asserting an unprovided collision normal.
- [ ] Use event types actually exported: `authored_tree_contact`, `authored_bird_contact`, `authored_board_contact`, `fairway_contact`, `rough_contact`, `bunker_contact`, `water_contact`, `verified_flag_contact`, `cup_lip`, `cup_capture`. Check the bridge for part/height fields before using them. Do not invent `tree_trunk_contact` as an emitted type. Route creature fright from the verified event bus once, with bounded ID dedupe and reset on new/replayed sessions.
- [ ] Photographer flash: tiny camera-local core 1–2 units and faint halo 3–5; 0.08–0.12 sec. Schedule occasional anticipation/confirmed-shot reactions, no repeated fullscreen flash. Reduced-flashes disables it. Crowd gestures remain local and do not all trigger together.

**Accept:** Leaves are recognisable at phone size, wind direction agrees with the HUD, all roots remain fixed, repeated event IDs create one effect/sound, and restarting a round allows new events with reused serials. No unbounded emitters or square placeholder debris.

## 8. Task F — wildlife with real states and deliberate activity

**Modify:** `PixteeLivingWorld.gd`, `PixteePondDucks.gd`, `PixteeLayeredBirds.gd`, package layered-sprite player. **Create:** `pixtee-godot/courses/lakewood-hole-1/wildlife-habitats.json` and, if needed, `pixtee-godot/scripts/PixteeWildlifeController.gd`.

Store habitat anchors/routes after validating them against existing course polygons and placements. Use an independent seeded cosmetic RNG, never native gameplay RNG. Keep creature paths in world coordinates. Offscreen actors retain lightweight logical state; fade/spawn only under cover, never in an empty visible fairway. For QA, allow a test-only forced encounter mode; remove it from normal menus.

| Creature | Required state sequence and timing | Layer/motion requirements |
|---|---|---|
| Ducks | Two pond residents. Drift 8–20 sec → turn 0.3–0.6 sec → paddle 5–12 sec → drift. Verified nearby splash/impact within 45–55 units: alert 0.25 sec → swim away 3–5 sec → settle. Fright cooldown 8 sec. | Use v2 eight headings. Constant individual scale; choose heading from velocity with angular hysteresis to prevent toggling. Smooth opacity transition preserving shader tint. Body/head/wake layers; tiny head bob, no bobbing whole duck out of water. Speeds 1–2.5 units/sec normal, 4–6 startled. Constrain full body to pond with margin, not only centre. No flight unless flight art is supplied. |
| Dragonflies | One near reeds, optional second at High. Hover 2–5 sec → curved dart 0.6–1.5 sec → hover; occasional perch 3–7 sec. | Use all eight existing directional body/wing sets. Hover ±0.5–1 unit, darts 10–20 units/sec. Wings loop independently; body turns toward motion. Small height-ground shadow. Stay within authored pond-edge/reed habitat; never orbit the player. |
| Frogs | One bank/rock resident; second appears occasionally. Breathe 6–15 sec → look/alert 1 sec → short hop 0.4–0.7 sec → settle 0.25 sec. Rare native nearby splash makes it hop to bank cover or into water. | Reuse throat/body/rock layers. Validate existing hop art; derive complementary body/leg layers only if needed. Feet lift in hop and reconnect to the fixed ground shadow. Do not move a baked rock with the frog. For entering water, native ball audio must not fire; use a silent small cosmetic ripple unless an owner frog sound exists. |
| Squirrel | Occasional 45–110 sec interval per tree cluster; at most one visible. Appear behind trunk → climb 2–4 sec → perch/tail 5–12 sec → retreat behind canopy or descend 2–4 sec. Nearby confirmed tree impact can accelerate retreat. | Author tree ID, base anchor, trunk route and canopy occlusion zone. Use alternating paw placements and body compression/translation without changing scale, plus independent tail. Existing idle-tail loop alone is insufficient. Add a small 6-frame climb sequence if needed, with consistent canvas/pivots and layered limbs. No ground shadow floating up the tree. |
| Rabbit | Rare edge visitor every 50–120 sec, visible 8–18 sec. Emerge from cover → 2–4 hops → sniff/ear twitch 3–6 sec → return/flee into cover. | Existing ears/body/shadow; use inspected pose frames to build layered locomotion if required. Hop 4–7 units, 0.35–0.5 sec; land before next hop. Avoid fairway aiming corridor, water and bunkers. No teleport/disappearance in open ground. |
| Flying birds | Keep existing native schedules, hit detection and species grouping: generic birds 2, gulls 3, blackbird/kingfisher solo. | Existing fixed-canvas frames and body/wing layers. Constant scale across flap frames. Wing phase offset per member. Rendering stays aligned with native positions/height so a visible bird can actually be hit. Never add independent visible evasive paths for collidable birds. Only the hit member reacts; surviving members continue until native visibility ends. |

- [ ] For cosmetic animal fright, use distance to a **confirmed** event, never the rendered ball alone. Add/reuse correct `authored_tree_contact` routing for squirrels. Dedupe once per native event.
- [ ] Keep world fauna rate calm: usually 1–3 secondary activities visible, not every creature performing at once. Priority is golfer/ball, then habitat activity, then rare encounters.
- [ ] Use authored layers for movement. Do not replace a layered actor with a whole sprite and claim the requirement is met. Render transitions may crossfade, but two differently posed fully opaque animals must never overlap.

**Accept:** Record a 90-second pond view and 120-second tree-edge view with forced encounters, then normal timing. Ducks show several headings with no changing size; frog has grounded landings; squirrel visibly grips/climbs and becomes occluded; rabbit comes from/returns to cover. Pause all states and resume without jumps. A real native bird-hit test still matches the visible member and produces one feather/audio response.

## 9. Task G — golfer, crowd and caddy animation finish

**Modify:** player rendering in `LakewoodWorld.gd`, `PixteeOutfit.gd`, `PixteePremiumLayeredSprite.gd`, `PixteeCaddy.gd`; add per-pose anchor/mask metadata only where needed.

- [ ] Validate all eight headings for male/female address, backswing, contact and follow-through. Keep a common foot anchor and stable apparent height. The present atlas has a limited authored pose sequence; inspect actual frames rather than assuming a full smooth swing exists.
- [ ] Split or mask hips/legs, torso/head, hands/club where useful. Keep the same source canvas and align overlaps by 1–2 source pixels to prevent seams. Interpolate small rigid-layer translations/rotations between authored poses. Do not rotate the entire flat golfer like a cardboard cutout, mirror handedness, scale limbs, or interpolate the club through the body.
- [ ] Ready breathing: torso/head movement 0.2–0.45 world units, period 2.8–4.2 sec; feet fixed. Backswing follows meter phase smoothly; native contact triggers the contact pose/sound once. Follow-through is presentation after native launch and never delays/duplicates the native shot.
- [ ] Outfit changes must update both upper/lower materials atomically. Name, skin, shirt, trousers, cap, hair, face, beard and fringe preview must match gameplay after Apply. Cancel restores previous settings; pause edits do not alter native state. Existing limited hair/fringe options must be named honestly; create additional silhouettes only if a missing approved option is being completed.
- [ ] Crowd: retain feet, use subtle layered breath/head/arm motion with independent phases. A small subset reacts to a shot. No synchronous bouncing or whole-body scale pulsing.
- [ ] Caddy: remains on validated land just outside the green, clear of bunker/cup/line of play. Flag/pole visible in hands only when `flag_removed_for_putting` is true; the cup renderer and native flag collision use that same state. Keep caddy's feet/shadow attached and cloth/breath animated. No new physical caddy collision is implied by these visuals.

**Accept:** Capture male/female full swings in eight directions, plus a 30-second idle. No handedness changes, feet sliding, head tearing, club teleport, skin tint on clubs/eyes, or double body shadow. Repeatedly apply five different outfit combinations during pause and resume. Native score/replay tests remain unchanged.

## 10. Task H — reference-quality HUD, minimap, aiming and power bar

**Modify:** `PixteeClubCard.gd`, `PixteeMiniMap.gd`, `PixteeAimTouch.gd`, `PixteeWhackMeter.gd`, `PixteeVerifiedGreenArrows.gd`, HUD layout in `LakewoodGame.gd`. Use existing approved ball/club/UI art; new frame/glow masks may be small procedural assets.

### Layout and common style

- [ ] Design in logical 360×760 UI pixels, with safe margins at least 8 px. Uniform UI scaling; recompute layout for narrow/Fold/wide windows. Do not enlarge UI until it hides the active player/cup. Use dark evergreen panels (`#071C16` starting colour), warm cream text, restrained gold inner line and dark outer stroke. Suggested border: 1 px dark outline + 1–2 px gold + faint 2–4 px outer glow. Avoid shiny oversized bevels.
- [ ] Top-left card: initial 164×126 px. Put the ball/lie vignette in its top 40–44 px, distance left and wind arrow/speed right below it, selected club/icon in the bottom row. Main distance 26–30 px; labels 13–15; club 17–19. Wind arrow ~18×18 with a tiny light edge and `6 mph` below. Keep a clear divider between distance and wind. The native lie, distance, club and wind values are live, never copied from the reference numbers.
- [ ] Card tap cycles clubs exactly once. No popup list or separate previous/next buttons. Minimum usable touch region 44×44 logical px. Animate a short border highlight/icon settle on selection, not the whole screen.
- [ ] Header hole/par/stroke and pause control must not collide with the card/minimap. Make the pause button legible and easy to hit; replace the current tiny-looking artwork treatment with a consistent 44×44 target using approved controls.

### Minimap and aim

- [ ] Keep minimap top-right and standalone. Preserve map aspect; crop/letterbox inside its frame. Initial compact content about 72–84×121–141 px, enlarged about 144–174×242–292 within available safe area. Double tap toggles a 0.25-sec cubic/eased size transition; anchors stay top-right. Reduced motion changes instantly. Never double-scale map markers or let a single tap accidentally aim/shoot through it.
- [ ] Show actual ball, cup and a subtle current shot-direction line. Keep north/world orientation consistent with the wind arrow. Do not integrate unrelated menus in the minimap.
- [ ] On-course aim: retain invisible left/right tap regions. Dotted line originates at the native ball and uses the current club-dependent length. End marker can be a small red/white target with a soft 1.2–1.8-sec pulse; 1–3 restrained motes at High only. Fade aim during meter/flight/transition. No fake landing prediction, distance bonus or trajectory physics in Godot.
- [ ] Native green arrows: read the verified 16×16 direction/magnitude fields. Small grass-integrated chevrons, initial 2–3 world units and opacity 0.22–0.38, visible mainly while putting/aiming. Cap visual brightness and density without inventing slopes. Suppress arrows inside a 5-unit cup radius and 4-unit ball radius; never blanket the entire green with thick neon arrows.

### Whack-o-meter

- [ ] Keep the single long bar at the bottom, logical outer rect approximately x=10, width=340, height=38–44, bottom margin=10–14. Keep generous invisible touch height 56 px. No Start Swing button, numbered steps, Hook/Straight/Slice text or duplicate floating power controls.
- [ ] First tap starts travel left→right. Second captures chosen power and immediately returns. Third captures native accuracy. No handcoded GDScript timing or manual broadening of the native red/yellow zones. Preserve the already implemented slowdown and test it before making further speed changes.
- [ ] Red `#FF283C`, yellow `#FFE338`, high contrast against the track. Keep the tested native mapping of positions/zones, even though the decorative references place colour regions differently. If the entire axis is intentionally reversed for a new design, indicator, scale, zone centres and input interpretation must transform together and native assertions must pass; prefer retaining current mapping.
- [ ] Add a dark inset track, a fine warm rim, subtle gradient depth and a continuous moving indicator (approved ball or bright ivory needle with a small glow). Indicator core 3–5 logical px, halo 6–10; short trail 12–22 px that follows current direction and disappears at rest. No oversized bloom covering the accuracy region.
- [ ] Draw quiet ticks at 0/25/50/75/100. Keep the selected-power marker visible during the return phase. Accuracy red means straight at that chosen power. Red/yellow zone widths come directly from native lie metadata; tee/fairway easier, rough/sand smaller. Keep minimum visibility through outline/glow, not by lying about the hitbox width.
- [ ] When a shot is available, run a restrained cyan/gold rim breathing glow, period 1.5–2 sec, alpha 0.12–0.28. Stop it while meter/flight/pause/fade is active. On second/third press show a 0.12–0.18-sec local pulse, with no extra tap or sound triggered by animation callbacks.

**Accept:** At 360 px wide, read distance, wind, club, power and accuracy without zooming. Tap every control on a real input run; no gesture leaks. Test tee, rough, bunker and putter native windows. Selected power persists. No effects hide the red zone. Capture compact/enlarged map and each meter phase alongside the references.

## 11. Task I — title, pause, customisation, results and transitions

**Modify:** `LakewoodGame.gd`, `PixteeMenuAttract.gd`, `PixteeShotTransition.gd`, profile/menu styles; reuse `ui/supplied/`.

- [ ] Title: approved logo and wooden button assets over a slowly moving live native course. Preserve visible world detail behind the menu; add a soft local readability backing only under text/buttons. Do not replace it with a flat black screen. Hide the postponed AI-golfer roster section and loading Skip control.
- [ ] Keep an approximately 90-second slow flyover, 3-second background fade-out, 1-second hidden camera/hole change, 4-second fade-in. Menu/logo/music controls remain fully visible and stationary throughout. With one hole, restart the same course naturally. Keep future hole selection behind a data interface, not copied scene logic.
- [ ] Current attract golfers are isolated native rounds driven by authored inputs. Complete native CPU shot choice is a separate functional gap. Do not advertise it as solved by slowing animation. If implementing that gap, use the recovered C++ CPU evaluator and the same wind state; do not add GDScript trajectory selection. This visual document does not specify a new AI algorithm.
- [ ] Pause: show a dimmed frozen course behind a coherent approved-art panel, legible Resume/Player/Scorecard/Settings/Restart/Main Menu controls and a visible Music toggle. Keep touch targets ≥44 px, panel width within screen margins, and scroll only when necessary. Restart/leave confirmation uses the same visual style.
- [ ] Customisation: prominent grounded animated preview, readable two-column labels/selectors where width permits, consistent panel frames, live changes, fixed visible Apply/Cancel area, name field with caret/keyboard-safe layout. Male/female, skin, hair, face, beard, fringe, cap/shirt/trousers must be discoverable without tiny controls. Scroll content behind the fixed actions, not through them.
- [ ] Settings: matching visual hierarchy; clear audio, motion/flashes, quality, frame limit, cosmetic weather and next-round native wind groups. Never expose raw shader coefficients to players.
- [ ] Score/results: native hole score, par/relative score and persistent statistics, with restrained provided stinger and reveal. Show only supported one-hole actions. Do not fabricate career/trophy content. Use the same button/frame vocabulary, not default Godot widgets left unstyled.
- [ ] Preserve the implemented whole-screen shot transition: wait 1.0 sec after native stop, fade out 0.32 sec, relocate hidden, hold 0.08 sec, fade in 0.42 sec. Do not delay the native physics or scoring. Pause/background interruption settles the transition once; the score screen must not remain behind a black overlay.
- [ ] Audio: title/pause Music toggles persist and agree. Three replacement themes only, random nonrepeating pause opener and 60-sec gaps. Gameplay uses supplied SFX and spaced nature ambience; no gameplay theme. Confirm control animations do not duplicate clicks/club impacts. Duck/frog/rabbit movement stays silent unless matching owner audio exists.

**Accept:** Record title for a complete 98-second loop with menu opacity unchanged. Navigate title→player→settings→play→pause→resume→score→title at portrait and wide sizes. Verify Apply/Cancel, Music toggle and focus loss. Every visible button works, no controls leave the viewport, no black-screen trap and no original copyrighted media appears.

## 12. Task J — performance, evidence and final acceptance

### Starting effect budgets

| Budget | Low | Standard | High |
|---|---:|---:|---:|
| Live impact flecks | 32 | 96 | 128 |
| Live ripple/wake pieces | 6 | 12 | 16 |
| Falling leaves | 0–2 | 8 | 16 |
| Motes | 0 | 12 | 24 |
| Sparkles | 2 | 8 | 14 |
| Concurrent mist planes | 0–1 | 3 | 4 |
| Secondary active fauna in view | 2 | 4 | 6 |

Quality changes cosmetic density, never native birds/collisions/wind/ticks or animal size. Keep ducks present at Low if visible; simplify animation/effects rather than hiding essential presentation abruptly. Cull outside the visible world plus 48-unit margin. No per-frame atlas parsing or unbounded CPUParticles/GPUParticles creation. Use the existing GL-compatible effect path; do not change renderer just to obtain glow. Use small glow textures/shaders if the selected renderer lacks an intended postprocess feature.

- [ ] Add `VisualPolishCI.gd` to the existing scene workflow for pause/resume, bounded pools, quality switching, shader alpha, anchor/depth fixtures and one confirmed event per reaction. Extend existing `PlayerReportedRenderingCI.gd`, `MobileHUDCI.gd`, `WhackMeterCI.gd`, `PlayerJourneyCI.gd`, `MenuJourneyCI.gd`, `ShotTransitionCI.gd`, `MenuVisualCI.gd` only where their behaviour changes.
- [ ] Run the existing native build/CTest and Godot 4.5 import/headless checks. Use the commands already in `.github/workflows/pixtee-lakewood-playable.yml`. Add the wind native tests to that build. Do not weaken old assertions to get green checks.
- [ ] Capture actual GL renders at 360×760, 1080×1920 and 1200×1000: tee summer, tee mist, golden light, river bend, pond animals, tree squirrel, male/female swing/contact, green/caddy/arrows, each meter phase, expanded minimap, title, pause, customisation and score. For static tests, update both native snapshot and all HUD controls before capture; do not leave a stale club label in a putting screenshot.
- [ ] Record clips: 30-sec light/cloud/river; 90-sec pond; 120-sec encounters; eight-direction swings; full native round including water/OOB/pause; full title fade loop. Keep a deterministic cosmetic capture seed, with forced encounters restricted to QA. Normal gameplay remains occasional.
- [ ] Measure frame time during the busiest visible view on the actual test machine; record device, renderer, resolution, median/p95, memory and effect counts. Target Standard at 60 fps on supported normal Android/Fold devices, but do not claim device performance from desktop or headless tests. Android/Fold approval/testing remains later; no APK now.

### Grade-1 visual acceptance rubric

Score each item 0–3: 0 broken/missing, 1 present but visibly rough, 2 coherent and polished, 3 convincingly matches the reference's finish. Required: no item below 2; lighting/depth, character motion and HUD must reach 3. This rubric structures the owner's review; it does not replace it.

1. Grounded coherent shadows and light direction.
2. Soft convincing mist/cloud/shafts with a clear playable centre.
3. Flowing water, bank-safe reflections, restrained sparkles/ripples.
4. Smooth right-handed golfer/crowd/caddy animation and stable outfits.
5. Natural layered fauna, recognisable motion, no pop/scale/occlusion defects.
6. Attractive readable reference-style HUD, truthful wind and native meter zones.
7. Approved-art menus, consistent customisation/results and smooth fades.
8. Clean supplied audio routing and no gameplay music.
9. Consistent pause/reduced-motion/quality behaviour and no distracting flashes.
10. Smooth measured performance and preserved native determinism.

Finish by reporting implemented tasks, exact tested commit/build, native-wind calibration evidence, real screenshot/clip comparisons and outstanding gaps. Keep commercial gates open until their actual evidence exists. A successful compile, a decorative screenshot or many added particles is not a beautiful finished game.

## 13. Execution order and short instructions to the implementing model

1. Read section 0 and the existing handoff/checkpoint. Capture the current baseline once.
2. Complete A (state/depth), B (shadows), C (atmosphere), D (water).
3. Complete W (native wind) before promising a live wind HUD or wind-driven gameplay. Preserve explicit zero-wind test fixtures.
4. Complete E (vegetation/particles), F (wildlife), G (characters).
5. Complete H (HUD/meter), I (all menus/transitions/audio).
6. Complete J, inspect real renders and fix failures. Deliver a Windows review build when requested. Do not build an APK.

For each task, read this document's global constraints and only that task plus its named dependencies/files. Do not start by auditing every recovered engine function. Reuse existing assets and code. Ask the owner only for a real missing decision or unavailable required source; routine implementation choices are settled here. Record unresolved functional gaps honestly, especially native menu AI and device acceptance. Do not substitute more planning documents for executing the tasks.
