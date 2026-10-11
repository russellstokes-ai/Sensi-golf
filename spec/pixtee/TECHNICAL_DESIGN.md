# Pixtee Golf — Android and simulation architecture

## Runtime

Android Java/Kotlin UI with a native-independent deterministic gameplay service or shared module. Render on SurfaceView/OpenGL ES/Compose Canvas at device refresh rate while gameplay uses a fixed tick and integer-counted input capture. No real-time frame duration should change ball outcomes. Support 60/90/120Hz screen refresh, paused application and resume, accessibility touch/gamepad settings.

**Do not link Sensi-golf recovered C++ sources** into this independent app.

## World / renderer specification

Use an original 2D logical coordinate system with:
- position and height separate (ball arc depth cue);
- editable course collision/terrain map and independent authored art layers;
- renderer-side camera offset + dynamic UI safe rectangle, with world-to-screen transform applied consistently to ball, flag, putt marker, aiming arrow and panoramas;
- selectable crisp nearest-neighbour pixel scale in Classic Graphics; improved sprite interpolation or high-resolution artwork in Enhanced Graphics;
- stable user-adjustable camera lock, pans, optional zoom, and cutout-aware HUD;
- collision/physics sampled from game-world data, never from painted pixels.

**Camera constants are placeholders until independently measured:** world units per logical pixel, camera tracking response, dead zone, tee/cup anchoring, ball/flag proportions, map-axis conventions. Supply per-device view aspect tests; don't invent original numeric values.

## Three-click input pipeline

State transitions: Ready -> PowerSweep (press 1) -> AccuracySweep (press 2 captures raw power) -> ShotCommitted (press 3 captures raw accuracy) -> Flight -> Rest/Terminal -> Ready. Aim and club lock while either meter sweep is active. Never submit a shot on rotate, backgrounding, double tap, overlay, or button press auto-repeat. The power and accuracy sweeps use their own independently engineered and tunable repeatable curves, calibrated with gameplay observations.

Default controls:
- Aim: drag a clear on-course aiming dial; +/- step arrows, gamepad D-pad/stick.
- Club: scroll list / left and right arrows; gamepad bumpers; show shot category and loft.
- Welly: one stable thumb target for all three taps, stages visibly identified.
- Camera: two-finger pan/pinch or a secondary camera control, separate from aiming touches.
- Pause, scorecard, settings, help and replay.
- **Standard portrait Android phone is primary**; Fold closed and open adapt via layout, not altered world size. Keep minimum 48dp touch targets. Horizontal Whack-o-meter with source-game-backed swing and accuracy phases; direct touch to left/right of target X for slow steering and club tap to change club. Onboarding, not gameplay captions, teaches the controls.

## New physics contract

Functional components: ball state, aimed heading, club launch model, surface-lie modifier, curved flight, gravity, pitch/landing impulse, terrain friction, putt slope, collision with cup/flag/hazards, deterministic RNG and round scorecard. Numeric values are original Pixtee design parameters. No copied original physics table, program text, trace array or course collision lookup bank.

Pure simulator: `simulate(tick, input, terrain) -> snapshot`.
Recorded replay uses versioned seed, content ID/hash, course revision, stable input events and deterministic core version. Write unit tests for all shot types and compare generated histories against Pixtee golden masters that are independently authored.

## Course files

Use `game_data.schema.json`. Each hole uses new geometric primitives, multiple collision tags (tee/fairway/rough/semi/green/bunker/water/OOB), new slopes and height/camera layers, original par, designer-authored yardage and landmark objects. Coordinates and object geometry must be authored independently. Content schema version is independent of display scale.

## Security/offline

Offline save journal or state snapshots at safe boundaries, with version checks and content checksums; atomic write with backup; no gameplay-breaking login requirement. Never download original commercial archive through the app. Optional local profile, parental/children-friendly modes; any future leaderboard is separately opt-in.

## Quality standards

Perform golden-master gameplay regression, Android emulator test, touch-only real-device test, closed/open fold test, screen rotation, process death/resume, low-memory, audio focus, accessibility and 18-hole session endurance.
