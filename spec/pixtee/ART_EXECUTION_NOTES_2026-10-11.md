# Pixtee art execution — integrity gate

The prior generic procedural PNG batch and generic contact sheet are REJECTED. They are not canonical and must never be imported into production.

Production visual reference: approved Pixtee artwork and genuine Sensible Golf running-game camera, not an invented isometric or full-hole view.

The source brief remains `ASTRA_FINAL_ART_ASSET_BIBLE_2026-10-10.md`. Preserve 256×128 golfer frame, feet pivot (80,116), and 576 unique direction/action frame names; do not claim approval from file existence.

Run `python tools/pixtee_art_clip_audit.py . --require-all-golfer-frames --json /tmp/pixtee-art-audit.json` against actual review exports to detect missing frames, clipped alpha margins and incorrect golfer dimensions. The script does not certify artistic quality or game-scale parity; those require owner review and actual Android gameplay captures.

**Status: pending original-approved artwork recovery, export, gameplay integration and owner approval.**
