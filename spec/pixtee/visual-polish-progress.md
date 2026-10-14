# Reference-quality execution ledger

Baseline: cb19300d7cc7abc113b1323e571c11d36e1824a0. Plan: docs/superpowers/plans/2026-10-14-pixtee-reference-quality-implementation.md. Feature: feat/pixtee-reference-quality-2026-10-14. Original sources and restoration checkpoint untouched.

A/B/C/D: in progress. W/E/F/G/H/I/J: pending.

Ruling: implement one uniform render-band hierarchy using native world-foot Y as object depth, preserving source sprite geometry. Shadow projection is separate derived geometry, not deformation of visible artwork. No 3D/voxel renderer.

Verification: VisualPolishCI asserts missing shared state/depth/fractional-motion contract before implementation. Existing native/Godot suite and actual GL captures remain required. Commercial-ready=false; APK disabled.

A/B/C/D first implementation: 3fdb8d298547fd2b8e6990ddb2bd2c0d08c67c6c; scene/render run38606346476 SUCCESS, all28 native tests and VisualPolishCI passed. Actual river/caddy captures saved in work/*-quality-a.png. Visual tuning and motion acceptance still pending; do not mark reference quality achieved.

W: native kernel calibrated at 96 units/tick/mph: 8mph crosswind drift0.080830 of calm airborne carry. Source exact zero-wind trace test passes. Runtime integration test exposed shot reset discarding wind configuration; fixed by retaining hole configuration and reapplying it on each source shot reset. Native session midflight replay test passes. Bridge/HUD/profile migration pending Godot suite.

Ruling: local Godot process crashes in this sandbox; use actual Linux Godot4.5 GL CI for scene validation. Do not infer a game defect or desktop acceptance from that local launch limitation. Native C++ tests run locally.
`nW verified: 8c898891957c9cee79480694db895f955a82bf50, run38615586729 SUCCESS. Shared wind HUD/direction, next-round settings and exact midflight replay pass. F/H/I incremental: 8516a4385d2d15721d0ba99d6548724a0643b8fb run38615971571 SUCCESS; 29 native tests, shared freeze/layered-climb contract, full Godot scene suite and actual GL captures pass. F/H/I remain incomplete: frog/dragonfly routes, animation refinement, menu finish and visual motion acceptance. Current native calm captures confirm reference-quality gap remains; tests are not visual acceptance.
