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

# Probe classic DOS splash skip keys through the Android->libretro key bridge.
# The emulator menu is disabled; these must go to the actual game executable.
adb shell input keyevent 111  # ESCAPE
sleep 3
adb exec-out screencap -p > "$OUT/after-escape-11s.png"
test -s "$OUT/after-escape-11s.png"
adb shell input keyevent 62   # SPACE
sleep 5
adb exec-out screencap -p > "$OUT/after-space-16s.png"
test -s "$OUT/after-space-16s.png"

# Original baseline snapshot at approximately 32 seconds, after skip-key test.
sleep 16
adb shell pidof "$PKG" | tee "$OUT/later-pid.txt"
adb exec-out screencap -p > "$OUT/intro-32s.png"
test -s "$OUT/intro-32s.png"

# The original intro can last substantially longer under software rendering.
# Verify later progression and one *real* Android touchscreen action rather
# than interpreting a live process as a playable full game.
sleep 48
adb shell pidof "$PKG" | tee "$OUT/pid-80s.txt"
adb exec-out screencap -p > "$OUT/intro-80s.png"
test -s "$OUT/intro-80s.png"

sleep 48
adb shell pidof "$PKG" | tee "$OUT/pid-128s.txt"
adb exec-out screencap -p > "$OUT/intro-128s.png"
test -s "$OUT/intro-128s.png"

# Touch the game's visible centre inside the 4:3 fitted viewport; if the
# intro is skippable or menu is displayed, this exercises core pointer wiring.
# adb emulates a screen tap; it is NOT proof of real-finger timing/accuracy.
adb shell input tap 1200 540
sleep 3
adb exec-out screencap -p > "$OUT/after-tap-131s.png"
test -s "$OUT/after-tap-131s.png"
adb shell input tap 1200 540
sleep 8
adb exec-out screencap -p > "$OUT/after-tap-139s.png"
test -s "$OUT/after-tap-139s.png"

python - "$OUT" <<'PY'
import hashlib,json,sys
from pathlib import Path
root=Path(sys.argv[1])
names=["intro-8s.png","after-escape-11s.png","after-space-16s.png",
       "intro-32s.png","intro-80s.png","intro-128s.png",
       "after-tap-131s.png","after-tap-139s.png"]
hashes={name:hashlib.sha256((root/name).read_bytes()).hexdigest() for name in names}
(root/"visual-progression-sha256.json").write_text(json.dumps(hashes,indent=2)+"\\n")
assert len(set(hashes.values()))>1,"All game frames identical; no visual progress"
print("GAME VIDEO FRAMES CHANGED between samples (does NOT establish menu or play).")
PY

adb shell dumpsys activity activities > "$OUT/activity-state.txt"
grep -q "$PKG" "$OUT/activity-state.txt"
adb logcat -d -s AndroidRuntime:E libretrodroid:E > "$OUT/android-errors.txt"

if grep -Eq 'FATAL EXCEPTION|UnsatisfiedLinkError|Unable to start activity' "$OUT/android-errors.txt"; then
  cat "$OUT/android-errors.txt"
  exit 1
fi

echo "ANDROID ACTIVITY SMOKE PASS: process remains alive; original game launch screenshots captured."
echo "NOTICE: Activity + changing frames + simulated tap are not evidence of playable menus, original input, audio or 25-course coverage."
