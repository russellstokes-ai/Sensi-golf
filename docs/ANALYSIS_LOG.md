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
- A TDC reference build exists as id `5865.0`.
- Reference metadata established likely PC files and hashes before local ingestion.

### Verified from original/manual descriptions
- The released gameplay is described without active wind.
- Ball lie can reduce shot distance.
- 1 Wood maximum-power example is 240 yards.
- Draw/fade is controlled by the lower Welly-o-meter timing zone.
- Woods travel far/low; irons can provide higher trajectory.
- Putting is surface-bound and green slope affects direction/speed.

## 2026-10-06 — Original PC payload successfully ingested

### Verified input identity
The public preservation PC archive was fetched on a disposable GitHub Actions runner and analyzed without committing original payloads.

- `GOLFDOS.EXE`
  - size 582895
  - CRC-32 `45bcac33`
  - SHA-256 `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed`
- `GOLFWIN.EXE`
  - size 239616
  - CRC-32 `23c300b4`
  - SHA-256 `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8`
  - internal strings identify version 1.014
- `GOLF.EPF`
  - size 833273
  - CRC-32 `fbd76014`
  - SHA-256 `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e`
  - 277 entries, 250 compressed, all 277 extracted successfully.

This resolves the earlier 582895-vs-582975 DOS executable discrepancy: the selected parity/reference package contains the 582895-byte DOS build.

## 2026-10-06 — Windows v1.014 physics recovery

Detailed record: `docs/RECOVERED_PHYSICS_1_014.md`.

### Observed
- Ball structure stride is `0x2C` bytes.
- Ball X/Y are dword fixed-point-style coordinates; high words are used as integer map coordinates.
- Ball fields recovered: X, Y, vertical force, horizontal force, direction, height, distance-to-hole, pause, plus terrain-tail fields.
- Direction is masked to 12 bits: 4096 angular units/circle.
- Horizontal projection uses signed Q14 trig.
- Original sine lookup is regenerated exactly by `trunc(sin(2*pi*i/4096)*16384)` with minimum clamped to -16383; 5120 compared entries produced zero mismatches.
- Club physics table contains 13 records, 12 bytes each, at Windows VA `0x41F108`.
- Club record contains vertical base, horizontal base and power scale; first two are halved when loaded.
- Club index 12 uses the special zero-vertical putter path.
- Launch force uses `loaded_base + power_scale * DropPower`.
- Direction adjustment uses `direction -= 2 * swing_adjuster`, masked to `0xFFF`.
- 11 swing/accuracy profile tables were extracted at `0x41F204`, 28 bytes each, with signed symmetric adjustment values around a zero centre.
- Normal airborne gravity is `0x2100` raw force units per logical update.
- Normal rolling drag is `0xF00`; green rolling drag is `0x780`.
- Green mode is an explicit state determined by recovered green bounds.
- Bounce reverses/halves vertical force and transfers half of that rebound into horizontal force.
- Terrain lookup supplies direction/magnitude-like fields that are projected with the same Q14 trig machinery for slope movement.
- Wind-named debug globals exist and X/Y wind values are computed, but static xref analysis finds those X/Y values written and not otherwise read in this Windows build. Treat them as vestigial/debug unless runtime evidence contradicts this.

### Still open
- user-facing meter timing -> raw `DropPower`;
- exact club display-name mapping for indices 0–11;
- exact mapping of club/player state to the 11 swing profiles;
- whether draw/fade has any additional per-tick curvature beyond recovered launch-direction adjustment;
- original logical tick frequency;
- meanings/effects of all terrain IDs and lie classes;
- cup-capture/special near-hole branch semantics;
- runtime golden-master trace and numerical parity.

### Rule
Do not promote static recovery to `parity-verified` until a runtime original-game trace is reproduced by the portable core.

## 2026-10-06 — Live launch-to-rest golden-master parity

### Parity-verified
GitHub Actions run `37523005402` executed original Windows v1.014 live-player machine code under Unicorn and compared it with the portable recovered core at zero tolerance on a controlled generic flat surface.

- straight 1W: 153 samples, 0 mismatches
- draw mid shot: 131 samples, 0 mismatches
- fade high shot: 111 samples, 0 mismatches

Compared X/Y, height, vertical force, horizontal force, direction, swing adjuster and adjusted power at every logical sample. Landing and final-rest events also matched exactly.

The draw case exposed and fixed a subtle original branch: when positive horizontal force is reduced below zero by drag during an airborne tick, the original sets H=0 and ends that tick without applying the direction/curve update. The portable core now preserves that behavior.

Scope limitation: terrain lookup was controlled to generic surface code 0. Real terrain, hazards, putting/green and cup-capture branches remain to be parity-verified.
