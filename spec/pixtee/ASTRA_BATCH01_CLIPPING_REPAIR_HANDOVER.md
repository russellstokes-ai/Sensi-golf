# Astra Batch 01 — clipped sheet repair and production completion

## Status (2026-10-11)

The repository contains **12 generated PNG source sheets** at `art/previews/creation-batch-01/`, and a `manifest.json`. Every listed asset is `approved:false` and `runtimeExportComplete:false`. The source images must be treated as **style references**, not finished frame exports. The owner likes the existing style and specifically reports figures/elements truncated by margins. Preserve style, palette, relative proportions and original Sensible Golf gameplay view.

## Existing sheets, ALL to be audited

- golfer_swing_sheet.png
- golfer_directions_sheet.png
- golfer_putting_sheet.png
- ball_lies_sheet.png
- terrain_sheet.png
- trees_sheet.png
- foliage_sway_sheet.png
- spectator_breathing_sheet.png
- photographer_sheet.png
- impact_effects_sheet.png
- bird_species_sheet.png
- sponsor_boards_sheet.png

## Repair procedure — never fake missing art

1. Inspect every image at original pixel resolution and inventory each depicted character/object/animation pose with source pixel bounds. Record which touch or cross sheet boundaries. Preserve an unmodified original.
2. For clipped elements, **extend/repaint the missing parts matching that specific sheet's existing palette, line weight, pixel cluster language, light/shadow, anatomy and pose**. Do not merely increase canvas dimensions, stretch/mirror unrelated anatomy, clone half a golfer or use a generically regenerated replacement. If missing information cannot be inferred reliably, redraw the whole **individual affected pose** from its intact adjacent frames/style reference, not the entire approved sheet.
3. Leave at least 16 source pixels transparent breathing room around every complete subject in review sheet layouts. This is a **review-layout margin only**: final per-frame export dimensions and pivot positions must follow the canonical manifests; never rescale game objects to add this margin.
4. Create individual transparent, fully registered production frames and editable masters. Confirm actual frame counts, dimensions, pivot/foot anchor, contact point, directional silhouette and animation continuity. Preserve source-game gameplay scale and camera.
5. Produce one before/after comparison for every repaired sheet, plus individual-frame playback and real in-game portrait screenshot at normal phone zoom. Owner approval before replacing the canonical source sheet or integrating it.
6. Finish all outstanding categories in `ASTRA_FINAL_ART_ASSET_BIBLE_2026-10-10.md` and `ASTRA_AMBIENCE_AND_IMPACT_MANIFEST_v2.json` — including 21 biome birds, comical bird strike/recovery, 576 golfer direction/pose frames, living foliage, grounded crowd breathing, photographer flash, sand/water/tree impact sequences, and tee/green-facing signs. Track each as missing/created/repaired/exported/integrated/approved.
7. No fabricated completion claims: `manifest.json` must continue to report `approved:false` and `runtimeExportComplete:false` until review and real production exports have happened.

## Delivery locations

- Keep source references: `art/previews/creation-batch-01/`.
- Repaired previews: `art/previews/repair-batch-01/`.
- Editable art: `art/source/` per asset bible.
- Review PNGs: `pixtee-android/app/src/main/assets/art/review/`.
- Approved production PNGs: `pixtee-android/app/src/main/assets/art/production/`.

## Access constraint for this handover

The available GitHub connector can read the text manifest and list the PNG blobs, but returns a UTF-8 decoding error on binary PNG files. **No PNG pixel inspection or repairs have yet been performed by this assistant.** Upload the original sheets as files or a ZIP into the conversation, or use a GitHub-connected image-capable workstation/agent, before asserting clipping has been repaired.
