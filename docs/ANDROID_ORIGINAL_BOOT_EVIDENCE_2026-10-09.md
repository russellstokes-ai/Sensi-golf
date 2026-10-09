# Original Sensible Golf Android executable — visual proof and remaining gates

**Date:** 2026-10-09. **Route:** Android app shell + LibretroDroid + DOSBox Pure running unmodified `GOLFDOS.EXE` with original `GOLF.EPF` inside a local/private APK. This is a technical POC, not a commercial release.

## Verified so far

1. [Full native-core Android build / boot run 37991123629](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37991123629): Android x86_64 emulator installed and launched APK, app process survived, AndroidRuntime had no fatal exception; screenshot at 8s shows **Sensible Software Presents**, screenshot at 32s shows **Published by**. No visible DOS/RetroArch startup menu.
2. [Extended boot run 37993060538](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37993060538): CI **PASS**. Captured screens at 8, 32, 80, 128 and after 131/139-second simulated taps. At 80s screenshot showed **East Point Software** original animated logo; 128s showed **Original Soundtrack JOPS**; at ~139s screen showed **Geneva / Hole 1 / Par 3 / Demo Mode**, including the original golfer, course scenery, 4-player scoreboard, distance/club indicators and terrain. This is genuine original game content rendering, not the recovered C++ proof-of-concept collision-colour preview.
3. The Android screen uses 4:3 preserved game projection with margins and no emulator user interface. The fixed original art bilinear output is part of the Android host. Fold/tablet viewport computation has separate unit tests, not yet physical-device proof.

## Important limitations — NOT yet demonstrated

- Not proven: playable menu, original keyboard/mouse button dispatch, user-operated club selection and three-click swing, 25-course navigation, season/tournament modes, music audible on hardware, save/resume, local multiplayer, Fold rotation across active game.
- The screenshot says **Demo Mode**. It demonstrates original game engine/graphics *attract-mode execution*, **not** a user-controlled completed round.
- Original startup reaches demo only after roughly 139 seconds in the tested software-rendered x86_64 environment. This is far too slow for a polished Android app and may be due to intro defaults/emulator slowdown; no causal diagnosis confirmed yet.
- A valid original game ZIP is built privately inside CI for compatibility evidence, without adding its copyrighted payload to git. CI intentionally does not upload a redistributable public game APK.

## Corrective experiments now pending

- Android Activity explicitly forwards keyboard/controller **down AND up** to `GLRetroView.sendKeyEvent` (previously only Android Back and Escape were intercepted by the host); Android Back remains a pause action, Escape reaches the DOS game.
- Extend Android screenshot automation to press ESC and Space at 8/11 seconds and again during original attract/demo mode (~139 seconds), capturing intermediate frames. The objective is to prove intro skip and original main-menu/input transitions.
- After the original menu is visible, automate touch interaction with the original settings/course selection, a 3-click playable shot, scorecard and 18-hole continuation; then validate all original course/mode inventory.

## Repeatable proof locations

- `.github/workflows/sensigolf-full-android-shell.yml` — pinned native runtime and x86_64 emulator test.
- `.github/scripts/android_full_game_boot_test.sh` — screenshot/liveness/touch/keyboard validation.
- `tools/build_original_dosz.py` — operator/local complete-original DOS archive packager.
- `android-full/app/src/main/java/com/russellstokes/sensigolf/fullgame/FullGameActivity.kt` — original executable host and no-settings fixed smoothing.

**Release statement: not finished.** All original assets/modes and control fidelity need to be verified before presenting this to the user as a complete playable Android port. One screenshot of a demo hole is a genuine technical milestone, not 100% product readiness.
