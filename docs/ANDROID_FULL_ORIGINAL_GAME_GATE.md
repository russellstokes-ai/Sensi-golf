# Sensi Golf Android — FULL ORIGINAL GAME acceptance contract

**Product definition (locked 2026-10-09):** A complete, faithful, **fully playable Sensible Golf Windows v1.014 Android port**, not a physics demo, terrain colour preview, substitute practice hole, partial port or proof-of-concept. User acceptance is on actual phones, a Fold closed/open and a tablet. No partial implementation may be called **full**, **100%**, **finished** or **ready for user gameplay testing**.

## Original content, rights and distribution

The goal is a **self-contained installer**, containing all original 18 playable courses, original visual layers/sprites (optionally enhanced), audio, interfaces and course metadata. Do **not** put commercial assets in a public repo, GitHub artifact or APK unless the rights holder has granted redistribution/derivative-port permission (record the licence/permission and distribution scope). The user's reported ability to obtain a licence is not proof of permission.

A user-owned original game import may be maintained as an optional privately exercised engineering validation path, but **manual EPF extraction is not the user-facing product** and cannot satisfy full-release acceptance. Do not replace copyrighted content with a practice course or synthetic geometry and call it the complete original. A proposed first-party licence is the preferred path for the self-contained commercial remaster.

## Full-game matrix — each item needs tested PASS evidence

| Gate | Required behaviour | Actual state on 2026-10-09 |
| --- | --- | --- |
| RIGHTS | Permission to bundle original course geometry/graphics/audio and ship mobile remaster | NOT VERIFIED / BLOCKED |
| CORE | Preserve reference original flight, putting, hazard, scoring, PRNG, collision, tick fidelity | Verified principal branches and real 18-hole solver run; incomplete branches remain |
| FULL_RULES | Every original shot outcome, extreme accuracy miss, code9/10 event11 and all terrain states, no undocumented gate/stop | NOT COMPLETE |
| COURSES | All original 18 holes, correct original data, starting positions, greens, hazards, tee/cup geometry, round order and par; usable by a human | Continuous solver fixture PASS; human/device coverage NOT COMPLETE |
| ART | Original course graphics layers and golfer/ball/flag/tile animation decoded/rendered and correctly registered to original collision/world coordinate camera | NOT IMPLEMENTED; test APK uses coloured collision raster |
| CAMERA | Fidelity-verified scroll, zoom/scale, ball tracking, framing and correct pixel/world transform | NOT VERIFIED; test values are placeholders |
| INPUT | Exact original meter logical timing, full original accuracy/miss sweep, 13 clubs, touch aim/shot/cancel/putt; optional controller | Three-click bridge passes tests; actual UI approximate meter and incomplete miss inputs |
| MENUS | All original startup screens, mode selection, player options, scoring/scorecards and round-end game flow | NOT COMPLETE |
| AUDIO | Original licensed music/SFX, timing tied to shot/game events, pause/resume/focus | NOT IMPLEMENTED |
| COMPLETE_PLAY | Finish all 18 holes using human touch input and full UI without imported test harness, unsupported-state dead ends or placeholder shots | NOT VERIFIED |
| DEVICES | Android ordinary phones, folded/unfolded Fold, tablets, portrait/landscape (adaptive) | Layout smoke test PASS; device/emulator gameplay tests NOT VERIFIED |
| GRAPHICS_OPTIONS | Menu Classic Pixels and optional Enhanced 2× filtering, consistent game art without altering collision | Temporary terrain-raster mode present; original artwork absent |
| RESUME | Pause, process death, save/resume, original course state/stroke and score preserved, no duplicate strokes | NOT VERIFIED on device |
| QA | Android emulator automation and physical test logs, performance, stability, all parity suites PASS, distribution smoke test | Build CI PASS, no end-to-end real-device proof |
| PACKAGE | A uniquely named, self-contained APK with assets and legally cleared distribution, installs and launches into the actual full game | NOT COMPLETE |

No isolated "18/18 solver works" result, no green Gradle build, and no synthetic practice hole may override a NOT COMPLETE gate.

## Required original resource/product pipeline

1. Inventory all 277 original `GOLF.EPF` entries and graphical/audio/script/menu dependencies; build original archive decoding parity tests (no asset committed publicly).
2. Implement full original-background/tile/golfer/sprites/flag/ball/sound pipeline in the Android host, rather than drawing a terrain descriptor colour map.
3. Tie camera world projection/green zoom to original captured reference images; mark measured dimensions, don't guess.
4. Replace approximate test Welly meter with original scheduling/animation, including untested accuracy-edge branches.
5. Integrate exact code9/code10 event11 continuation and remaining terrain, scoring, input and PRNG cases with original-oracle golden masters. Do not guess original state changes.
6. Recreate complete original menu navigation, settings, players, scoreboards, end-of-hole and end-of-round behaviour from the original executable.
7. Implement all original 18 courses, hole traversal, complete UI and actual touch playtests. Test every hole on emulator, folded phone, opened Fold and tablet.
8. Capture and restore original state reliably across rotation/Fold changes/background/process death.
9. License verification, content packaging, release APK, full-coverage acceptance sign-off.

## Separate optional modern features

- **Classic Pixels / Enhanced 2×** render settings are optional visual treatments, not alternative physics or course layouts.
- Smoothing and high-refresh display are strictly renderer-side; core logic remains tied to the recovered clock.
- Pixtee Golf is a **separate independent** project; its career/rewards/modified courses must not be silently mixed into original Sensible Golf's 100%-fidelity release.

## What exists already

- Original Windows v1.014 physics parity core and all-18-hole deterministic C++ replay.
- Real JNI/NDK Android binary with touch layout and collision-map diagnostic renderer.
- `SensiGolf-0.1.0-test-02` is **an incomplete engineering preview, not a full port**.
- See `docs/PROJECT_STATUS.md`, `docs/RECOVERED_PHYSICS_1_014.md`, `analysis/evidence/gate2_full_original_eighteen_hole_checkpoint.json` and `android/README.md`.

## Explicit completion statement

**FULL RELEASE = NO** until every gate above passes and original commercial asset distribution is lawfully authorized. Every test build must state its incompleteness in the name, readme and release description. Stop suggesting practice courses as a replacement for the original.
