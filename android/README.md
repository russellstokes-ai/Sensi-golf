# Sensi Golf — original-engine Android touch preview

> **NOT THE FULL GAME.** This is only the historical Test 02 engineering artifact. The user requires the **complete original Sensible Golf Android port**: original licensed courses/art/music/SFX, full game rules, menus, modes, scoring, 18-hole human gameplay and phone/Fold/tablet support. Do not describe this preview as a completed or playable full port. See [Full Original Android Game Gate](../docs/ANDROID_FULL_ORIGINAL_GAME_GATE.md). The released game must not demand manual EPF extraction or use a substitute practice course.

This is the real recovered `sensigolf_core` compiled via Android NDK and controlled with on-screen controls. This is a **test harness APK**, not a final art-complete port.

It contains **no commercial original assets**. On first launch the user must select a folder containing legitimately obtained and extracted original Windows v1.014 EPF resources. At minimum: `MAPI01.RAW`, `MAPI02.RAW`, `MAPM42.MAP`, `MAPM42.SPT`, `MAPS42.MAP` (and analogous resources for subsequent holes). The UI imports by Android Storage Access Framework; it does not fetch or redistribute original content.

**Known fidelity boundary:** touch layout and map colours are temporary debugging visuals, not recovered original artwork or measured original camera scale. Original meter extreme-accuracy behaviour and code9/10 special-green continuation remain incomplete; the test meter limits its accuracy sweep to the previously validated range rather than fabricating missing physics. The original C++ simulation is unchanged.

Android debug build uses Gradle 8.9 / Android Gradle Plugin 8.7.3 / SDK35 / pinned NDK 27.2 / CMake 3.22.1.

Install the unique APK from the GitHub workflow artifact; it is `app-debug.apk` internally, with distinct artifact name and package `com.russellstokes.sensigolf.preview`.

## Screen/device and graphics test matrix

- Phone portrait or folded Fold: compact bottom dock; phone landscape: compact sidebar.
- Open Fold and landscape tablet: expanded side controls and course view; tablet portrait uses bottom dock.
- All primary touch targets >=48dp. Screen orientation and Fold resize keep the same Activity and native game session.
- In menu: Classic Pixels (nearest neighbour) / Enhanced 2x (bilinear upscaled course test raster); stored locally.
- This build does **not** yet render original sprites and scenery; it colours the course from original terrain collision data. Actual hardware Fold and tablet testing is still required.
- Import extracted original course resources using Android's folder picker. No content is supplied in the APK.

## Original archive decoding status

The native-original archive extraction problem was already solved for the reference game: `tools/epf_inspect.py` reads the EPFS container table, and `tools/epf_extract.py` decompresses its 9–14-bit LZW format. `docs/PROJECT_STATUS.md` records **277/277 `GOLF.EPF` entries extracted** during engine recovery. This does not mean graphics/audio decoding, licensing or bundling are finished. Future licensed full-game packaging should consume the existing extraction pipeline and bundle approved, ready-to-use resources as part of the installer. A manual extraction flow is not acceptable for the finished game.
