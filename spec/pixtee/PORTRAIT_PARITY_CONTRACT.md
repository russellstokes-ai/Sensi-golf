# Canonical game-space / phone presentation contract

## User-locked requirements (10 Oct 2026)
Pixtee recreates **observable Sensible Golf gameplay** as closely as possible in an independently built game. Physics, collision behavior, swing timing, putting, course/character/ball/sprite ratios, camera movement and playing scale are acceptance criteria. The only intended creative departures are **new authored courses, original art/branding, and small menu changes**, with optional additive career/stats/rewards.

- **Phone portrait, full screen 19.5:9 and 20:9**; Fold closed portrait as priority. No black letterbox bands.
- Strict top-down 2D tiny character and ball, left original-style compact HUD, blue/wood retro menus, Whack-o-meter (three clicks).
- World sizes and physics MUST NOT depend on device resolution, density, refresh rate or aspect ratio.
- One uniform scale X == Y. Extra portrait height reveals more course (vertical viewport), not a stretched or artificially zoomed player, course or collision mesh.
- World-to-screen transform is separate from physics/collisions; one source of truth. All dynamic hitboxes/world points are projected identically.
- Fixed 60Hz deterministic update, replay fixtures for driver, iron, sand, water, putting, flight/bounce/roll, offscreen camera/panning and score.
- Measure visible golfer/ball/flag/green/tee ratios against original gameplay footage before locking numeric pixel values. **The initial Pixtee world units, physical shot model and art geometry are provisional and not demonstrated identical to the original.**
- New geometry and visuals must be independently authored. No original executable/sprite/course layout copied and no assumption that only cosmetic alterations resolve infringement risks.
- No XP/level rewards changing traditional ball physics.

## Phased parity gates
1. A 360×720, 360×760, 360×800 portrait sample renders with the **same golfer-to-fairway scale** and **same physics-state trajectory** after identical inputs. Automated tests of transforms plus screenshot checks.
2. Measure original game HUD position, native golfer sprite dimensions, ball/flag ratios, camera tracking, swing bar cadence and acceptance tolerances with a reference video and permission. Preserve evidence, do not invent measurements.
3. Independent physics matches observed launch, carry, bounce, rolling, green slope, lies, collisions, hole and penalties. Cover every club, swing accuracy and lie with golden-master traces.
4. First complete human-controlled hole, scored/verified through Android emulator, then real portrait Fold closed/open. Only after these gates call it a gameplay-faithful clone.
