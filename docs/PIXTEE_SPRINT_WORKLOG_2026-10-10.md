# Pixtee Golf — engineering handover / acceptance log (10 October 2026)

## Release policy

**NO TEST APK / NO PUBLIC RELEASE YET.** The October 10 v0.1.1 artifact was an unverified framework and failed basic phone usability; do not refer to it as a game or distribute it. This branch is a draft until the actual Android emulator and full game acceptance gates pass. The GitHub Actions development workflow now builds internally and publishes diagnostics, not downloadable preview APKs.

Active branch: `feat/pixtee-native-portrait-framework`, PR #11.
The original Sensi Golf restoration in `main` must remain untouched.

## Product contract (locked)

An independent, original-art portrait Android pixel-golf game closely matching the **observable** classic Sensible Golf gameplay: straight-down view, tiny golfer/ball/course scaling, 13 clubs, familiar aim/shot response, Welly-o-meter semicircular power and red-zone strike timing. World is never stretched to fill phone aspect ratios. The game's graphics may have mildly smoother animation, plus small swaying flowers/grass, animated water, occasional spectator poses and non-colliding sponsor boards near tees and greens. Different authored courses, original branding and mobile button layout. No intrusive advertising.

Also required before player acceptance: full course roster target matched after reference verification; 3/9/18-hole rounds, modes, five-tier career, save/resume, statistics, achievement rewards, cosmetic unlocks, accessible/persistent settings, full fold/phone coverage, original-only asset provenance, sound, device tests and balanced gameplay.

Do not import proprietary game code, maps, artwork, samples or course data without proof of the necessary rights.

## Work recorded in this chat (implementation not equivalent to signoff)

### S1 — Touch architecture, screen navigation and Android tests
- New `MenuInput.kt`: keeps logical drawing and touch coordinates aligned after scrolling, distinguishes drag/swipe from click; supports shorter Fold portrait viewports.
- Reworked `PixteeActivity.kt` event routing to use actual on-screen button rectangles, screen-based handlers and accessibility state descriptions. Existing 1-hole start path remains MAIN -> COURSES -> PLAYER -> TEE -> PLAYING.
- Unit tests for scrolling, hit mapping and gesture cancellation.
- Added `androidTest` UiAutomator smoke tests for menus, start and the first swing. Added an Android emulator GitHub Actions job.
- **Evidence:** an early actual emulator run (workflow 38055521511) passed OPTIONS but failed navigating COURSES -> PLAYER and CAREER -> MAIN. This must be investigated, not dismissed. Test coordinates were updated to reference the actual Canvas bounds and now log expected/actual screen on failure. No device signoff until green repeatedly.

### S2 — Ambient course art
- Deterministic 8-frame, optional motion cycle in `CourseAmbient.kt` with unit tests.
- Animated pixel-blue water ripples, light one-pixel swaying grass, small flowers, two-pose idle spectators and optional motion setting in Options.
- Course signs already occupy unique tee/green world positions; they must stay visual and never touch collision code.
- All ambience is calculated only in the Canvas and does not alter the physics tick, ball coordinates, scoring or RNG.

### S3 — Welly-o-meter
- Original manual confirms an **arched semicircular** indicator, high-arc power and lower red-zone timing; the horizontal bar from the earlier APK was wrong.
- Current `drawMeter` uses an independent pixel-art semicircular design. `PixteeCore` now carries the selected power position into the returning downswing, whose timing changes with power, rather than resetting to another bar.
- Added tests for continuity, straight-red-zone timing and missing the zone.
- **Still unverified:** actual colours, arcade tick timing, timing window, power-to-distance calibration and swing-animation parity against footage/reference.

### S4 — Actual rolling and bounce foundation
- Replaced the former stationary rolling delay with fixed-step ball movement, lie-dependent friction, water check while rolling and a bounded roll time.
- Putter moves on the ground. Woods/irons keep airborne stage then roll. Ball lift now renders the small bounce rather than an identical stationary circle.
- Added deterministic unit tests for actual rolling and ground putts.
- **Still unverified:** exact original physics, obstacles/trees/green slope and shot traces. These remain major gates, not solved by this approximation.

## Essential backlog before a polished APK

1. **Emulator gate:** fix ALL native navigation test failures; add short viewport scroll device test, full touch / power cycle; test portrait phone and Fold configurations. Repeat until reliable, not only build green.
2. **Arcade gameplay parity:** independently calibrate 60Hz-ish tick, visual Welly zones/timing, original-feeling club trajectory, bounce/roll, putt slopes, wind-off, trees/obstacles, hazards, ball and golfer scale. Do not label exact parity without evidence.
3. **Full match/session:** actual 3/9/18-hole game progression, per-hole par and tees, hazard drops, scoring, next-hole resources, quit/resume, no stuck terminal state.
4. **Content:** complete varied original hole/course layouts, 25-course target only after parity/reference validation; no placeholder names or locked boxes in final UI.
5. **Career & commercial:** all agreed career tiers, tournaments and rewards, high-quality pixel sound/animation, sponsor inventory/sales back office with rights and child-suitability controls.
6. **Final acceptance:** automated seeded full rounds, input/UI screenshot review, stability/crash and performance tests, option persistence, accessibility/reduced-motion, Android portrait phone and Fold closed/open, copyright/provenance review. Only then export a named, production-version APK.

## Test evidence conventions

Record every reported claim with commit SHA, run ID, green/failing tests, screen size, actual screenshots/video and whether a **real emulator or hardware** was involved. JVM tests and successful Gradle `assembleDebug` alone cannot establish gameplay. While tests are failing, status is **red/blocked**, even if packaging succeeds.

## Latest native-device evidence — subsequent commits

- The previously persistent emulator navigation failures came from Android's **ImmersiveModeConfirmation** system window overlay, which intercepted screen touches and set the focused package to `android` despite the game remaining underneath. The test-only emulator setup now sets `settings put secure immersive_mode_confirmations confirmed` before launching the game.
- **Green baseline:** commit `c6712cb29c85dc335e1bf17891e53c5cc8393c19`, [Actions 38059603934](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/38059603934): `build=success`, `Android emulator interactive gameplay smoke tests=success`, **3 of 3 Android tests passed** (menu/options navigation and full three-press shot). This proves the tested paths, not full commercial readiness.
- Original numerical research documents confirm approximately **70 Hz** original gameplay clock and important lie/putt/meter behaviour. Pixtee currently uses an independently authored **60 Hz** approximation; exact shot parity is not yet shown.
- Follow-up commits corrected maximum Welly power to the **top** of its semicircle, visibly show negative-side late hits, varied the red zone by lie and added a fourth real emulator test for scrolling to an offscreen button in a compact portrait/Fold-like viewport.
- The follow-up four-test Android gate has to pass before the new Welly/Fold code is accepted. Do not describe the latest development commit as hardware-validated until that result is available.
- The game still only contains one authored hole, and placeholder career/course screens are NOT finished features. The no-APK-until-polished release rule remains in force.
