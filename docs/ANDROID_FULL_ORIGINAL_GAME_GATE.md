# Sensi Golf Android — FULL ORIGINAL GAME acceptance contract

**Product definition (locked 2026-10-09):** A complete, faithful, **fully playable original Sensible Golf Android release (prefer original DOS executable under an Android emulator; Windows v1.014 is an alternative)**, not a physics demo, terrain colour preview, substitute practice hole, partial port or proof-of-concept. User acceptance is on actual phones, a Fold closed/open and a tablet. No partial implementation may be called **full**, **100%**, **finished** or **ready for user gameplay testing**.

## Original content, rights and distribution

The goal is a **self-contained installer**, containing all original 25-course game content and its 72 distinct hole designs, as verified against the exact edition, original visual layers/sprites (optionally enhanced), audio, interfaces and course metadata. Do **not** put commercial assets in a public repo, GitHub artifact or APK unless the rights holder has granted redistribution/derivative-port permission (record the licence/permission and distribution scope). The user's reported ability to obtain a licence is not proof of permission.

A user-owned original game import may be maintained as an optional privately exercised engineering validation path, but **manual EPF extraction is not the user-facing product** and cannot satisfy full-release acceptance. Do not replace copyrighted content with a practice course or synthetic geometry and call it the complete original. A proposed first-party licence is the preferred path for the self-contained commercial remaster.

## Full-game matrix — each item needs tested PASS evidence

| Gate | Required behaviour | Actual state on 2026-10-09 |
| --- | --- | --- |
| RIGHTS | Permission to bundle original course geometry/graphics/audio and ship mobile remaster | NOT VERIFIED / BLOCKED |
| CORE | Preserve reference original flight, putting, hazard, scoring, PRNG, collision, tick fidelity | Verified principal branches and real 18-hole solver run; incomplete branches remain |
| FULL_RULES | Every original shot outcome, extreme accuracy miss, code9/10 event11 and all terrain states, no undocumented gate/stop | NOT COMPLETE |
| COURSES | All original **25 courses** and **72 distinct hole designs** (verify edition-specific asset mapping), starting positions, greens, hazards, tee/cup geometry, round order and par; usable by a human | Single 18-hole solver fixture PASS only; full course/mode coverage NOT COMPLETE |
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

## Port-vs-rewrite architectural correction (2026-10-09)

**Preferred route: wrap/run the actual game executable rather than reconstruct it.** The validated original files include `GOLFDOS.EXE` and `GOLFWIN.EXE`, plus `GOLF.EPF`. First test the exact DOS executable in desktop DOSBox then DOSBox Pure/Android to verify sound, menus, full career/season, 25 courses, competition formats, local multiplayer, save/load and performance. DOSBox already has a community compatibility listing for Sensible Golf; that is evidence of viability, not proof our exact edition runs perfectly.

Second route: run the verified Windows v1.014 executable through a suitable Android Wine/x86 compatibility layer (e.g. Winlator) if DOS edition differs. Confirm Windows API, 32-bit dependencies, display, audio and touch timing experimentally. Both approaches run the original game's own code and assets, so preserve all modes and original physics by construction when compatible.

Once runtime verified, **package an app-specific frontend**: bundled legally licensed original files, dedicated launcher, fullscreen 4:3 / aspect-accurate viewport, phone/Fold/tablet adaptation, touch aim and click mappings, controller support, Classic Pixels / optional Enhanced scaling, menu and save path integration. Avoid simulating or recreating content the original executable already provides. Emulator open-source licences (DOSBox Pure core GPLv2, and any chosen frontend) and original game distribution rights require separate review before publishing.

The recovered C++ engine remains a **fallback and test oracle**, not the primary porting route. Stop prioritising reconstruction of original menus, graphics and rules while direct executable compatibility remains untested.

## Original direct-runtime acceptance prerequisites

1. Confirm original DOS game launches from the complete game installation and archive (no manual per-hole EPF extraction); record hashes.
2. Verify all 25 courses and 72 unique hole designs, menu navigation, career/season/tournament options, single-player, local multiplayer, save/load, sound and music in a desktop emulator.
3. Repeat under Android runtime at actual screen sizes; verify game timing and touch mapping without mouse/keyboard.
4. Create dedicated wrapper APK only after original runtime launch is established; never call a successful Gradle build a full playable game.
5. Secure rights to bundle the original game content before shipping a self-contained distributable APK.

## Previously planned source-reconstruction pipeline (fallback only)

1. Inventory all 277 original `GOLF.EPF` entries and graphical/audio/script/menu dependencies; build original archive decoding parity tests (no asset committed publicly).
2. Implement full original-background/tile/golfer/sprites/flag/ball/sound pipeline in the Android host, rather than drawing a terrain descriptor colour map.
3. Tie camera world projection/green zoom to original captured reference images; mark measured dimensions, don't guess.
4. Replace approximate test Welly meter with original scheduling/animation, including untested accuracy-edge branches.
5. Integrate exact code9/code10 event11 continuation and remaining terrain, scoring, input and PRNG cases with original-oracle golden masters. Do not guess original state changes.
6. Recreate complete original menu navigation, settings, players, scoreboards, end-of-hole and end-of-round behaviour from the original executable.
7. Verify the complete **25-course / 72-hole-design** inventory against the actual game, every original competition mode, and original course-selection transitions. Test every course and mode on emulator, folded phone, opened Fold and tablet.
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

**FULL RELEASE = NO** until every gate above passes. A **private proof of concept is authorised to proceed without commercial release clearance**, using the operator's original copy locally; public redistribution of the original game's protected contents remains a separate permission question. Every test build must state its incompleteness in the name, readme and release description. Stop suggesting practice courses as a replacement for the original.
