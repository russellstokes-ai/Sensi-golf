# Sensible Golf — complete original Android shell (source under development)

**Non-negotiable product goal:** Run the **entire original Sensible Golf** with its original course selection, season/tournaments, menus, multiplayer where supported, graphics, audio, shot behaviour, scoring and saved games. Do **not** substitute a synthetic practice course or claim that one verified 18-hole round covers the full game. Target inventory reported for the original: 25 courses and 72 distinct hole designs; verify those counts against the exact original edition before release.

## Implementation strategy
- One installed Android app with an embedded **DOSBox Pure** (GPLv2-or-later) libretro core hosted by **LibretroDroid** (GPLv3), no separate emulator download, external app or setup screen.
- Starts an **internal archive containing exactly one original DOS executable** to bypass the DOSBox Pure startup chooser; verify this behavior using real Android gameplay video before claiming success.
- Original game executable stays authoritative for every ball bounce, meter, slope, shot, course, tournament and scoring rule; no C++ physics reconstruction runs here.
- One graphics style: original art scaled to device resolution with **fixed low-effort bilinear smoothing**, aspect ratio preserved. No Classic/Enhanced options, no graphics selection, no user-facing emulator text.
- Android fullscreen branded activity, touchpad-mouse forwarding, optional gamepad, Android pause/exit only, folds open/closed, phones and tablets.
- The **approved eagle/golfer launcher icon** is staged as a user-approved image in the conversation; drawable currently is a temporary build-only placeholder until that raster is shipped in the project. Never release with that placeholder.
- The first Android native-core debug APK must NOT be described as a playable full game until a real-content runtime and full-game UI smoke test is recorded.

## Rights and release

**Full distribution blocked until verified licence** to distribute original code, EPF, artwork and audio as a complete new Android app. Do not fetch abandonware/Archive.org copies into a publicly distributed artifact in lieu of permission. Runtime GPL licences additionally require appropriate source/notice compliance.

Licensed packaging (once authorized): `tools/build_original_dosz.py` validates the exact original executables and EPF, creates `app/src/main/assets/game/SensibleGolf.dosz` with only one DOS executable, and the Android Gradle build includes it. No manual extraction on the phone. This file is deliberately gitignored. A Gradle release build without it intentionally fails.

## Source pinning

- DOSBox Pure: `libretro/dosbox-pure` commit `cdaea10ce8cac231bc341ca05d4c5e4e5fe0fbd5`.
- LibretroDroid: `Swordfish90/LibretroDroid` commit `8835c3098514390a271e36983957f7bb5f40abf1`.
- Both are cloned into local `vendor/` only during CI; note GPL obligations.

## Not yet achieved

- Actual complete-original DOS runtime game boot proven on Android
- Touch accuracy/left click timing on a real closed/open Fold or tablet
- No-start-menu assertion proven from captured video rather than the upstream single-executable documentation
- Approved launcher art committed to packaged APK
- 25-course navigation/season/multiplayer/music/stability validation
- Original distribution rights

CI is allowed to produce **build-verification evidence only**, not a falsely labelled full-game release APK.
