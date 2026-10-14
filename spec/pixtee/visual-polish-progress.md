# Reference-quality execution ledger

Baseline: cb19300d7cc7abc113b1323e571c11d36e1824a0. Plan: docs/superpowers/plans/2026-10-14-pixtee-reference-quality-implementation.md. Feature: feat/pixtee-reference-quality-2026-10-14. Original sources and restoration checkpoint untouched.

A/B/C/D: in progress. W/E/F/G/H/I/J: pending.

Ruling: implement one uniform render-band hierarchy using native world-foot Y as object depth, preserving source sprite geometry. Shadow projection is separate derived geometry, not deformation of visible artwork. No 3D/voxel renderer.

Verification: VisualPolishCI asserts missing shared state/depth/fractional-motion contract before implementation. Existing native/Godot suite and actual GL captures remain required. Commercial-ready=false; APK disabled.
