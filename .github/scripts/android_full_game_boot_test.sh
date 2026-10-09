#!/usr/bin/env bash
# A single bash file is essential: android-emulator-runner@v2 runs each line
# of its "script:" argument separately via /usr/bin/sh.
set -euo pipefail

APK="android-full/app/build/outputs/apk/debug/app-debug.apk"
OUT="analysis/private/android-full-boot"
PKG="com.russellstokes.sensigolf"
ACT="$PKG/com.russellstokes.sensigolf.fullgame.FullGameActivity"
mkdir -p "$OUT"

# ALWAYS preserve the underlying Android failure. The previous test stopped
# after a failed pidof call eight seconds into boot and lost the exception.
# Capture logcat, activity/process state, and a last screenshot before exit.
boot_failure_diagnostics() {
  local status=$?
  trap - ERR
  echo "::error::Android original-game boot gate failed (exit ${status}); collecting underlying runtime diagnostics."
  adb logcat -d -v threadtime -t 2200 > "$OUT/boot-failure-logcat.txt" 2>&1 || true
  adb shell dumpsys activity activities > "$OUT/boot-failure-activity.txt" 2>&1 || true
  adb shell dumpsys activity processes > "$OUT/boot-failure-processes.txt" 2>&1 || true
  adb shell pidof "$PKG" > "$OUT/boot-failure-pid.txt" 2>&1 || true
  adb exec-out screencap -p > "$OUT/boot-failure-screen.png" 2>/dev/null || true
  echo "--- Relevant Android fatal/native/runtime lines ---"
  grep -Ei -B 6 -A 25 "FATAL EXCEPTION|Fatal signal|Process: $PKG|UnsatisfiedLinkError|AndroidRuntime|linker.*(failed|cannot)|SIGSEGV|SIGABRT|libretrodroid|dosbox_pure|Core could not be loaded|failed to load core" "$OUT/boot-failure-logcat.txt" | tail -n 240 || true
  echo "--- Activity/process state ---"
  grep -Ei -m 18 "mResumed|ResumedActivity|mCurrentFocus|$PKG|crash|ANR" "$OUT/boot-failure-activity.txt" || true
  echo "All forensic output retained in original-full-game-android-poc-boot-evidence."
  exit "$status"
}
trap boot_failure_diagnostics ERR

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

# Once the attract/demo course is active, Escape should return control to
# the original title/menu if that command is supported. Record actual result.
adb shell input keyevent 111
sleep 4
adb exec-out screencap -p > "$OUT/after-demo-escape-143s.png"
test -s "$OUT/after-demo-escape-143s.png"
adb shell input keyevent 62
sleep 6
adb exec-out screencap -p > "$OUT/after-demo-space-149s.png"
test -s "$OUT/after-demo-space-149s.png"

# The 1995 executable's intro/demo length fluctuates between emulator boots.
# One passing run reached the real menu at 149s; another was still in Demo Mode.
# Never infer menu presence from elapsed seconds or from changing frames.
menu_found=0
# Prior failure 37996364852 was a FALSE NEGATIVE: its last screenshot
# (menu-probe-30) visibly captured the original title/button menu during
# a black fade. The test stopped precisely as the menu emerged.
# Allow 60 checks; during the second half stop injecting SPACE/ESC so a
# successfully reached menu can settle instead of launching another demo.
for attempt in $(seq 1 60); do
  probe="$OUT/menu-probe-$(printf '%02d' "$attempt").png"
  adb exec-out screencap -p > "$probe"
  test -s "$probe"
  if python tools/detect_sensible_main_menu.py "$probe"; then
    # Real game behavior: run 37997727471 captured a COMPLETE bright main
    # menu at probe-46, but only 3 seconds later Demo Mode had resumed.
    # Requiring two stable frames incorrectly FAILS the full original game.
    # Grab the first strongly matching real menu frame and click PLAY ROUND
    # immediately, before the title transitions back into attract mode.
    cp "$probe" "$OUT/original-main-menu-confirmed.png"
    echo "CONFIRMED: actual original Sensible Golf selectable main menu frame."
    menu_found=1
    adb shell input tap 1260 390
    sleep 1
    adb exec-out screencap -p > "$OUT/after-immediate-play-round-tap.png"
    test -s "$OUT/after-immediate-play-round-tap.png"
    break
  fi
  if (( attempt <= 30 )); then
    if (( attempt % 2 )); then
      adb shell input keyevent 111  # ESC; may exit Demo Mode
    else
      adb shell input keyevent 62   # SPACE; original menus
    fi
  fi
  sleep 4
done
if [ "$menu_found" -ne 1 ]; then
  echo "::error::Original DOS app ran but stable main menu was not detected after 60 probes."
  exit 1
fi

# Only NOW test touch on the proven original menu. In touchpad mode, a tap
# may click at the current cursor rather than jump to absolute screen coords.
# These frames diagnose actual behaviour; their mere existence does not claim
# that Play Round was selected or that a human round is playable.
adb shell input tap 1260 390
sleep 4
adb exec-out screencap -p > "$OUT/after-confirmed-menu-touch.png"
test -s "$OUT/after-confirmed-menu-touch.png"
python tools/detect_sensible_main_menu.py "$OUT/after-confirmed-menu-touch.png" || true

adb shell input swipe 1200 540 1260 390 650
sleep 2
adb exec-out screencap -p > "$OUT/after-confirmed-menu-drag.png"
test -s "$OUT/after-confirmed-menu-drag.png"
adb shell input tap 1260 390
sleep 5
adb exec-out screencap -p > "$OUT/after-confirmed-menu-drag-tap.png"
test -s "$OUT/after-confirmed-menu-drag-tap.png"

# Keyboard ENTER is a diagnostic CONTROL, not a substitute for native touch.
adb shell input keyevent 66
sleep 5
adb exec-out screencap -p > "$OUT/after-confirmed-menu-enter.png"
test -s "$OUT/after-confirmed-menu-enter.png"

python - "$OUT" <<'PY'
import hashlib,json,sys
from pathlib import Path
root=Path(sys.argv[1])
names=["intro-8s.png","after-escape-11s.png","after-space-16s.png",
       "intro-32s.png","intro-80s.png","intro-128s.png",
       "after-tap-131s.png","after-tap-139s.png",
       "after-demo-escape-143s.png","after-demo-space-149s.png",
       "original-main-menu-confirmed.png","after-confirmed-menu-touch.png",
       "after-confirmed-menu-drag.png","after-confirmed-menu-drag-tap.png",
       "after-confirmed-menu-enter.png"]
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

echo "ANDROID MENU BOOT PASS: captured original full main-menu frame and immediate Play Round tap; saved interaction screenshots."
echo "NOTICE: Menu frame and touch dispatched are verified, but the resulting game-mode selection, audio and all-course completion remain separate gates."
