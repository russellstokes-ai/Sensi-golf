# Sensi Golf Enhanced

A preservation-first modern port/remaster investigation for **Sensible Golf**.

> **Step 2: COMPLETE** — original PC v1.014 identified, fingerprinted and unpacked.  
> **Gate 1: COMPLETE / GO** — consolidated zero-tolerance parity sign-off passed against original Windows v1.014.

## Project rule

Original gameplay behaviour is now recovered and formally signed off. Classic mode must remain locked to the Gate-1 parity evidence. Development now moves through a platform-neutral classic game/session layer, with Android as the first actual playable product target.

## Current recovery state

The project is no longer at the “can we recover the physics?” stage. The answer is **yes**.

The portable core now reproduces original Windows v1.014 behaviour across:

- straight, draw and fade shots;
- launch, clear-air flight, landing, bounce, roll and final rest;
- skirt, fairway, semi rough, rough, very rough and sand;
- water, NO GO and out-of-bounds;
- club-12 putting;
- green slope movement;
- hole/cup capture;
- the original ranged PRNG and three PRNG-driven shot-interaction fragments;
- the original course MAPI collision lookup, exhaustively compared over 81,920 cases.

The logical gameplay timer has also been traced to the Windows wall clock. The original callback interval is `0x3A8` in 16.16-second units, approximately **70.02 Hz**.

## Verified original PC reference

- Sensible Golf Windows v1.014
- `GOLFWIN.EXE` SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`
- `GOLFDOS.EXE` SHA-256 `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed`
- `GOLF.EPF` SHA-256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`
- `GOLF.EPF`: **277/277 entries extracted**

Original commercial payloads remain outside the public repository.

## Gate 1 closure

Gate 1 closed on **2026-10-07**.

Final consolidated sign-off:

- commit: `34bfa1c0999d2e5e196ce900edc9e2bd563ed3d8`
- Gate 1 workflow: `37603222703` — **PASS**
- CI: `37603222791` — **PASS**
- end-to-end course interaction workflow: `37603087138` — **PASS**
- course MAPI lookup cases: **81,920**, zero tolerance
- durable evidence: `analysis/evidence/gate1_final_signoff.json`

**Gate 2 — complete classic game/core integration — is in progress. Android is the first playable product target.**

Current Gate-2 progress includes the production `ClassicShotModel`, real MAPM/MAPI/SPT course loading, recovered per-player tee/cup coordinates and the first platform-neutral hole/session state machine. Putter cup-edge terminal rules and original hazard position recovery are now integrated. The current focus is the remaining score/stroke ownership and the distinction between cup capture, completed/scored hole state and next-hole transition.

## Documentation

- [Canonical project status](docs/PROJECT_STATUS.md)
- [Gate 1 recovery gate](docs/GAMEPLAY_RECOVERY_GATE.md)
- [Golden-master harness and evidence](docs/GOLDEN_MASTER_HARNESS.md)
- [Recovered v1.014 physics](docs/RECOVERED_PHYSICS_1_014.md)
- [Executable-analysis record](docs/STEP3_EXECUTABLE_ANALYSIS.md)
- [Gate 2 mobile integration plan](docs/GATE2_MOBILE_INTEGRATION.md)
- [Roadmap](docs/ROADMAP.md)
