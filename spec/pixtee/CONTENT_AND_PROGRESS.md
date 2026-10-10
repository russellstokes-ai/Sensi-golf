# Pixtee Golf — content and long-term progression

## Initial playable package

One original par-4 course at first, built with an authored fairway dogleg, one water hazard, optional sand and green slopes. **Do not create it by moving bunkers on the original course.** Its art must be new, even if familiar in style. As a gameplay product, show a ready-to-hit tee, aiming, 13 clubs, three-click swing, moving ball, landing, rough, bunker, water/out-of-bounds, green, putter, cup, scorecard and next-hole flow.

Expansion packs: introductory course set, championship course set, varied environments and difficulty. Independent hole archetypes cover straight par-3, short strategic par-4, long par-5, dogleg, island approach, elevated green, narrow fairway and risk/reward carry.

## Main menu and modes

- Classic Play: quick hole, 3/9/18 holes; no gating and **no wind by default**.
- Career: Amateur -> Regional -> National -> Pro -> World tours.
- Tournaments: stroke play and match play where rules are independently implemented.
- Challenges: longest drive, accuracy, putting, par-save, themed course targets.
- Statistics: career and individual course reports.
- Trophy Cabinet / Player Profile: offline trophy collection, achievements and cosmetic unlocks.

## Career design

At least five eras/tiers, each with events; starts with 3-hole cup and introduces full tournaments gradually. Scoreboard, event bracket, per-course difficulty, earned ranking points, unlock conditions and career milestones. AI opponents can be simulated reproducibly at difficulty bands, but never alter player-ball physics or cheat against player shot results. Every event accessible offline once unlocked.

## Stats contract

Capture per-shot (course/hole/club/aim/power/accuracy/shot type/lie before-after/distance/terminal), per-hole (par/strokes/GIR estimate/putts/hazards), per-round (total strokes/par/relative score/fairways hit/greens in regulation/putts), personal best and rolling score charts. Track optional wind mode separately to prevent misleading comparisons.

Statistics must be derived from authoritative shot events; **do not infer putting or FIR from score alone**. Record denominators, exclusions and exact formula definitions for every dashboard stat. Allow clear/reset or full offline export.

## Rewards

Achievements: first birdie, first eagle, first ace, first bogey-free 9 holes, flawless par-3 streak, first tour title, first championship, 10/50/100 rounds, milestone putts and course records. Smooth pixel-art reward reveal with end-frame hold, reduce-motion alternative and instant skip. Badges, trophies, golfer outfit colours, profile frames and celebration effects are cosmetic only. Difficulty/career ranks calibrated so max level cannot be reached after a handful of rounds.

No pay-to-win and no physics upgrades. Optional monetization only after a playable, fair base game and clear parental safeguards.

## Artwork and sound inventory

Original authored:
- golfer 8-way base, backswing/impact/follow-through and putter variants;
- ball, shadow, flight trail, cup, flag, tee markers;
- terrain tiles and transitions: fairway/rough/semi/green/bunker/water/trees/rocks/OOB;
- shadow/light variants, weather/wind icon only when feature active;
- 13 club selection symbols, aim pointer, power/accuracy meter, scorecard, leaderboard, pause HUD;
- five tour emblems, level ring, trophy icons, badges, optional cosmetics;
- swing, impact, wind (optional), bounces, putting, water, crowd/trophy audio cues;
- title, icon, splash, store screenshots and privacy UI.

Avoid original character artwork, logos, copied tile layouts, course bitmaps, fonts taken from original and sound samples. Each asset should have author, licence, prompt/reference if applicable, creation date, version, hash, approval.

## Course/asset review

Each authored course reviewed for playable par, realistic stroke accessibility, putt bounds, hazard fairness, camera scale readability and visual distinctions from each original protected hole. Include recorded designer intent and independent source files; never use original map data as editable template.
