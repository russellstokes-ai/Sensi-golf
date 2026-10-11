# Pixtee Golf — independent portrait Android game

**Active development:** `feat/pixtee-native-portrait-framework`, separate from the original Sensible Golf Android restoration. This source tree shares no original commercial binary, course files, visual assets or recovered physics code.

## Non-negotiable design contract
- **Portrait-only gameplay**, full edge-to-edge tall 19.5:9/20:9 phone view; fold closed/open portrait support.
- Familiar original-era **2D straight-down** camera, tiny golfer and course scale, tight left HUD, compact blue/wood styling, no pseudo-3D, voxel graphics or modern panoramic camera.
- Classic three-click **Whack-o-meter** (power then accuracy), 13 club slots, shot/camera/aim behaviour to be independently tuned against observable gameplay.
- Keep original-style pacing and no-wind classic rules. Wind optional OFF by default.
- New course layouts, branding and independently authored pixels. Career, statistics, trophies as additive screens. No gameplay buffs or pay-to-win.
- The user-approved phone-screen concept collage (10 Oct 2026) is a design direction, *not* pixel-perfect approval: relative UI size and meter still require refinement.
- No original-game files or import/emulator setup.
- Small non-interactive course sponsor signs **near tee and green**, 4 inventory slots per hole, advertiser creative only if approved, active and family-safe. House-brand default, world-projected with physics isolation, optional ON/OFF. No full-screen or video ads.

## Build
From a machine with Android SDK/Java 17 and Gradle 8.9 installed:
`gradle -p pixtee-android :app:assembleDebug`

## Status
Native-Android deterministic mechanics foundation, original Lakewood hole and basic touch UI in development. This is a new game with the intended classic mechanics, not the DOS binary under a different title. Visual and physics fidelity require measured tests before claiming parity.


## Course sponsor placements
Four small world-coordinate sponsor boards per hole (two tee, two green), with PIXTEE placeholders until approved paid campaigns are assigned through the validated local creative manifest. The player may toggle Course Boards ON/OFF in Options. These decorative boards do not alter ball physics, collision, targeting, shot scoring or difficulty. Sponsor packages, creative standards, time windows, approval and planned sales tooling: `spec/pixtee/ON_COURSE_SPONSORSHIP.md`. The paid booking/payment portal is **not yet implemented**.
