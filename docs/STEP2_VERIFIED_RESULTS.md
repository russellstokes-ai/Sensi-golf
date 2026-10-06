# Step 2 — Verified Original PC Results

Status: **COMPLETE**

## Acquisition

The original Old-Games link was followed through its normal free-download sequence, including the advertised wait. That endpoint did not return ZIP bytes to the non-interactive runner.

The same preserved DOS package was then obtained from the public Internet Archive CORS URL exposed by a public game-preservation page. No access restriction was bypassed.

Archive:

- size: **1,287,227 bytes**
- SHA-256: `7685a9657c50fc2fad77d96d35c68d37b36f0d9cac13d3211727d1ab030b53df`

The game payload was analysed in temporary CI storage and was **not committed**.

## Selected reference build

Embedded strings identify the game as:

**Sensible Golf — Version 1.014**

Critical files:

| File | Size | CRC-32 | SHA-256 |
|---|---:|---|---|
| `GOLFDOS.EXE` | 582,895 | `45bcac33` | `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed` |
| `GOLFWIN.EXE` | 239,616 | `23c300b4` | `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8` |
| `GOLF.EPF` | 833,273 | `fbd76014` | `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e` |

This is **not identical** to the previously catalogued TDC 5865.0 executable build, although `GOLF.EPF` is byte-identical to that reference.

## GOLF.EPF

Our parser/decompressor successfully extracted:

- **277 / 277 entries**
- **250 compressed entries**
- no extraction failure

Major structural groups:

- `MAPM01.MAP` … `MAPM72.MAP`: 72 files, **7,848 bytes each**
- `MAPS01.MAP` … `MAPS72.MAP`: 72 files, **1,236–1,760 bytes**
- `MAPM01.SPT` … `MAPM72.SPT`: 72 files, **50 bytes each**
- 19 LBM graphics
- 15 MCH graphics/sprite resources
- 13 RAW files
- 5 DAT files
- 4 BIN files including `WOOD1.BIN` … `WOOD4.BIN`

The exact one-to-one numbering across 72 large maps, 72 small maps and 72 short SPT records strongly suggests a per-hole data model. The semantics of each field are still to be recovered rather than guessed.

## Windows executable

`GOLFWIN.EXE` is a six-section PE32 executable:

- `BEGTEXT`
- `DGROUP`
- `.bss`
- `.idata`
- `.reloc`
- `.rsrc`

The probe found **1,955 printable strings**.

Most importantly, the executable contains a debug-information interface with explicit internal-state labels:

- `HDBall X coord`
- `HDBall Y coord`
- `HDBall V force`
- `HDBall H force`
- `HDBall direction`
- `HDPlayer direction`
- `HDBall height`
- `HDBall pause`
- `HWSwing adjuster`
- `HWDrop Power`
- `DWball x position`
- `DWball y position`
- `DDball-hole in yards`
- `DWgreen x1/x2/y1/y2`
- `HDgreen offset`

These are direct navigation anchors for the actual physics/state code.

## DOS/Windows cross-check

The same labels occur in the DOS executable at a consistent **+368,364-byte file-offset shift** for this whole debug block. That is strong evidence that the two ports share a substantial common data organization and gives us a second implementation to cross-check.

## Wind discrepancy to investigate

The same debug block also contains:

- `DWWind amount`
- `HDWind adjustment`
- `HDWind x adj`
- `HDWind y adj`

Earlier manual-level evidence indicates Sensible Golf does not expose wind as a gameplay consideration. Therefore we no longer state simply that “the engine has no wind.” The binary proves wind-named internal state exists.

Possible explanations include inactive/vestigial code, an unreleased feature, AI/animation use, or an internal adjustment not exposed to the player. Runtime/code-reference evidence will decide this.

## Recovery outlook

This result upgrades the project materially:

**Gameplay recovery remains GO, with higher confidence.**

We now have:
- exact executable identities;
- a completely extractable data archive;
- repeated per-hole data structures;
- two PC executables for cross-checking;
- explicit debug labels naming core ball forces and coordinates.

Step 3 can now trace those labels to the code that reads the associated variables and recover one complete shot path.
