# Android gameplay controls — canonical implementation contract

Status: **portable control bridge integrated; Android UI/JNI not yet built**.
This is a product specification, not a claim that an APK exists.

## Guiding rule

Classic mode uses the recovered Windows v1.014 physics and game flow.
A touch, stylus, gamepad or accessibility action must produce the **same**
`ClassicShotRequest`: `aim_raw` 0..4095, `club_index` 0..12,
`power_tick` 0..105 and `accuracy_tick` 0..105.
The Android shell must **never** compensate physics, change slopes,
interpolate meter readings into adjusted power, or decide whether a ball
is holed. Only the C++ `ClassicHoleSession` can do that.

## Player-facing controls (landscape-first)

| Action | Touch | Gamepad | Behaviour |
| --- | --- | --- | --- |
| Aim | Drag/rotate aim dial or press left/right fine nudges | Left stick / D-pad | Adjust original 12-bit 0..4095 heading (wrap at edges) |
| Choose club | Tap club selector, or previous/next arrows | LB/RB | Cycle the original 13 club indexes; show human-readable names **only where verified** |
| Start power | Tap large Welly button once | A | Start visible original power sweep |
| Lock power | Tap the **same** Welly button again | A | Capture raw original meter 0..105 sample |
| Lock accuracy | Tap Welly a third time | A | Capture raw original accuracy 0..105 sample and queue **one** shot |
| Cancel meter | Dedicated cancel/back affordance, before third tap | B | Return to idle without dispatch |
| Pause | Pause button outside play surface | Start | Freeze logical simulation and input meter together; never keep power meter running behind overlay |
| Zoom/pan | Two-finger gesture, separate from aiming dial | Right stick | Camera only; does not alter course coordinates or collision |
| Help | Help button | Menu | Explain three-click timing without affecting gameplay |

The three-click button stays in one location for all three presses; do not
shuffle the button during a live sweep. Aimed direction and club are locked
after the first press. No new stroke can start before the authoritative
session reports `ReadyForShot`. Hazards, code-9/code-10 terminal, scored
holes and resource loads suspend all shot controls.

## Welly meter fidelity

The recovered raw meter power range is **0..105** and accuracy sweet spot
is **63**. The original logical timer interval is `0x03A8` (16.16 seconds),
approximately 70.02 updates/second.

The newly implemented `ClassicMobileControls` owns **click sequencing only**.
It takes raw, already-displayed integer meter samples from the original meter
timer producer; it **does not invent a substitute speed/ramp**.
The Android meter renderer/scheduler must use the recovered timer/dispatcher
mapping and a common logical tick clock with the native game before
the buttons can be claimed fully classic-accurate. On third click:
`dispatch_to(ClassicHoleSession&)` validates that the core is ready
and sends the exact captured request. Rendering can animate independently
at 60/90/120 Hz without changing logical tick timing.

## Layout and responsiveness

- **Phone landscape:** game fills viewport; aiming/club strip on left,
  prominent Welly button and meter on right; score/wind information at top;
  course and ball must remain visible, never under opaque full-height HUD.
- **Foldable opened:** use extra canvas width for a persistent club wheel,
  complete meter and scorecard; keep shot button on a reachable edge.
  Foldable closed falls back to compact phone layout without lost controls.
- **Portrait:** usable compact control arrangement and clear camera area,
  though landscape is the primary gameplay target.
- Keep all primary interactive targets **at least 48dp**.
  Respect cutouts, system bars, navigation gestures and changing insets.
- The selected club, aim, meter phase and score must persist across rendering
  invalidations; rotating the device must not submit or duplicate a shot.
- Screen readers announce the selected club, meter stage, captured power and
  captured accuracy. Offer a reduced-motion indicator without altering the
  logical tick or score.

## Implementation / test boundary

Portable, C++17, newly registered in CMake:
- `engine/include/sensigolf/classic_mobile_controls.hpp`
- `engine/src/classic_mobile_controls.cpp`
- `engine/tests/classic_mobile_controls_tests.cpp`

Use `sync_hole()` at each authoritative phase update. Call `meter_click(raw)`
only on a real user press and with the recovered meter sample for that tick.
Use `dispatch_to(hole)` once when `ShotQueued` and return renderer control
to the core as it enters `ShotActive`. No raw meter sampling on touch DOWN
if the user gesture fires on click UP; exactly one tap event per release.

The new test covers:
- disabled state before hole load and while shot is active;
- 12-bit aim wrap and the original 13-club cycle;
- exact three-click 105-power / 63-accuracy request (including retained aim);
- cancellation, invalid ranges, prevention of duplicate dispatch;
- dispatch into an actual `ClassicHoleSession`, rather than a UI-only mock.

## Android acceptance tests (future APK, not yet met)

1. Complete an original hole using touch only, including actual meter and score.
2. Repeat the same inputs with gamepad; confirm identical core shot request/trace.
3. Three taps never produce two strokes; pause/rotate/cancel never causes a shot.
4. Folded and unfolded Android layouts show all primary controls without overlap.
5. Accessibility input and large text preserve the click timing and original
   physics; original round replay/golden-master suites remain green.
6. Code-9/code-10 unresolved continuation is **not** silently approximated
   by the Android interface. The UI must show a supported/unsupported state.

Commercial original artwork and audio stay external until licensed.
