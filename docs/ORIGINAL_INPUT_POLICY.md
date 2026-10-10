# Original Game Input Policy

## Public repository policy

Do not commit original commercial Sensible Golf material to this public repository unless the applicable licence explicitly grants redistribution rights.

This includes:
- `GOLFDOS.EXE`;
- `GOLFWIN.EXE`;
- EPF archives;
- Amiga disk images;
- original sprites/tiles;
- music and sound effects;
- course data;
- manuals/scans when redistribution is not permitted;
- decompiler databases containing substantial copied binary content.

## Local layout

Use an ignored local tree:

```
original/
  dos/
  amiga/
analysis/
  binaries/
  extracted/
  traces/
```

## Reproducibility

Every original input used for analysis must be recorded by:
- exact filename;
- byte size;
- SHA-256 digest;
- platform/version;
- provenance/licence note.

Use:

```
python tools/hash_inputs.py original
```

The generated manifest may be committed **only if it contains hashes/metadata and no copyrighted payloads**.

## Derived data

Small factual tables recovered from behaviour may be committed when legally permitted by the project's licence. Avoid committing large raw dumps or mechanically reconstructed assets until rights are confirmed.
