# DOS / Windows Binary Cross-Check

## Purpose

Sensible Golf's PC release gives us a useful advantage: DOS and Windows executables can be analysed side-by-side.

A constant in one binary may be:
- gameplay;
- rendering;
- installer/platform glue;
- a coincidence.

A shared string/table/behaviour in both ports is a stronger anchor.

## Workflow

After Step 2 ingestion:

```bash
python tools/executable_probe.py \
  analysis/private/step2/input/.../GOLFDOS.EXE \
  --number 240 \
  -o analysis/private/golfdos-probe.json

python tools/executable_probe.py \
  analysis/private/step2/input/.../GOLFWIN.EXE \
  --number 240 \
  -o analysis/private/golfwin-probe.json

python tools/compare_probes.py \
  analysis/private/golfdos-probe.json \
  analysis/private/golfwin-probe.json \
  -o analysis/private/pc-crosscheck.json
```

## Interpretation rules

### Shared string
Useful as a navigation anchor, not proof of physics.

### Shared integer
Useful only when:
- its surrounding data forms a coherent table, or
- executable code reads it in a shot/ball path.

### Shared formula/branch behaviour
Strong evidence once code/data references and black-box behaviour agree.

### Different implementations with identical black-box output
Still acceptable. We need the gameplay semantics, not identical machine instructions.

## Evidence ledger

Each promoted gameplay claim gets its own JSON record conforming to:

`spec/recovery_evidence.schema.json`

Statuses are deliberately progressive:

1. `hypothesis` — worth investigating.
2. `observed` — directly seen in one trustworthy source.
3. `cross-checked` — supported by both ports or independent runtime evidence.
4. `parity-verified` — implemented and proven against an original-game trace.

Validate records with:

```bash
python tools/validate_evidence.py path/to/evidence.json
```

This prevents plausible-looking reverse-engineering guesses from quietly becoming permanent classic-mode behaviour.
