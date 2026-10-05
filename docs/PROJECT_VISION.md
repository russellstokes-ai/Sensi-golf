# Project Vision

## Goal

Create a faithful modern **Sensible Golf Enhanced** port/remaster that preserves the original game's feel and content while replacing obsolete platform technology.

## Preservation first

The original shot model is the product. Rendering, input, audio and presentation may be modernized, but gameplay changes must be optional and layered above a verified classic mode.

## Non-negotiables

- Classic gameplay remains deterministic and independently testable.
- Simulation does not depend on display frame rate.
- Renderer consumes simulation state; renderer never drives physics.
- Original course geometry is preserved for classic mode.
- Mobile controls must map cleanly onto the original aiming/power/accuracy loop.
- HD assets must not silently alter collision or gameplay geometry.
- Every recovered mechanic is documented and covered by parity evidence.

## Enhanced edition direction

Only after parity:
- high-resolution 2D/2.5D presentation;
- 60/120 fps interpolation without changing simulation ticks;
- modern widescreen/mobile composition;
- touch, gamepad and keyboard input;
- improved animation, effects and audio presentation;
- Android and iOS packaging;
- optional new courses, modes, achievements and online systems.

Classic mode remains the reference implementation.
