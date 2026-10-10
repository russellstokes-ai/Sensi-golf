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

# Keyboard forwarding now works. In verified run 38041416122, ESC
# reveals the REAL main menu here, long before the legacy 149s wait.
# Take Play Round immediately, while the original menu is on screen.
if python tools/detect_sensible_main_menu.py "$OUT/after-escape-11s.png"; then
  cp "$OUT/after-escape-11s.png" "$OUT/early-main-menu-11s.png"
  adb shell input swipe 1260 380 1260 380 230
  sleep 1
  adb exec-out screencap -p > "$OUT/after-early-play-round.png"
  if python tools/detect_sensible_player_select.py "$OUT/after-early-play-round.png"; then
    cp "$OUT/after-early-play-round.png" "$OUT/early-human-player-select.png"
    adb shell input swipe 1250 840 1250 840 230
    sleep 1
    adb exec-out screencap -p > "$OUT/after-early-human-okay.png"
    early_course_frame="$OUT/after-early-human-okay.png"
    if ! python tools/detect_sensible_course_select.py "$early_course_frame"; then
      # Preserve the failed first click evidence. A true Android ESC/ENTER
      # is now forwarded to the original game by our libretro key callback.
      adb shell input keyevent 66
      sleep 1
      adb exec-out screencap -p > "$OUT/after-early-human-enter.png"
      early_course_frame="$OUT/after-early-human-enter.png"
    fi
    if python tools/detect_sensible_course_select.py "$early_course_frame"; then
      cp "$early_course_frame" "$OUT/early-human-course-menu.png"
      adb shell input swipe 1260 310 1260 310 230
      sleep 2
      adb exec-out screencap -p > "$OUT/after-early-human-augusta-2s.png"
      sleep 5
      adb exec-out screencap -p > "$OUT/after-early-human-augusta-7s.png"
      # Run 38042290360 proves the above Augusta tap opens the genuine
      # ORIGINAL Tournament Leaderboard, HUMAN 1, 'Play Next Hole' button
      # centered at (1260,845) on the 2400x1080 emulator. This is the
      # actual missing step; the prior probe exited before pressing it!
      echo "HUMAN 1 / AUGUSTA: selecting ORIGINAL Play Next Hole (1260,845)."
      adb shell input swipe 1260 845 1260 845 240
      sleep 1
      adb exec-out screencap -p > "$OUT/after-play-next-hole-1s.png"
      sleep 3
      adb exec-out screencap -p > "$OUT/after-play-next-hole-4s.png"
      sleep 5
      adb exec-out screencap -p > "$OUT/after-play-next-hole-9s.png"
      # Do not blindly press game controls while still in a blue menu.
      # The authentic golf course has grass-green terrain. Capture both
      # the raw tee image and the decision for human inspection.
      if python - "$OUT/after-play-next-hole-9s.png" <<'PYTEE'
from PIL import Image
import sys
im=Image.open(sys.argv[1]).convert("RGB")
w,h=im.size
pixels=[im.getpixel((x,y)) for y in range(int(h*.25),int(h*.78),14)
        for x in range(int(w*.26),int(w*.74),14)]
green=sum(g>55 and g>r*1.25 and g>b*1.15 for r,g,b in pixels)/len(pixels)
print(f"ORIGINAL TEE CANDIDATE: sampled grass-green={green:.3f}; "
      f"gameplay verification requires screenshot review")
raise SystemExit(0 if green > .18 else 1)
PYTEE
      then
        # Probe the actual native Android overlay, NOT a separate game
        # implementation. Toggle Pad and press SWING three times through
        # its touch-to-Keyboard-CTRL binding; retain every intermediate
        # original DOS frame for verification of the three-click meter.
        echo "TEE COLOR PRESENT: testing original on-screen direction/SWING."
        adb shell input tap 2290 86
        sleep 1
        adb exec-out screencap -p > "$OUT/tee-pad-expanded.png"
        adb shell input swipe 2140 880 2140 880 350
        sleep 1
        adb exec-out screencap -p > "$OUT/tee-after-direction.png"
        for press in 1 2 3; do
          adb shell input swipe 2260 895 2260 895 190
          sleep 0.75
          adb exec-out screencap -p > "$OUT/tee-after-swing-${press}.png"
        done
        sleep 4
        adb exec-out screencap -p > "$OUT/tee-after-swing-flight-4s.png"
      else
        echo "NOT TEE: retained post-Play Next Hole frames; do not assert playability."
      fi
      adb shell pidof "$PKG" > "$OUT/human-after-play-next-hole-pid.txt"
      echo "EARLY HUMAN GAMEPLAY PROBE COMPLETE: inspect real tee / swing screenshots."
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
    sleep 3
    adb exec-out screencap -p > "$OUT/after-augusta-4s.png"
    sleep 5
    adb exec-out screencap -p > "$OUT/after-augusta-9s.png"
    sleep 11
    adb exec-out screencap -p > "$OUT/after-augusta-20s.png"
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
