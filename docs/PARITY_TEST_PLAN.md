# Gameplay Parity Test Plan

## Objective

Prove that the portable core reproduces original Sensible Golf behaviour closely enough that classic mode is the same game rather than an approximation.

## Test dimensions

### Swing
- minimum, quarter, half, three-quarter and maximum useful power;
- centre accuracy;
- symmetric early/late accuracy inputs;
- repeatability of identical inputs.

### Clubs
At minimum sample:
- 1 Wood;
- a mid wood;
- low iron;
- mid iron;
- high iron/wedge;
- putter.

### Lies/surfaces
- tee;
- fairway;
- poor/recessed lies identified by the game;
- bunker;
- green;
- water/out-of-bounds boundary cases.

### Environment / geometry
- flat landing area;
- green slope;
- obstacle/tree interaction;
- hazard boundaries.

**No wind cases:** the original manual explicitly describes Sensible Golf as not modelling wind, so wind must not be invented in classic-mode parity tests.

## Trace schema

Each controlled shot should record:

```json
{
  "build": "dos-or-win-version",
  "course": "identifier",
  "hole": 1,
  "start": {"x": 0, "y": 0},
  "aim": 0,
  "club": "1W",
  "lie": "tee",
  "power_tick": 0,
  "accuracy_tick": 0,
  "trajectory": [],
  "landing": {"x": 0, "y": 0},
  "rest": {"x": 0, "y": 0},
  "strokes": 1
}
```

Coordinates/ticks are placeholders until native units are identified.

## Pass criteria

Before Gate 1 closes:
- heading/sign matches for all representative cases;
- power-to-distance curve matches within a documented tolerance;
- draw/fade direction and practical magnitude match;
- club trajectory classes match;
- lie penalties match;
- hazard/collision outcomes match;
- green slope response matches;
- landing and final rest positions match within agreed tolerance;
- repeated deterministic shots remain deterministic.

Tolerance must be based on native game units once recovered, not arbitrary screen pixels.
