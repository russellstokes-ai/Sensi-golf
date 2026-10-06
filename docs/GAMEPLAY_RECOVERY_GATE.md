# Gate 1 — Original Gameplay and Physics Recovery

Status: **GO — core normal-shot physics parity proven; terrain/rules parity remains**

## Purpose

Before HD graphics, mobile presentation or enhancement features, prove that original Sensible Golf gameplay can be recovered faithfully enough for a modern port.

The central feasibility question is now answered positively. The selected PC v1.014 build is fully accessible to the analysis harness, and representative normal shots reproduce original live-player machine code through final rest at zero tolerance.

## Recovery checklist

1. **Shot aiming / heading** — recovered and parity-tested.
2. **Welly power** — raw captured-power path parity-tested; complete user-facing meter timing mapping still needs closure.
3. **Accuracy / draw / fade** — recovered profile selection, power penalty, heading and per-tick curve; straight/draw/fade full-shot cases parity-tested.
4. **Club table** — 13 physics records recovered; normal-club path parity-tested; putter special path remains.
5. **Lie/surface penalties** — profile selection is recovered; complete surface semantics still open.
6. **Airborne movement** — parity-verified for representative normal shots.
7. **Tree/obstacle collision** — open.
8. **Landing, bounce and roll** — parity-verified on generic flat surface.
9. **Water/out-of-bounds** — open.
10. **Putting / green slope** — substantial static recovery, runtime parity open.
11. **Hole/cup capture** — open.
12. **Randomness** — final classification pending.
13. **Logical simulation timing** — logical update sequence is reproduced; wall-clock tick frequency still pending.
14. **Golden-master runtime parity** — achieved for representative generic-flat normal shots; expand across game-rule branches before closing Gate 1.

## Verified full-shot parity

Successful workflow run: **37523005402**

Three representative live-player shots matched original v1.014 at zero tolerance from launch through final rest:

- straight 1W — 153 samples;
- draw mid-club shot — 131 samples;
- fade high-club shot — 111 samples.

Every sample matched X/Y, height, vertical/horizontal force, direction, swing adjuster and adjusted power. Landing/rest events matched exactly.

This is materially stronger than visual or endpoint similarity: the recovered core follows the same logical state trajectory.

## Verified reference build

- `GOLFWIN.EXE` SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`
- `GOLFDOS.EXE` SHA-256 `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed`
- `GOLF.EPF` SHA-256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`

## Remaining gate

Gate 1 remains open until the same evidence standard covers the important non-generic branches: real terrain/lie effects, hazards/collisions, green/putting and cup capture.

No modern physics substitute is permitted for unresolved behavior.
