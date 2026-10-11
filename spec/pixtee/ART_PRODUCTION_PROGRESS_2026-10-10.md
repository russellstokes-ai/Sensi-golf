# Pixtee production progress — 10 October 2026

Master: ASTRA_FINAL_ART_ASSET_BIBLE_2026-10-10.md. Work is sequential on the requested feature branch. No artwork generated, integrated or approved in this batch.

## Stage 0 — in progress
- Implemented final 256×128 golfer registration: feet 80,116; contact 108,116; 8 pixels/world. World rectangle remains 32×16; contact remains +3.5 world X. Legacy 128×64 accepted only in debug review mode.
- Implemented terrain shader mapping to 16×8 world units independent of bitmap pixel dimensions.
- Updated export validator and regression fixtures; observed failing tests before implementation, then all five pass.
- Added explicit world-rectangle/native registration assertions to Kotlin tests. Native tests have NOT run locally: Android SDK, adb and Kotlin compiler unavailable here. GitHub Android workflow is the native verification route.
- Expanded all 1,655 v2 manifest frames plus compatibility IDs into ART_REVIEW_REGISTER.json, all pending. Register explicitly identifies remaining inventory sections; do not claim full inventory completion.
- Still pending: manifest/atlas loader, on-demand memory budgets, UI nine-slice and native screenshots.

## Ordered remaining work
1. Finish Stage 0 native pipeline verification and reference calibration.
2. Golfer and ball: original editable artwork, 576 frames, aliases, lies and contact previews.
3. Primary terrain, transitions, foliage, wildlife, crowds, photographer, impacts and boards.
4. Interface and horizontal combined meter; final zones require source proof.
5. All 35 original course packs and 630 individually authored/tested holes.
6. Career/rewards, wardrobe, original/licensed audio, branding and authentic marketing captures.
7. Normal-phone/Fold native evidence, physics invariance, then owner review per pack.

## Evidence gates
Original renderer camera scale, sprite-role proportions, tree hitboxes and meter geometry are still PENDING_REFERENCE in the master brief. No exact parity or final approval is claimed. No physics/camera implementation was changed in this batch. Runtime image assets remain absent; spec validation is not visual QA.
