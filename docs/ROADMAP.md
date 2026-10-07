# Roadmap

Canonical current snapshot: [PROJECT_STATUS.md](PROJECT_STATUS.md).

## Gate 1 — Recover original gameplay/physics

**Current: GO — ready for final sign-off**

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

Final sign-off work:

- retain one end-to-end course/object interaction activation trace;
- run the complete current-head parity/core suite as one sign-off;
- freeze the final Gate 1 evidence ledger and close Issue #1.

Exit: the consolidated suite is green and Issue #1 is closed.

## Gate 2 — Faithful desktop harness

Deliverables:

- recovered classic implementation compiled outside DOS/Windows legacy runtime;
- one full original hole/course path through importer;
- original-resolution reference renderer;
- classic input/meter layer using the recovered timing semantics;
- input replay and deterministic save/load tests.

Exit: a full round can be played with classic behaviour.

## Gate 3 — Modern renderer

Deliverables:

- resolution-independent coordinates;
- widescreen composition;
- 60/90/120 Hz presentation interpolation above the classic simulation cadence;
- scalable asset pipeline;
- original-vs-enhanced visual toggle.

Exit: modern visuals do not alter simulation results.

## Gate 4 — Android playable

Deliverables:

- landscape mobile shell;
- touch aiming and three-click controls;
- gamepad support;
- saves/resume;
- all classic gameplay paths.

Exit: complete round playable on real Android hardware with parity intact.

## Gate 5 — HD remaster

Deliverables:

- redrawn sprite/tile/UI set;
- modern animation/effects;
- audio presentation upgrade;
- polished mobile UX.

## Gate 6 — Full enhanced edition

Potentially:

- iOS;
- new courses;
- career/tournaments;
- achievements/statistics;
- optional enhanced rules/physics;
- multiplayer.

Classic mode remains preserved.
