# Golden Master — Predictor Baseline

Status: **PARITY VERIFIED FOR INTERNAL PREDICTOR ONLY; NOT A LIVE-SHOT RESULT**

The first zero-tolerance oracle run executed the original v1.014 routine at `0x40C84F` plus the normal clear-air path and matched the portable math for three controlled cases.

Subsequent call-site analysis showed that `0x40C84F` belongs to an internal shot prediction/simulation path. It must therefore **not** be cited as live-player launch parity.

That result remains useful as an independent cross-check of club scaling and airborne integer arithmetic, but the live-player launch is the routine at `0x40AD1D`.

The live-player golden-master workflow now targets `0x40AD1D` directly with real inputs:

- club index;
- lie/surface selector slot;
- Welly power;
- accuracy meter tick;
- player heading.

No live gameplay claim is promoted until that workflow passes.
