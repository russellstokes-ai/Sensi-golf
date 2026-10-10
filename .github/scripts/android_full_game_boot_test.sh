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


# Single shared navigation probe for BOTH early and delayed Player Select.
# Always press the original Augusta leaderboard Play Next Hole before exiting.
probe_original_human_tee() {
  probe_original_human_tee
      exit 0
    fi
  fi
fi

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

# Player Select has been seen in previous Android evidence at 139s and even
# at the final 'menu-live' frame. The old script looked ONLY for Main Menu,
# then waited ~7 minutes and incorrectly failed on a valid next screen.
# This navigation helper is evidence collection, not gameplay sign-off.
player_select_probe() {
  local frame="$1"
  if ! python tools/detect_sensible_player_select.py "$frame"; then
    return 1
  fi
  cp "$frame" "$OUT/original-player-select-confirmed.png"
  echo "CONFIRMED: original DOS Player Select reached before attract mode."
  # Preserve the original HUMAN 1 / Keyboard selection. Earlier CI
  # tapped the Type field once, inadvertently changing Human 1 into a CPU
  # player (York / CPU Easy). A CPU golf round is NOT a human-playability
  # test. Leave this field untouched in the normal compatibility run.
  echo "Testing native Human 1 default control, not CPU AI."
  adb shell input swipe 1250 840 1250 840 230
  sleep 1
  adb exec-out screencap -p > "$OUT/after-player-okay-1s.png"
  sleep 2
  adb exec-out screencap -p > "$OUT/after-player-okay-3s.png"
  # The exact ORIGINAL course list was confirmed in Android run
  # 38038987483: Augusta is at (1260,310), but only touch it AFTER
  # the non-OCR course-state detector has verified this is that screen.
  course_frame=""
  for frame in "$OUT/after-player-okay-1s.png" "$OUT/after-player-okay-3s.png"; do
    if python tools/detect_sensible_course_select.py "$frame"; then
      course_frame="$frame"
      break
    fi
  done
  if [ -z "$course_frame" ]; then
    # Human Keyboard differs from a CPU player in run 38040976533:
    # the first 230-ms Okay click left the EXACT frame unchanged. Probe
    # a separate tap, then original keyboard ENTER (now actually
    # forwarded through the native libretro keyboard callback).
    echo "Human Player Select still visible: try fresh Okay tap."
    adb shell input tap 1250 840
    sleep 1
    adb exec-out screencap -p > "$OUT/after-player-okay-retry-tap.png"
    if python tools/detect_sensible_course_select.py "$OUT/after-player-okay-retry-tap.png"; then
      course_frame="$OUT/after-player-okay-retry-tap.png"
    fi
  fi
  if [ -z "$course_frame" ]; then
    echo "Human Player Select still visible: try keyboard ENTER."
    adb shell input keyevent 66
    sleep 1
    adb exec-out screencap -p > "$OUT/after-player-okay-enter.png"
    if python tools/detect_sensible_course_select.py "$OUT/after-player-okay-enter.png"; then
      course_frame="$OUT/after-player-okay-enter.png"
    fi
  fi
  if [ -n "$course_frame" ]; then
    cp "$course_frame" "$OUT/original-course-select-confirmed.png"
    echo "CONFIRMED: real original COURSE SELECTION after HUMAN player."
    adb shell input swipe 1260 310 1260 310 230
    sleep 1
    adb exec-out screencap -p > "$OUT/after-augusta-1s.png"
    # The previous test stopped at the genuine Human 1 Augusta leaderboard.
    # Now use the SAME first-tee probe regardless of early or fallback path.
    probe_original_human_tee
  else
    echo "::warning::Human Player Select did not reach detected course list."
    sleep 4
    adb exec-out screencap -p > "$OUT/after-player-okay-7s.png"
  fi
  python - "$OUT" <<'PYPLAYER'
import hashlib,json,sys
from pathlib import Path
sys.path.insert(0,"tools")
from detect_sensible_player_select import is_player_select
from detect_sensible_course_select import is_course_select
root=Path(sys.argv[1])
names=["original-player-select-confirmed.png","after-player-okay-1s.png",
       "after-player-okay-3s.png","after-player-okay-retry-tap.png",
       "after-player-okay-enter.png","original-course-select-confirmed.png",
       "after-augusta-1s.png","after-augusta-4s.png",
       "after-augusta-9s.png","after-augusta-20s.png",
       "after-player-okay-7s.png"]
frames=[]
for name in names:
    p=root/name
    if not p.is_file(): continue
    frames.append({"file":name,"sha256":hashlib.sha256(p.read_bytes()).hexdigest(),
         "original_player_select":is_player_select(p),
         "original_course_select":is_course_select(p)})
course_accepted=(root/"original-course-select-confirmed.png").exists()
(root/"original-human-round-navigation.json").write_text(json.dumps({
  "original_human_player_select_reached":True,
  "human_1_control_type":"Keyboard (original untouched default)",
  "player_okay_screen_xy":[1250,840],
  "original_course_selection_reached":course_accepted,
  "augusta_tap_screen_xy":[1260,310] if course_accepted else None,
  "original_first_tee_verified":False,
  "human_aim_swing_score_verified":False,
  "touch_hold_ms":230,
  "screenshots":frames},indent=2)+"\\n")
print(json.dumps(frames,indent=2))
print("ORIGINAL COURSE MENU:",course_accepted,
      "original playable tee still REQUIRES screenshot inspection.")
PYPLAYER
  adb shell pidof "$PKG" > "$OUT/post-player-select-pid.txt"
  adb logcat -d -s AndroidRuntime:E libretrodroid:E > "$OUT/android-errors.txt"
  if grep -Eq 'FATAL EXCEPTION|UnsatisfiedLinkError|Unable to start activity' "$OUT/android-errors.txt"; then
    echo "::error::Android exception after Player Select navigation."
    return 2
  fi
  echo "PLAYER SELECT PROBE COMPLETE: screenshots preserved; gameplay NOT yet proven."
  return 0
}
if player_select_probe "$OUT/after-tap-139s.png"; then
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

# Real screenshot from run 38004770739 already showed the original, fully
# lit main menu at this exact time. A second screenshot/detection cycle
# allowed the ~3-second Demo Mode timeout to win. Use the first captured
# frame IMMEDIATELY and avoid another expensive adb screencap before tapping.
if player_select_probe "$OUT/after-demo-space-149s.png"; then
  exit 0
fi
if python tools/detect_sensible_main_menu.py "$OUT/after-demo-space-149s.png"; then
  cp "$OUT/after-demo-space-149s.png" "$OUT/first-visible-main-menu.png"
  echo "EARLY MENU: selecting Play Round directly from the already-captured frame."
  adb shell input swipe 1260 380 1260 380 230
  sleep 0.5
  adb exec-out screencap -p > "$OUT/after-early-play-round-0.5s.png"
  sleep 1
  adb exec-out screencap -p > "$OUT/after-early-play-round-1.5s.png"
  sleep 2
  adb exec-out screencap -p > "$OUT/after-early-play-round-3.5s.png"
  for frame in "$OUT/after-early-play-round-0.5s.png" "$OUT/after-early-play-round-1.5s.png" "$OUT/after-early-play-round-3.5s.png"; do
    if player_select_probe "$frame"; then
      exit 0
    fi
  done
  echo "EARLY MENU TOUCH: no verified Player Select yet; continue diagnostic polling."
fi

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
  # Player Select is itself proof that Play Round was already entered.
  # Never wait for the earlier main menu once this screen has appeared.
  if player_select_probe "$probe"; then
    exit 0
  fi
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
    # Re-check the NEXT original screen; some boots reach Player Select,
    # others return to Demo Mode. Do not treat those outcomes as equivalent.
    for frame in "$OUT/after-play-round-0.4s.png" "$OUT/after-play-round-1.4s.png" "$OUT/after-play-round-3.4s.png" "$OUT/after-play-round-6.4s.png"; do
      if player_select_probe "$frame"; then
        exit 0
      fi
    done
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
