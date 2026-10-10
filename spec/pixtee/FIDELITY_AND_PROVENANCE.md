# Fidelity specification and independent-development boundary

## Objective

Players of 1990s overhead arcade golf should recognise the **pace, golfer proportions, screen presentation, fairway/green readability, spin/flight/bounce/roll feel, 3-click timing, putting challenge, score display and short loop**. This is a **design target**, not a claim of legal entitlement to reproduce protected expression.

## Behaviour and measurements

| Category | Target / measuring method | Porting rule |
| --- | --- | --- |
| Aiming | full circle, fine adjustments, deterministic aim indicated on green/fairway | independent angle mapping |
| Power/accuracy | same three presses, moving bar, sweet-spot challenge; calibrate with volunteer gameplay observations | write own timer and UI; no copied code |
| Clubs | 13 functional slots including putter; choose comparable carry/loft curves through original authored numeric tuning | do not transplant recovered constants or tables |
| Ball movement | 2D overhead air flight, spin/curve, bounce, terrain drag, slopes, sand and rough penalties | implement own model, tune from observed outcomes |
| Putting | separate gentle putter behaviour with slope/green detail | independently model and test |
| Hazards | out of bounds, water, NO GO, relief and penalties as openly specified golf rules | original logic independently specified |
| Course scale | ball/avatar/flag/green ratios, camera tracking, UI proportion from screenshots/video | record measured ratios; do not copy course meshes/bitmap |
| Camera | scrolling overhead world, gentle focus on ball and hole, fixed game-world units distinct from device px | independently implement; validate in phone/fold |
| Timing | responsiveness and animation duration compared visually; physics update independent of draw | independently defined deterministic simulation ticks |
| Sound | satisfying club/landing/putt/score feedback | new recordings and compositions |
| Graphics | clean 16-bit inspired pixels, recognizable grass/water/bunker values, legible golfer | fresh illustrations |
| Original holes | similar strategic challenge and difficulty **but original geometry, hazards, tee/green shapes, arrangement and decorative expression** | never use original course data as a template and shift bunkers a few cells |
| Wind | optional **off** by default; isolated feature flag | independent optional module |
| Career/stats/rewards | modern Pixtee-only layers that never silently adjust classic shot physics | new designs |

## Calibration evidence to collect, not invent

No reliably measured original **rendered viewport**, **native pixel pitch**, **sprite pixel dimensions**, **course-to-screen projection**, **camera dead zone**, **zoom**, **golfer frame timing**, **exact HUD placements**, or **graphics asset colour values** have been established by the recovered C++ physics test suite. The original physics world has recovered MAPM tile decomposition (logical horizontal 16 units and vertical 8 units per map tile), a detailed green-mode conversion, and fixed-point coordinate representation. **These physical data lookup units are not the original screen-pixel scale**. It would be misleading to equate them.

Create a separate observer-based calibration workbook with at least 5 publicly viewable reference clips/screens:
1. Capture native or unscaled visual frame dimensions, device scaling and gameplay screenshot crop metadata.
2. Mark sprite width/height, ball diameter, cup/flag size, green width, fairway edge, hole overview and HUD boundaries in pixels.
3. Log camera follow/dead-zone across several second-by-second ball-flight samples.
4. Log shot timing and button feel against capture framerate, accounting for video playback speed.
5. Record qualitative similarity results; calculate target dimensionless ratios and acceptable variances.
6. Make independent artists produce all graphics without pixel-tracing the original sprites or course imagery.

## Evidence separation

The Sensi-golf restoration contains reverse-engineered machine-code facts and parity-derived numeric constants. **Do not move those source files, tables, original binary or original course geometry into Pixtee.** If exact original physics code/data is required, obtain a licence and treat that as a separately licensed remaster, not a clean-room experiment.

The Pixtee specification may use generally understood gameplay concepts, empirical player testing, and newly written publicly documented sport rules. For strict clean-room practice, a team without access to disassembly/recovered source should implement from this functional specification and records of gameplay observations. Preserve who wrote each module and source provenance in a reviewable asset ledger.

A technical fidelity claim (e.g. exact physics) requires a measured independent test suite and recorded tolerances, not an unsupported declaration. Visual and course resemblance must be separately reviewed by an IP specialist before distribution.

## Release gates

- Independent game and course geometry; independently designed pixel art and audio.
- No embedded original binaries, course archives, raster tiles, music or original program text.
- IP review: trademarks, titles, store listings, trade dress, course-layout substantiality, copied imagery.
- Keep a separate legal/permissions track for any future high-fidelity licensed use.
