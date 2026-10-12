# Pixtee Golf — Milestone One World FX, Shader and Game-Feel Standard
**12 October 2026 | Godot 4.5 | Three-hole production integration | DRAFT IMPLEMENTATION SPEC — effects NOT YET BUILT OR APPROVED**

**Owner direction:** Pixtee must feel *alive*, not like a static sprite sheet. Use high-quality, subtle, consistent Godot 2D particle effects, CanvasItem shaders, lighting/shadow layers, animation and sound reactions **without changing the approved overhead pixel-art look or recovered Sensible Golf gameplay**.

## Authority and change protection

1. **Frozen first-hole visual wins.** The approved frozen Hole 1 HTML, its integrity hashes and the Work-authored `COURSE_PRODUCTION_STANDARD.md` are owned by the parallel Work session. They were referenced via **local Windows paths and have not been opened, imported or verified in this GitHub update**. The Work production standard may already cover particles/shaders/lighting. Cross-check and reconcile this additive brief **before implementing**; do not silently replace its decisions. Do not edit the frozen HTML/images/hashes or the approved source asset ZIP.
2. **Gameplay physics wins.** Only `sensigolf_core` native C++ advances ball, heading, height, shot, contact, penalties, hazard, lie, PRNG, cup or score. Shader animation, particle systems, Godot timelines, camera or device FPS NEVER choose gameplay outcomes or advance the engine clock. VFX listens to read-only snapshots / native event packets; no reverse writes into physics except the established safe UI acknowledgements when an event is actually settled.
3. **Style lock.** True straight-down 2D overhead; same playable world unit scale and partial-hole crop. Refine the existing painterly pixel art with natural motion and gentle transient effects, **not** an isometric angle, bloom-heavy mobile look, realistic 3D grass, photorealistic reflections, giant explosions, saturation shifts or reillustrated sprites. Motion must not cover the target X, shot meter, ball or cup.
4. **Milestone One scope.** Prove FX on Hole 1 and reuse the same effect library on Holes 2 and 3. A single portable set of reusable world materials/prefabs is better than one-off course-specific GPU shaders. Course assets remain Pixtee-authored; 50-course catalogue expansion happens later.
5. **Assets and copyright.** Only original Pixtee-approved materials, new independently authored particles and licensed audio/textures ship. No original Sensible Golf proprietary sprites, EPF, music or UI frames. A source behaviour study is not a licence to ship original media or code/tables.

## A. Effects that bring the course to life (priority order)

| Layer / event | Godot implementation direction | Desired motion / look | Trigger and critical rule | M1 tier |
|---|---|---|---|---|
| Grass and blades | Region-masked 2D shader/short animation frames | Gentle staggered lean, individual moving patches; very low amplitude, restrained colour variation | Independent course breeze vector; never change terrain/friction | **P0** |
| Flowers, reeds and small plants | Shared wind shader or short looping approved frames | Nearby stems sway asynchronously, some delayed response to gust | Same breeze phase as grass; anchor roots, no clipping | **P0** |
| Tree foliage | Pivot-anchored 2D material / preauthored frames | Canopy moves a fraction independently of trunk, slight shifting shadow beneath | *Visual only*. Tree collision requires a **separately authored original-informed 3D gameplay collider** | **P0** |
| Water bodies and streams | Lightweight world-space masked CanvasItem UV drift and subtle highlight/edge shimmer | Slow surface motion, muted pixel-sized light ripple; **NO big waves, boiling, noisy turbulence or full-screen refraction** | Flow direction authored per water body; no physical drift to ball or hazard tiles | **P0** |
| Golf flag, bunting, small boards | Subtle pivot motion; pinned Sprite2D/AnimationPlayer, optional shader | Flag flutters coherently with grass/flowers; advertising remains legible and stationary | Breeze drives flag. Boards **never** sway enough to obscure brands or collide | **P0** |
| Ball elevation/shadow | Shadow sprite driven by native height and world coordinates | Clear vertical separation during flight; soft/offset shadow intensity/size varies by native ball altitude | Read-only `ball_state`, NEVER infer ball height from animation curve | **P0** |
| Tee hit and ground strike | Finite burst, Godot 2D particles | 2–7 tiny grass/sand flecks plus restrained dust matching lie | Native confirmed shot contact; no extra simulated club collision | **P0** |
| Landing and successive bounces | Short burst/impact sprite with particles | Grass tuft on fairway/rough; small dust in sand; clear settle cues | Native ball/contact/lie transition. One visual event per authentic impact; avoid repeated tick-spam | **P0** |
| Bunker impact | Sand-colour short-lived CPUParticles2D/GPUParticles2D + tiny dimple/puff | Brief granular sand scatter; no thick cloud hiding the ball | Native sand collision; no change to drag, friction or spin | **P0** |
| Water splash and ripples | Small burst + expanding pixel ring masked to water | Quick splash and 1–2 gentle decaying rings, then still-water motion | Native source water/hazard event; subsequent relief remains native | **P0** |
| Tree branch hit | Small leaf fleck burst and short foliage recoil | Immediate tactile response to a verified branch/trunk interaction | **Do not trigger until actual tree physics confirmed**; decoration near a tree ≠ hit | **P1 BLOCKED** |
| Cup/flag impact and hole-out | Tiny flag twitch / cup rattle, quick sink frame and restrained celebratory sparkle | Satisfying but not arcade fireworks; respect cup lip/outcome | Source native code8/lip/score event, never proximity alone | **P0** |
| Green slope arrows | Pixel-clean existing art / slow pulse | Easy to read slope direction and magnitude, not distracting | Direction/strength sourced from native MAPS, never a shader approximation | **P0** |
| Golfer idle and swing follow-through | Approved frame animation | Soft breathing during ready, satisfying club follow-through on actual strike | Native shot phase, animation cannot independently launch a ball | **P0** |
| Spectators/crowd/photographer | Short staggered idles, reaction frames, one flash | Gentle collective bob/shift, occasional camera flash, restrained celebration after good shots | Background loops separate from gameplay PRNG; no physical barriers unless explicit manifest | **P1** |
| Side-on birds by region | Existing approved side-on flight sprites, path animation | Occasional bird crossing **above course**, natural different birds by climate; rare near-ball reaction | Visual flight alone has no hit/ball deviation. Funny bird-hit reaction **only after explicit verified collision/gameplay event**, not fabricated | **P1** |
| Moving cloud shade / dappled sunlight | Very subtle large low-frequency world-space overlay, clipped | Warm outdoor depth, no day/night relight or global hue change | Optional by biome, pixel-art compatible; no moving hazard cues | **P2** |
| Wind-aware meadow / dune / tropical variation | Material parameter presets | Grass, reeds, tree sway and flags collectively respond to gentle L→R or R→L breeze | Per-hole direction in independently authored course metadata; **visual breeze != ball wind physics** unless native wind mechanic explicitly supports it | **P1** |
| Audio reactions | Licensed authored SFX samples + lightweight mixer controls | Crisp club strike, lie contact, bird chirp, splash, leaf rustle, crowd murmur and cup | Dispatch only after authoritative event; no Sensi commercial sounds | **P1** |
| UI micro motion | restrained easing/focus feedback | responsive club cycle, meter start/return, clear button, scored-stroke cue | Visual only; native tick and inputs fixed; reduced-motion alternative | **P1** |

### P0: first-hole demonstration standard

**Hole 1 must visibly breathe while no one touches the screen.** At minimum:
- Ambient grass/flowers/tree leaves/flag respond to the **same small breeze vector** but with phase offsets; one course can flow left→right, another right→left.
- Water subtly moves even during idle without visibly rough waves; reed edge motion and tiny highlights reinforce it.
- A tee strike produces a tiny surface-specific response; real ball height drives the shadow; real bounce creates a small impact; sand and water have recognisably different particles.
- As ball reaches a green, grass/green ambience quiets slightly and the slope arrows remain legible.
- Cup interactions, score change and transition feel responsive but do not falsify a cup capture or freeze controls.
- Sponsor boards are camera-facing world scenery near tee/green, **not on paths, in shot lanes or HUD**, and cannot be counted as physics obstacles.

## B. Godot 4.5 proposed architecture (not a claim of existing files)

```text
PixteeCourseScene.tscn
  World
    TerrainVisuals         # approved course materials, tile edge masks
    WaterVisuals           # masked low-motion UV/canvas_item material instances
    VegetationVisuals      # approved flowers, grass, foliage with breeze material
    CoursePropsVisuals     # flag, tee, world-space sponsor boards, crowd, birds
    AmbientFx             # reusable low-budget scene particles
    GameplayVisuals
      BallSprite           # reads native x/y ONLY
      BallShadow           # reads native x/y/z ONLY
      ImpactFxPool         # event-driven reused hit/bounce/sand/water/leaf FX
      CupFx
  Camera2D                 # true overhead, partial hole follows authoritative ball
  Hud                     # independent responsive portrait/Fold layout
  FxDirector               # subscribes to native gameplay snapshots/events
  BreezeDirector           # only drives visual materials/animation defaults
```

Implement a shared `FxDirector`/effect-event adapter as a proposed new Godot host module; names here are **design targets, not existing APIs**. It must subscribe to the real `PixteeRoundBridge` or native `ClassicHoleSession` state and use a deduplicated gameplay-event identity: `(hole_id, shot_id, event_kind, native_event_sequence)` (proposed host envelope) so one bounce does not emit dozens of particle bursts on adjacent frames.

**Source of truth hierarchy:**
1. Fixed native tick delivers a read-only snapshot, resolved native lie, ball position/altitude/heading/velocity, shot/hole phase and any supported pending event.
2. Godot interpolates *only drawing* between snapshots to follow the ball smoothly; never modifies C++ positions. On authoritative state corrections, reset visual interpolation cleanly.
3. FxDirector maps **confirmed event kinds** to reused particle/animation templates. Unknown native events must degrade gracefully into a safe neutral acknowledgement or explicit unsupported state, **never invent effects/outcomes**; don't automatically acknowledge an event that requires user interaction.
4. Cosmetic particles/foliage/birds use a **distinct visual RNG seed** from recovered gameplay PRNG. Render/timing differences must not change physics or replay; seek or load save state resets/clears FX exactly once.
5. Wind visual vector can be included in authored course metadata. If gameplay wind is later independently verified and added, display the **real native gameplay wind**; otherwise visual breeze must be explicitly presentation only and must not mislead the user about ball drift.

**Material/particle choices:**
- Prefer **2D CanvasItem shaders on region masks** and approved Sprite2D frames for vegetation/water, with per-instance phase variation (avoid rigid synchronous sway).
- Use **pooled short-lived 2D particle emitters** for collisions. Pick GPUParticles2D or CPUParticles2D after profiling target Android renderer/device; do not assume GPU is always cheaper on low-end phones.
- Keep art nearest-neighbour crisp. Quantize/decorrelate displacement when needed so pixel edges don't become a smeared or stretched gradient. Do not blur the whole background or touch target.
- Apply world-space masks for water and grass. Water sparkle shouldn't leak onto bunkers/greens, nor should foliage shader move fixed tree trunks.
- Lighting: start with *baked/painted* daylight and optional gently changing masked shade. Minimize live 2D light/normal maps, expensive full-screen postprocessing, multi-pass particles and shader overdraw. Only add justified higher-cost effects after device profiling.
- Shader input parameters are authored once per material type (recommended names: `breeze_dir`, `breeze_strength`, `phase`, `flow_direction`, `effect_quality`); an artist can tune approved values without C++ changes.
- Loop animation durations vary organically; no obvious synchronized heartbeat across every grass tuft, flower or spectator.

## C. Performance, quality settings, accessibility

**Device targets:** normal Android portrait phone; Samsung Fold closed and open. Godot renderer must support the chosen devices. Begin with **60 FPS target** in primary tested gameplay mode (16.7 ms total frame budget), with an explicitly measured lower quality mode and safe 30 FPS fallback on slower hardware. **These are proposed budgets, not measured performance claims.** The underlying original `936/65536` second simulation step remains fixed regardless of render FPS.

- Three visual quality presets `LOW`, `STANDARD`, `HIGH`: share identical game state/input; alter only density/layer count/texture filtering permitted by the style contract. LOW retains flag/water and impact readability, removes expensive shadows and most ambience. STANDARD is the intended polished commercial baseline; HIGH adds carefully profiled ambient detail, not new physics.
- Provide **Reduce motion**: suppress background sway/camera decoration/repetitive sparkles without hiding critical landing/contact, hazard, slope, hit confirmation or cup state. Avoid bright flashing effects, especially spectator photography.
- Pixel unit proportions and landscape/Fold reflow stay consistent with the approved true overhead camera. Resize the HUD, not world collision geometry; cap simultaneous particles and offscreen emitters by measured gameplay profiler data (set numerical caps only after measurements).
- No per-frame allocations or unnecessary scene instantiation during each physics tick; prewarm/pool burst objects, cull offscreen ambience, group shader instances, and avoid unlimited ripple/leaf trail stacks.
- Background application/pause/resume and save/load must not create a storm of delayed event particles or mutate the native engine clock. On replay/seek, visual event identities must be stable or deduplicated.

## D. Acceptance and tests for the first 3 holes

| Test | Required evidence |
|---|---|
| **FX-01 Visual reference lock** | Captured before/after Hole 1 screenshots show unchanged frozen art/layout/sponsor placement and preserved pixel scale; additive VFX approved by owner |
| **FX-02 Idle life** | 20–30 second video from Hole 1 idle shows natural non-loop-obvious grass/flowers/flag/foliage and subtle water; no distracting clutter |
| **FX-03 Coherent wind** | Two authored course settings show L→R and R→L movement coherent across foliage/flag/reeds; wind directions documented |
| **FX-04 Contact authenticity** | Native-event-driven tee hit, fairway landing, repeated bounce, sand impact, water splash, cup/lip, actual tree-hit once source verified. Zero duplicate event bursts |
| **FX-05 Determinism** | Exact same original C++ input+PRNG sequence yields **identical native ball/scoring/replay hashes** at LOW/STANDARD/HIGH, Reduce motion ON/OFF and 30/60/90/120 render frame scheduling |
| **FX-06 Android performance** | Recorded FPS/frame time, particle draw/overdraw, memory and thermal soak on phone and Fold open/closed; no geometry/score differences |
| **FX-07 Pause and resume** | Repeated pause/background/restart during splash/landing and after scoring causes neither duplicate contacts nor stuck pending native events |
| **FX-08 Accessibility and legibility** | Putter/green slope arrows, meter/sweet zone, aim X, ball, cup and sponsor ads remain visible; flash reduced-motion setting verified |
| **FX-09 Three-hole reuse** | Exactly the same reusable material/FX system serves Hole 1, Hole 2 and Hole 3 with course-authored wind/material variants, no special-case physics |
| **FX-10 Approved assets only** | Asset hash/provenance and licensed particle/SFX manifest; no original copyrighted source media in final APK |

**Release gate:** All 10 FX checks and the parent [M1-11 approved art/HUD](MILESTONE_ONE_COMMERCIAL_ENGINE_HANDOVER_2026-10-12.md) plus [M1-14 performance/QA](MILESTONE_ONE_COMMERCIAL_ENGINE_HANDOVER_2026-10-12.md) must pass before effects are called production-ready. FX-04's source tree-hit case remains **blocked** with original tree collider. No shader quality pass should block deterministic core gameplay testing.

## E. Coordination with concurrent Work session

- **Work session owns:** frozen Hole 1 visual reference and hashes, `COURSE_PRODUCTION_STANDARD.md`, `COURSE_CATALOGUE_50.csv`, approved Pixtee assets and the three-hole construction preview. Do not alter/rename/copy these files from a different repository location without a verified handoff.
- **Engine branch owns:** C++ gameplay, source-proven collision data, Godot bridge semantics, event snapshots, performance/replay tests and integration contract.
- **Integrator should reconcile:** map the Work standard's particles/shaders/lighting already specified against sections A–D above; keep its approved decisions, adding only missing concrete interaction/quality gates. Reuse the approved art, including subtle grass/flowers/water, side-on birds, spectator movement and advertiser boards. Report any discrepancy rather than silently changing a frozen screenshot.

**Next effect implementation step:** produce one small, **visually approved** Hole 1 effects proof with wind-sway vegetation, slow-water shader, native ball altitude shadow and three event-driven contact bursts (grass, sand, water). Validate no physics change, then scale that system to Holes 2/3. **Do not spend time beautifying all 50 course concepts before the playable three-hole engine works.**
