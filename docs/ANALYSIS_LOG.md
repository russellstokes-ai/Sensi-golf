# Analysis Log

Use this file for durable findings. Distinguish **observed**, **verified**, and **hypothesis**.

## 2026-10-05 — Repository bootstrap

### Verified
- Recovery is treated as a hard project gate before enhancement work.
- EPF inventory parser and unit tests are present.
- Public repository excludes original commercial binaries/assets by policy.

## 2026-10-05 — Step 2 external PC inventory

### Verified from independent public metadata
- Internet Archive item identifier: `msdos_Sensible_Golf_1994`.
- Internet Archive emulator start path: `SensGolf/golfdos.exe`.
- The Internet Archive item is marked access-restricted / stream-only.
- A TDC reference build exists as id `5865.0`.
- That build contains `GOLF.EPF`, `GOLFDOS.EXE`, and `GOLFWIN.EXE`.
- TDC reference values:
  - `GOLF.EPF`: 833273 bytes, CRC-32 `fbd76014`
  - `GOLFDOS.EXE`: 582975 bytes, CRC-32 `c070f837`
  - `GOLFWIN.EXE`: 239616 bytes, CRC-32 `4c11d6b0`
- A separate executable index reports `GOLFDOS.EXE` as 582895 bytes, so PC build/version variation or metadata inconsistency must be resolved from the local copy.

### Verified from original manual text
- No wind simulation.
- Ball lie can reduce shot distance.
- 1 Wood maximum-power example is 240 yards.
- Draw/fade is controlled by the lower Welly-o-meter timing zone.
- Woods travel far/low; irons can provide higher trajectory.
- Putting is surface-bound and green slope affects direction/speed.

### Locally unverified
- Exact binary build to use as the parity reference.
- Local SHA-256 hashes.
- Internal `GOLF.EPF` filenames/content.
- Club/trajectory tables.
- Shot power mapping.
- Accuracy/draw/fade mapping.
- Ball-state update loop.
- Terrain/lie modifiers.
- Putting/green slope implementation.
- PRNG usage, if any.

### Rule
Do not upgrade a hypothesis to verified until supported by binary/data evidence or repeatable original-game measurements.
