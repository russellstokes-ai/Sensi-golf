# Roadmap

## Gate 1 — Recover original gameplay/physics
**Current — GO; runtime parity remains**

Completed:
- public repo / original-file separation;
- exact PC v1.014 reference build fingerprinting;
- complete EPF extraction (277/277 entries);
- executable map and DOS/Windows cross-check anchors;
- ball-state layout;
- exact direction/trig model;
- club physics table and launch formula;
- swing/accuracy tables;
- gravity, landing/bounce and rolling drag;
- green-state and slope projection path;
- portable C++17 recovered math/kernel scaffolding;
- trace schema and numerical comparator.

Still required:
- close remaining meter/profile mapping details;
- finish lie/surface, collision/hazard and cup semantics;
- determine logical simulation tick frequency and randomness behaviour;
- capture controlled original-game runtime traces;
- complete recovered implementation for exercised paths;
- pass the representative golden-master parity suite.

Exit: representative parity suite passes.

## Gate 2 — Faithful desktop harness

Deliverables:
- recovered classic implementation compiled outside DOS/Windows legacy runtime;
- one full original hole/course path through importer;
- original-resolution reference renderer;
- input replay and deterministic save/load tests.

Exit: full round can be played with classic behaviour.

## Gate 3 — Modern renderer

Deliverables:
- resolution-independent coordinates;
- widescreen composition;
- 60/120 fps presentation interpolation;
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
