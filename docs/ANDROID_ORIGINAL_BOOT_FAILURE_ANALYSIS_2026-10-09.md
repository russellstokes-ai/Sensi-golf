# Original Sensible Golf Android — failure analysis and verification handover

Status: **actual original DOS gameplay footage captured in Android emulator; stable original menu/touch-to-round not yet proven.** This is *not* a full-release sign-off.

## What the user expects

- A **single native-feeling Android APK**, original full game with all its courses, original menus, modes, graphics, physics, sound and score flow.
- No separate emulator app, DOS prompts, emulator setup controls, user EPF extraction, synthetic course/demo substitute, graphics picker or test labels in shipped app.
- Approved eagle-and-golfer Android launcher icon, one fixed gentle upscaling, aspect-preserved full-screen phone/Fold/tablet.
- Private proof-of-concept compatibility investigation now; public redistribution of original copyrighted game files is a separate question.

## Root-causes of repeated CI failures (do not confuse)

1. [Run 37989285921](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37989285921): **test harness failure** — `reactivecircus/android-emulator-runner` executed raw script under `/usr/bin/sh`, but inline `set -o pipefail` needs Bash. Fixed by delegating the entire boot test to `bash .github/scripts/android_full_game_boot_test.sh`.
2. [Run 37996345616](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37996345616): Android launcher returned status OK and an activity component name, but the subsequent 8-second `pidof` command returned exit 1 and terminated early. **Cannot say the game crashed from this alone:** that run did not retain enough process/logcat forensic data. A boot failure handler was committed to collect `logcat`, activity/process state, current PID and last screenshot on *every* error.
3. [Run 37996364852](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37996364852): **real original game runs**. All 30 `menu-probe` screenshots saved, 1990s original menus/gameplay/hazards/courses visible in demo mode. In its **final screenshot `menu-probe-30.png` the original "Sensible GOLF" logo and six selectable brown buttons appear**, but at very low brightness during a fade-in. This is direct evidence the original main menu was reached and the detector made a false-negative.
4. [Run 37997475301](https://github.com/russellstokes-ai/Sensi-golf/actions/runs/37997475301): diagnostic rerun still had the previous 30-probe cutoff and again failed its menu gate after demo progressed; it **did not** establish a program or build failure.

## Preventing another false negative

- Updated `.github/scripts/android_full_game_boot_test.sh`: 60 menu probes, with the first 30 alternately sending ESC/SPACE, and the following probes observing without pressing buttons so a just-arriving menu is not inadvertently dismissed. A candidate menu requires **two screenshots 3 seconds apart**, not one transient intro frame.
- Updated `tools/detect_sensible_main_menu.py`: the original detector uses a brown-button high-ratio and low-green condition in a fixed, known 2400x1080 Android emulator crop. On dim-fade frames only (95th-percentile sample peak between 7 and 59), it normalizes brightness *in the analysis*, not in the displayed game. The captured previously rejected `menu-probe-30.png` goes from brown fraction ~0.000, green 0.000 to normalized **brown ~0.199, green 0.000**, exceeding the 0.18/0.15 threshold. Other captured demo frames retain green majority and remain negative.
- These scripts are only CI instrumentation. **They do not change the original physics, courses, graphics, sound, clock speed or UI.**

## Caution about apparent successes

Earlier Android build-verification runs passed compilation, boot and changing-frame checks, but **those checks did not prove the selectable main menu**. One previously passing screenshot (run 37995951786) still shows a visible `Demo Mode` banner after automated Enter. Never interpret its green job as proof of full playable original game.

Do not publish an APK labelled "full working Sensible Golf" merely because the native runtime loads and plays its attract demo.

## Next evidence gates (in order)

1. **Original menu:** two stable post-fade screenshot signatures and saved `original-main-menu-confirmed.png`.
2. **Android touch:** a touch action in menu that demonstrably selects a game mode; a changed screenshot alone is not enough if it may be an attract/demo frame.
3. **Playable original round:** select an original 18-hole round, hit three-click shots, finish a hole, see real scorecard and advance, preserve original physics.
4. **Whole game:** all 25 courses / 72 distinct hole designs (edition mapping verified), tour/season/other competitions, pause/save, audio, and final score flow.
5. **Devices:** Android ARM64 phone portrait/landscape, Fold closed/open resize mid-round, tablet, actual tap hit areas, timing, gamepad if used; no DOS/host visible.
6. **Packaging:** privately provision complete original game files before APK build. Never ship public unlicensed original commercial assets as a CI artifact. Test icon, fixed renderer smoothing and application identity.

## Reference material

- Source scripts: `.github/scripts/android_full_game_boot_test.sh`, `tools/detect_sensible_main_menu.py`.
- Full-game acceptance: `docs/ANDROID_FULL_ORIGINAL_GAME_GATE.md`.
- The output artifact `original-full-game-android-poc-boot-evidence` from run 37996364852 contains only screenshots and diagnostic metadata, **not original binaries**. Those images are not committed to the public repository.
- Separate C++ recovered engine remains intact as parity oracle, not the code powering the embedded original executable.
