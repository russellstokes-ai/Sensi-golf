#!/usr/bin/env bash
# A single bash file is essential: android-emulator-runner@v2 runs each line
# of its "script:" argument separately via /usr/bin/sh.
set -euo pipefail

APK="android-full/app/build/outputs/apk/debug/app-debug.apk"
OUT="analysis/private/android-full-boot"
PKG="com.russellstokes.sensigolf"
ACT="$PKG/com.russellstokes.sensigolf.fullgame.FullGameActivity"
mkdir -p "$OUT"

test -s "$APK"
adb install -r "$APK"

# Prevent the first-run Android immersive-mode tutorial from obscuring
# the original game in automated screenshots; not a game setting.
adb shell settings put secure immersive_mode_confirmations confirmed || true

adb shell am start -W -n "$ACT" | tee "$OUT/activity-launch.txt"
sleep 8
adb shell pidof "$PKG" | tee "$OUT/first-pid.txt"
adb exec-out screencap -p > "$OUT/intro-8s.png"
test -s "$OUT/intro-8s.png"

# Allow the original game's intro to reach its menu, without injecting inputs.
sleep 24
adb shell pidof "$PKG" | tee "$OUT/later-pid.txt"
adb exec-out screencap -p > "$OUT/intro-32s.png"
test -s "$OUT/intro-32s.png"

adb shell dumpsys activity activities > "$OUT/activity-state.txt"
grep -q "$PKG" "$OUT/activity-state.txt"
adb logcat -d -s AndroidRuntime:E libretrodroid:E > "$OUT/android-errors.txt"

if grep -Eq 'FATAL EXCEPTION|UnsatisfiedLinkError|Unable to start activity' "$OUT/android-errors.txt"; then
  cat "$OUT/android-errors.txt"
  exit 1
fi

echo "ANDROID ACTIVITY SMOKE PASS: process remains alive; original game launch screenshots captured."
echo "NOTICE: these checks do NOT automatically verify that the main menu was reached or that all 25 courses work."
