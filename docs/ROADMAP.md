# Roadmap

Canonical current snapshot: [PROJECT_STATUS.md](PROJECT_STATUS.md).

## Gate 1 — Recover original gameplay/physics

**COMPLETE — GO**

Completed:

- public repo / original-file separation;
- exact PC v1.014 reference build fingerprinting;
- complete EPF extraction (277/277 entries);
- executable map and DOS/Windows cross-check anchors;
- authoritative ball-state layout and fixed-point coordinates;
- exact 4096-step Q14 trig model;
- 13-row club physics table and live launch path;
- Welly power range and user-facing timer/dispatcher path;
- accuracy/profile mapping plus straight/draw/fade behaviour;
- clear-air flight;
- landing, bounce, roll and final rest;
- ordinary surface classes;
- water / NO GO / out-of-bounds;
- club-12 putting;
- green slope projection;
- cup/hole terminal branch;
- original 16-bit ranged PRNG;
- PRNG-driven interaction state mutations;
- course MAPI collision lookup with 81,920 zero-tolerance cases;
- logical timer source and approximately 70.02 Hz callback cadence;
- extensive original-machine-code golden masters.

Closure evidence:

- end-to-end course interaction run `37603087138` — PASS;
- consolidated Gate-1 run `37603222703` — PASS;
- CI on the sign-off commit `37603222791` — PASS;
- durable evidence stored under `analysis/evidence/`.

Exit: **achieved on 2026-10-07**.

## Gate 2 — Complete classic game/core integration

**ACTIVE PHASE**

Progress:

- [x] platform-neutral `IClassicModel` backed only by Gate-1 recovered mechanics;
- [x] original MAPM/MAPI/SPT course/resource model;
- [x] per-player tee and cup coordinates from original SPT data;
- [x] moving-ball terrain lookup from recovered 16.16 coordinates;
- [x] initial hole/session lifecycle and stroke counter;
- [x] explicit unsupported-rule boundary;
- [x] putter/cup-edge code-8/9/10 terminal branches;
- [ ] deterministic PRNG/session ownership;
- [x] hazard pause and original position recovery;
- [ ] hazard penalty/scoring ownership where applicable;
- [~] scoring/stroke ownership traced; session integration in progress;
- [ ] completed-hole and next-hole transition;
- [ ] input replay and versioned save/resume.

Exit: a complete original hole can be played deterministically through the platform-neutral game layer.

## Gate 3 — Android playable classic build

Deliverables:

- Android NDK/JNI host for the C++ classic game layer;
- landscape-first touch UI;
- three-click Welly controls and aiming;
- gamepad support;
- original-course rendering;
- saves/resume and deterministic replay;
- complete round on real Android hardware.

Exit: a complete classic round is playable on Android with Gate-1 parity intact.

## Gate 4 — Modern / HD presentation

Deliverables:

- resolution-independent widescreen composition;
- 60/90/120 Hz rendering interpolation above the ~70.02 Hz classic simulation;
- scalable/redrawn asset pipeline;
- enhanced animation/effects/audio presentation;
- original-vs-enhanced visual mode where practical.

Exit: modern presentation does not alter simulation results.

## Gate 5 — Full mobile remaster polish

Deliverables:

- redrawn sprite/tile/UI set;
- modern animation/effects;
- audio presentation upgrade;
- polished mobile UX.

## Gate 6 — Enhanced edition

Potentially:

- iOS;
- new courses;
- career/tournaments;
- achievements/statistics;
- optional enhanced rules/physics;
- multiplayer.

Classic mode remains preserved.
