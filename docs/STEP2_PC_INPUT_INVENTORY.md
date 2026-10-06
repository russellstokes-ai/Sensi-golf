# Step 2 — PC Input Inventory

Status: **TOOLING COMPLETE / LOCAL PAYLOAD INGESTION PENDING ACCESS TO A LEGAL COPY**

## Outcome

The Step-2 pipeline is complete. A legal PC ZIP or extracted installation can now be ingested with one command; it will be fingerprinted, compared to the known reference build, its executables identified, `GOLF.EPF` inventoried and decompressed, and metadata-only reports produced.

The public repository does not contain the original game.

## Sources checked

### Internet Archive
Item: `msdos_Sensible_Golf_1994`

Verified public metadata:
- access-restricted / stream-only;
- DOSBox emulator;
- emulator input is a ZIP;
- start path is `SensGolf/golfdos.exe`.

The item can be played in-browser but the underlying ZIP is not openly downloadable from this analysis environment. No access controls were bypassed.

### Old-Games.com
The title page exposes:
- original download labelled `SENSEGOLF.ZIP` at about 2.51 MB;
- two Easy Setup packages.

The free-download flow is timer/session gated and does not expose the binary to this environment. No attempt was made to bypass that control.

### My Abandonware
The site exposes a DOS download of roughly 1 MB and in-browser play. Its download endpoint intentionally requires a normal browser-page/referrer flow and redirects automated retrieval. No bypass was attempted.

### The Old Computer
A `Sensible Golf (1994)(Avalon Interactive).zip` entry exists at about 1.23 MB, but download access requires an authenticated/upgraded account.

## Known PC reference build

The Total DOS Collection metadata database records build `5865.0` with a detailed inventory. Critical reference files:

| File | Size | CRC-32 | Role |
|---|---:|---:|---|
| GOLF.EPF | 833,273 | fbd76014 | Main East Point archive; first data target |
| GOLFDOS.EXE | 582,975 | c070f837 | DOS4GW/LE gameplay executable |
| GOLFWIN.EXE | 239,616 | 4c11d6b0 | PE32 Windows gameplay executable |
| GOLF.HMP | 47,218 | 28fae5d2 | HMI music |
| GOLF.MID | 47,337 | 1ecd328d | MIDI music |
| EPSMIX32.DLL | 6,144 | fe73806a | Windows audio/support library |
| WINGPAL.WND | 5,024 | a2b26dc7 | graphics/palette support data |

Full reference: `reference/pc_build_5865.0.json`.

A separate executable index reports `GOLFDOS.EXE` as 582,895 bytes rather than 582,975. This is why the actual supplied copy must be hashed before reverse engineering; there may be multiple PC builds.

## EPF status

The EPF format is fully documented and is not encrypted:

- `EPFS` signature;
- 11-byte header;
- FAT offset in header;
- 8.3 filenames;
- compression flag;
- compressed/decompressed sizes;
- big-endian LZW;
- dynamic 9–14 bit code width;
- highest code = EOF;
- second-highest = dictionary reset;
- code width remains unchanged after reset.

Repository tooling:
- `tools/epf_inspect.py` — inventory;
- `tools/epf_extract.py` — extract/decompress;
- `tools/ingest_pc_build.py` — complete Step-2 pipeline.

## One-command ingestion

For a legal ZIP:

```bash
python tools/ingest_pc_build.py /path/to/SENSEGOLF.ZIP
```

For an already extracted installation:

```bash
python tools/ingest_pc_build.py /path/to/SensGolf
```

Default private output:

```
analysis/private/step2/
  input/
  extracted/epf/
  reports/
    input_manifest.json
    reference_comparison.json
    epf_inventory.json
    epf_extracted_manifest.json
    step2_report.json
    step2_report.md
```

That directory is ignored by Git.

## Physics facts already established from original/manual descriptions

These are behavioural constraints, not recovered implementation details:

- no wind simulation;
- lie quality reduces achievable shot distance;
- maximum-power 1 Wood is documented as 240 yards;
- the lower timing zone controls straight/draw/fade;
- woods are low/far while irons can provide higher trajectory;
- putting remains on the green surface and green slope arrows affect direction/speed.

## Step 2 completion definition

### Engineering/tooling: COMPLETE
- reference manifest pinned;
- build verifier implemented;
- SHA-256/CRC inventory implemented;
- EPF parser implemented;
- EPF LZW decompressor/extractor implemented;
- private end-to-end ingestion pipeline implemented;
- synthetic unit tests added;
- original payload exclusion enforced in `.gitignore`.

### Evidence ingest: PENDING LEGAL BINARY ACCESS
The only uncompleted evidence operation is running the pipeline against the actual game payload. Until that happens, internal `GOLF.EPF` filenames and the exact executable build cannot truthfully be called locally verified.

Once a legal ZIP is supplied, no additional Step-2 engineering is required: run the ingestion command and commit only the resulting non-copyright analysis findings.
