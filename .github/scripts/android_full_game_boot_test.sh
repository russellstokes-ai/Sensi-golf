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

# Regression: rerun 38002246293 reached ORIGINAL Player Select, but the
# old test mistakenly kept searching for the preceding main menu for ~7 min.
# Player Select is a stronger milestone. Use the user's real full-game path
# instead of backing out to attract mode. Coordinates below are the ORIGINAL
# visible Okay button, measured on a 2400x1080 Android screenshot.
if python tools/detect_sensible_player_select.py "$OUT/after-tap-139s.png"; then
  cp "$OUT/after-tap-139s.png" "$OUT/original-player-select-confirmed.png"
  echo "PASS: original Play Round path reached Player Select page."
  adb shell input swipe 1260 840 1260 840 180
  sleep 1
  adb exec-out screencap -p > "$OUT/after-player-okay-1s.png"
  sleep 2
  adb exec-out screencap -p > "$OUT/after-player-okay-3s.png"
  sleep 4
  adb exec-out screencap -p > "$OUT/after-player-okay-7s.png"
  python - "$OUT" <<'PYPLAYER'
import json,sys,hashlib
from pathlib import Path
sys.path.insert(0,"tools")
from detect_sensible_player_select import is_player_select,signature
root=Path(sys.argv[1])
names=["original-player-select-confirmed.png","after-player-okay-1s.png",
       "after-player-okay-3s.png","after-player-okay-7s.png"]
frames=[]
for name in names:
    p=root/name
    frames.append({"file":name, "sha256":hashlib.sha256(p.read_bytes()).hexdigest(),
                   "still_player_select":is_player_select(p),
                   "blue_brown_green":list(map(lambda x:round(x,4),signature(p)))})
(root/"player-select-okay-diagnostics.json").write_text(json.dumps({
  "player_select_proven":True, "touch":"held primary click at (1260,840) 180ms",
  "next_menu_verified":False,
  "screenshots":frames},indent=2)+"\n")
print(json.dumps(frames,indent=2))
if all(f["still_player_select"] for f in frames[1:]):
    raise SystemExit("Player Select Okay tap did not visibly advance - NOT PLAYABLE")
print("Player Select -> later state VISUALLY CHANGED; human review required before gameplay claim.")
PYPLAYER
  adb logcat -d -s AndroidRuntime:E libretrodroid:E > "$OUT/android-errors.txt"
  echo "ORIGINAL PLAYER-SELECT NAVIGATION PROBE FINISHED: screenshots saved."
  exit 0
fi

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
# The authentic menu can be visible for <3 seconds before attract mode.
# Poll rapidly. Keep periodic screenshots without inflating the CI artifact.
# Earlier runs proved the original menu can appear ~180 seconds after the
# first probes; our window must span that interval without 4-second blind
# spots. The only intended action after recognition is ONE Play Round tap.
menu_start=$SECONDS
for attempt in $(seq 1 320); do
  probe="$OUT/menu-live.png"
  adb exec-out screencap -p > "$probe"
  test -s "$probe"
  if python tools/detect_sensible_main_menu.py "$probe"; then
    cp "$probe" "$OUT/original-main-menu-confirmed.png"
    echo "CONFIRMED: original Sensible Golf six-button menu, attempt $attempt."
    menu_found=1
    # Screenshot from run 37997727471: first (Play Round) button is
    # within x=900..1635, y=344..416 on the 2400x1080 emulator.
    # With the Android core in absolute 'direct' mouse mode, an Android
    # tap here should move the DOS cursor before issuing a left click.
    # ADB's instantaneous tap can release between libretro input polls.
    # A 180-ms stationary swipe is an actual held left-click, not a
    # cursor drag; it spans multiple original/emulator frames.
    adb shell input swipe 1260 380 1260 380 180
    sleep 0.4
    adb exec-out screencap -p > "$OUT/after-play-round-0.4s.png"
    sleep 1.0
    adb exec-out screencap -p > "$OUT/after-play-round-1.4s.png"
    sleep 2.0
    adb exec-out screencap -p > "$OUT/after-play-round-3.4s.png"
    sleep 3.0
    adb exec-out screencap -p > "$OUT/after-play-round-6.4s.png"
    break
  fi
  if (( attempt <= 30 )); then
    if (( attempt % 2 )); then
      adb shell input keyevent 111  # ESC
    else
      adb shell input keyevent 62   # SPACE
    fi
    sleep 3
  else
    # Frequent checks are essential for a short-lived original menu.
    # Take periodic diagnostic frames without uploading hundreds of PNGs.
    if (( attempt % 12 == 0 )); then
      cp "$probe" "$OUT/menu-history-$(printf '%03d' "$attempt").png"
    fi
    sleep 0.55
  fi
  # Bounded wall-clock guard under a slow software-rendered emulator.
  if (( SECONDS - menu_start > 430 )); then break; fi
done
if [ "$menu_found" -ne 1 ]; then
  echo "::error::Original DOS menu was not captured in the bounded probe window."
  exit 1
fi

# A touch dispatched is NOT evidence of a selected playable round.
# Never follow with extra taps/swipes/Enter that could accidentally select
# another game mode. Retain the next frames for a strict state review.
python - "$OUT" <<'PYMENU'
import json,sys
from pathlib import Path
from PIL import Image
sys.path.insert(0,"tools")
from detect_sensible_main_menu import fractions, is_main_menu
root=Path(sys.argv[1])
frames=["original-main-menu-confirmed.png","after-play-round-0.4s.png",
        "after-play-round-1.4s.png","after-play-round-3.4s.png",
        "after-play-round-6.4s.png"]
reports=[]
for name in frames:
    path=root/name
    with Image.open(path) as im:
        size=list(im.size)
    brown, green=fractions(path)
    reports.append({"file":name,"size":size,"original_main_menu":is_main_menu(path),
                    "brown_fraction":round(brown,4),"green_fraction":round(green,4)})
(root/"menu-tap-diagnostics.json").write_text(json.dumps({
    "input_mode":"direct", "tap_screen_xy":[1260,380], "press_duration_ms":180,
    "proof_level":"menu visible and touch dispatched; round selection unverified",
    "screenshots":reports},indent=2)+"\n")
print("Original-menu immediate-tap diagnostics:")
print(json.dumps(reports,indent=2))
PYMENU

python - "$OUT" <<'PY'
import hashlib,json,sys
from pathlib import Path
root=Path(sys.argv[1])
names=["intro-8s.png","after-escape-11s.png","after-space-16s.png",
       "intro-32s.png","intro-80s.png","intro-128s.png",
       "after-tap-131s.png","after-tap-139s.png",
       "after-demo-escape-143s.png","after-demo-space-149s.png",
       "original-main-menu-confirmed.png","after-play-round-0.4s.png",
       "after-play-round-1.4s.png","after-play-round-3.4s.png",
       "after-play-round-6.4s.png"]
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

echo "ANDROID MENU CAPTURED: original main menu reached; direct-mode tap dispatched and screenshots preserved."
echo "NOTICE: Play Round selection and playable golf are NOT accepted without a verified post-tap state and original-game round test."
