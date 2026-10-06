# Roadmap

## Gate 1 — Recover original gameplay/physics
**Current**

Completed infrastructure:
- public repo / original-file separation;
- known PC build reference manifest;
- safe private ingestion pipeline;
- EPF parser/decompressor/extractor;
- executable metadata/string/constant probe;
- shot-trace schema and numeric comparator;
- portable C++17 classic-core contract and trace recorder.

Evidence still required from the original game:
- ingest exact original-file set;
- executable map;
- recovered gameplay tables/routines;
- controlled original-game shot traces;
- deterministic recovered implementation;
- parity report.

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
