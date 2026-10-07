# Sensi Golf Enhanced

A preservation-first modern port/remaster investigation for **Sensible Golf**.

> **Step 2: COMPLETE** — original PC v1.014 identified, fingerprinted and unpacked.  
> **Gate 1: GO / READY FOR FINAL SIGN-OFF** — the principal gameplay branches now have zero-tolerance original-machine-code parity.

## Project rule

No HD art, mobile UI or enhancement work begins until original gameplay behaviour is recovered and formally signed off.

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

## Gate 1 remaining sign-off

Gate 1 stays formally open until:

- the complete current-head parity suite is run as one consolidated sign-off;
- one end-to-end course/object interaction trace joins the recovered MAPI lookup/activation path to the already parity-tested interaction mutation;
- the final evidence record is frozen and Issue #1 is closed.

This is final verification/integration work, not a need to invent or substitute modern golf physics.

## Documentation

- [Canonical project status](docs/PROJECT_STATUS.md)
- [Gate 1 recovery gate](docs/GAMEPLAY_RECOVERY_GATE.md)
- [Golden-master harness and evidence](docs/GOLDEN_MASTER_HARNESS.md)
- [Recovered v1.014 physics](docs/RECOVERED_PHYSICS_1_014.md)
- [Executable-analysis record](docs/STEP3_EXECUTABLE_ANALYSIS.md)
- [Roadmap](docs/ROADMAP.md)
