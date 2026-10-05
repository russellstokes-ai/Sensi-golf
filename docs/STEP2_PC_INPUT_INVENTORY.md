# Step 2 — PC Input Inventory

Status: **PARTIAL — external reference inventory established; local binary acquisition still pending.**

## What is externally verified

Internet Archive identifies the preserved DOS item as `msdos_Sensible_Golf_1994`, marks it access-restricted/stream-only, and launches it with:

```
SensGolf/golfdos.exe
```

A preservation launcher project independently records the archive URL as:

```
https://archive.org/download/msdos_Sensible_Golf_1994/Sensible_Golf_1994.zip
```

The Total DOS Collection metadata database contains a detailed PC build inventory (TDC id 5865.0). The critical files in that reference build are:

| File | Size | CRC-32 | Role |
|---|---:|---:|---|
| GOLF.EPF | 833,273 | fbd76014 | Main East Point archive; first data target |
| GOLFDOS.EXE | 582,975 | c070f837 | DOS4GW/LE gameplay executable |
| GOLFWIN.EXE | 239,616 | 4c11d6b0 | PE32 Windows gameplay executable |
| GOLF.HMP | 47,218 | 28fae5d2 | HMI music |
| GOLF.MID | 47,337 | 1ecd328d | MIDI music |
| EPSMIX32.DLL | 6,144 | fe73806a | Windows audio/support library |
| WINGPAL.WND | 5,024 | a2b26dc7 | Windows graphics/palette support data |

The complete reference manifest is stored in `reference/pc_build_5865.0.json`.

## Important build discrepancy

A separate executable-index source reports `GOLFDOS.EXE` as 582,895 bytes while the TDC reference reports 582,975 bytes. `GOLFWIN.EXE` is reported as 239,616 bytes by both.

Therefore we must **not assume there is only one PC executable build**. When the local archive is obtained, its hashes and file sizes become the authoritative analysis identity.

## EPF status

The EPF format itself is solved/documented:

- signature `EPFS`;
- 11-byte header;
- FAT at an explicit offset;
- 13-byte null-terminated filenames;
- compression flag;
- compressed/decompressed sizes;
- no encryption;
- LZW compression with dynamic 9–14 bit codes.

The repo already contains `tools/epf_inspect.py` to inventory the archive once `GOLF.EPF` is present.

## Build verification

Run:

```bash
python tools/verify_reference_build.py original/dos
```

A non-zero exit is expected for a different build. A mismatch is evidence to record, not a reason to alter files.

Then hash the actual inputs independently:

```bash
python tools/hash_inputs.py original/dos -o analysis/input-sha256.json
```

## Physics facts already established from the manual

These are behavioural constraints, not recovered implementation details:

- The game deliberately does **not** model wind.
- Lie quality reduces achievable shot distance.
- Maximum-power 1 Wood is documented as 240 yards.
- The lower timing zone controls straight/draw/fade.
- Woods are low/far; e.g. a 7 iron is suggested when height is needed over trees.
- Putting keeps the ball on the green surface and green slope arrows affect direction/speed.

## Step 2 exit condition

Step 2 is complete only after a legally held local PC package has been:
1. byte-hashed;
2. matched or distinguished from the known reference build;
3. inventoried;
4. `GOLF.EPF` enumerated;
5. extracted to a non-repository analysis workspace.

Until then, all build-specific claims remain reference metadata rather than local verification.
