# Step 2 — PC Input Inventory

Status: **COMPLETE**

The original PC data-ingestion gate has been completed against the selected preserved build. This document records the reproducible pipeline; authoritative results are in [STEP2_VERIFIED_RESULTS.md](STEP2_VERIFIED_RESULTS.md).

## Selected parity/reference build

Embedded strings identify the ingested PC release as:

**Sensible Golf — Version 1.014**

Verified critical files:

| File | Size | CRC-32 | SHA-256 |
|---|---:|---|---|
| `GOLFDOS.EXE` | 582,895 | `45bcac33` | `14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed` |
| `GOLFWIN.EXE` | 239,616 | `23c300b4` | `3ab09a789ae3d11ffe6636def32f5d1c00f2930ad4068bf6dc1420ceac7f1ec8` |
| `GOLF.EPF` | 833,273 | `fbd76014` | `58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e` |

The selected build resolves the earlier 582,895-vs-582,975 DOS executable discrepancy. The selected parity package uses the **582,895-byte** executable. Its `GOLF.EPF` is byte-identical to the previously catalogued TDC reference archive.

## Archive ingestion result

`GOLF.EPF`:

- **277 entries**
- **250 compressed**
- **277/277 extracted successfully**
- no encryption
- big-endian dynamic-width LZW successfully decoded

Major recovered data families:

- `MAPM01.MAP` … `MAPM72.MAP`: 72 × 7,848 bytes
- `MAPS01.MAP` … `MAPS72.MAP`: 72 smaller map records
- `MAPM01.SPT` … `MAPM72.SPT`: 72 × 50-byte records
- 19 LBM graphics resources
- 15 MCH graphics/sprite resources
- 13 RAW resources
- 5 DAT files
- 4 BIN files including `WOOD1.BIN` … `WOOD4.BIN`

The one-to-one 72-file numbering across the map/SPT families is treated as structural evidence. Field semantics are promoted only when supported by code/data analysis.

## Acquisition and public-repo handling

The build was fetched by a disposable GitHub Actions analysis runner from a public preservation CORS URL. No access control was bypassed.

The original commercial ZIP, executables, EPF archive and extracted copyrighted payload were **not committed**. Only fingerprints, structural metadata, clean tooling and derived recovery findings are retained in this repository.

## Reproducible tooling

- `tools/hash_inputs.py` — SHA-256 manifest
- `tools/verify_reference_build.py` — historical-build comparison
- `tools/epf_inspect.py` — EPF inventory
- `tools/epf_extract.py` — EPF decompression/extraction
- `tools/ingest_pc_build.py` — complete Step-2 pipeline

Re-run on a legal local copy:

```bash
python tools/ingest_pc_build.py /path/to/SENSEGOLF.ZIP
```

Private output remains under ignored `analysis/private/`.

## Step 2 exit criteria

- [x] Original PC archive fingerprinted
- [x] Exact selected executable build identified
- [x] DOS and Windows executables identified
- [x] `GOLF.EPF` inventoried
- [x] EPF compression decoded
- [x] 277/277 EPF entries extracted
- [x] Major data families classified
- [x] Original payload excluded from the public repository
- [x] Metadata-only findings documented
- [x] Analysis is reproducible through repository tooling

**Step 2 is closed.**
