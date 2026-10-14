# Pixtee: gaps versus the owner reference pictures

## Stop point and quality target
The owner will switch to Astra for final polish. The supplied October 14 sunny/autumn and mist reference pictures are the quality target. Current gameplay captures remain materially flatter and less composed; do not claim that target has been achieved. Only one authored Lakewood hole is required now. No APK until the owner accepts readiness.

## Current implementation
Recovered C++ remains the sole ball, collision, scoring and gameplay PRNG authority. Source tick is 936/65536 seconds. Approved course remains 544x912, 445 placements, unchanged aspect ratios and nearest sampling. All original ZIPs/archive branches remain immutable.

Implemented: three-tap horizontal native meter with immediate return after power capture and reduced rough/bunker windows; integrated tap-to-cycle club card; invisible aiming touch zones; animated standalone minimap; male/female layered right-handed golfers and saved name/outfit/skin/hair/face/beard/fringe controls; layered crowds and animals; native species-sized bird flocks/contact responses; directional pond ducks with event-driven fright; occasional rabbit and tree squirrel sightings; off-green animated caddy holding the removed flag when the native putting state removes its collision; cosmetic weather presets; flowing water, falling leaves, cloud shadows, photo flashes, ready-meter glow; supplied audio and menu-only theme rotation.

After a shot ends, the stopped-ball view holds for 1 second, fades out over 0.32 seconds, changes the view while black, holds black 0.08 seconds and fades in over 0.42 seconds. Input is blocked during the transition; native simulation is unchanged. Pause settles the transition once. Score presentation follows the same transition. Title flyover already fades only the background, keeping the menu/logo visible, and repeats Lakewood until further holes exist.

## Gaps visible in actual captures versus references
| Area | Current gap | Astra acceptance target |
|---|---|---|
| Light and weather | Mist/sun shader and slow cloud shadows are active, but static captures still look comparatively flat. | Soft, readable mist banks, coherent sunlight/shade, depth and atmospheric composition at phone scale; preserve course colours/art authority. Review every weather preset in motion. |
| Ground and objects | Trees and terrain are preserved; shadows are less rich than the references. | Consistent grounded feet, tree/crowd/caddy/ball shadows, believable ball-height separation; no detached shadows or painted light overriding sprites. |
| Water | Flow is masked to authored river/pond boundaries; tiled detail remains visually repetitive. | Smooth current, bank-safe shimmer/ripples and convincing pond wakes; no stretching, crawling seams or flashing noise. |
| Golfers | Layered eight-direction art, fixed handedness and palette options are implemented. Beard/fringe are small shader details, not a large library of authored hair meshes. | Review full swing from all directions and both bodies, feet/club/ball contact, transitions and breathing. Eliminate pose popping/glitches; improve face/hair readability where necessary. |
| Wildlife | Eight-direction ducks, layered birds, occasional rabbit/squirrel and package frog/dragonfly are active. | Seamless turns, plausible sizes, natural timing and restrained reactions. Validate real bird hits, pond fright and tree squirrel visibility in play. |
| HUD and meter | Club card and minimap remain plain rectangular panels; meter housing lacks reference-quality light/frame treatment. | Cohesive premium pixel UI with clear contrast, spacing and attractive moving indicator/ready glow. Preserve tested native timing, selected power and lie-window widths. No separate club buttons or start-swing text. |
| Menus | Approved button/logo assets are integrated, but auxiliary settings/customisation panels remain utilitarian. | Approved-art-led menus with consistent readable mobile spacing, attractive customisation preview and polished transitions. Do not replace canon artwork. |
| Motion and effects | Event particles, leaf silhouettes, water, breeze, flash and fades exist; screenshots alone cannot validate their quality. | Watch a full round with audio, all aim directions, recovery and pause. Tune effect density, speed, layering and reduced-motion behaviour without changing physics. |

## Functional gaps to keep separate from visual polish
- Title-background golfers use authored shot inputs in isolated native rounds. Complete recovered CPU decision/evaluation is NOT integrated. Do not describe them as complete native AI opponents. Camera/background fade exists, but native AI integration remains a functional task.
- Original-executable parity is not certified for authored board/tree/bird collisions, custom mobile meter cadence, current CPU menu behaviour or optional soft-ground authoring. These run inside the native core; do not claim they are the exact original detector/controller.
- Advanced appearance currently offers limited face/fringe/beard variation, not arbitrary sculpting or many distinct haircut silhouettes.
- Multi-hole title cycling cannot be tested with distinct course resources until more authored holes exist.
- Hands-on Windows visual/audio/gameplay acceptance and Android/Fold device/performance/pause checks remain outstanding. All commercial release gates stay OPEN; no commercial-ready claim and no APK.

## Verification and restoration
Use the latest tested commit/run/build recorded in work/REMOTE_CHECKPOINT.json. The local Git HEAD is stale: f0e3f258. Working files contain newer remote changes. NEVER reset or bulk-push from that stale HEAD. Fetch the actual feature-branch head before further connector commits. Feature branch: feat/pixtee-living-world-polish-2026-10-14; draft PR16. Keep archive/canonical branches untouched.

The checks cover 28 native tests, source-boundary tests, deterministic native session replay/flag removal, Godot 4.5 import and scene tests, actual GL screenshots, profile/weather persistence, native tee-to-score completion and one-shot transition callback/pause/cancel safety. Automated passes are not visual or device acceptance.

## Owner playtest sequence
1. Start a fresh round after upgrading from R4/R5; old course-rule saves are deliberately incompatible. Choose body/name/outfit/appearance and verify Apply/Cancel repeatedly.
2. Tee shot: tap bar to start, capture chosen power, stop on red; verify timing/zone widths and no club-contact audio duplicates. Aim through every direction with no handedness flip.
3. Watch flight, shadows, landing, 1-second hold and fade. Try sand, rough, trees, signs, flag, water and both OOB routes; pause/focus away during shot/transition and Continue recovery.
4. Putt with flag removed, caddy clear of cup/green/bunker, native cup/lip/score intact. Finish one full hole and verify one recorded score.
5. Compare sunny/mist/rain effects, creatures and water with reference pictures, then review title flyover/background fade while menu remains opaque. Test music toggles and nature ambience, with no gameplay music.

Next: Astra should use the verified Windows review build/captures, resolve visible gaps, and obtain hands-on acceptance before any device/release gate promotion.
