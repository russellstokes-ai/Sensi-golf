# Analysis Log

Use this file for durable findings. Distinguish **observed**, **verified**, and **hypothesis**.

## 2026-10-05 — Repository bootstrap

### Verified
- Recovery is treated as a hard project gate before enhancement work.
- EPF inventory parser and unit tests are present.
- Public repository excludes original commercial binaries/assets by policy.

### To verify from original files
- Exact DOS/Windows file inventory and hashes.
- Executable versions/build identity.
- EPF archive names and contents.
- Club/trajectory tables.
- Shot power mapping.
- Accuracy/draw/fade mapping.
- Ball-state update loop.
- terrain/lie modifiers.
- putting/green slope logic.
- PRNG usage, if any.

### Rule
Do not upgrade a hypothesis to verified until supported by binary/data evidence or repeatable original-game measurements.
