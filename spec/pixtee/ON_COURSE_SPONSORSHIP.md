# Pixtee Golf — On-course sponsorship sales & creative contract

## Commercial idea (approved scope)
Use small pixel-art **physical sponsor boards** beside tees and greens, like real tournament boards. No advertising banners, interstitials, video ads, pop-ups, forced waits or intrusive floating logos. Boards remain fixed to course coordinates, keep the original-era 2D presentation, and have no physics/collision effect. Unsold inventory is a subtle PIXTEE house sign.

## Inventory & packages
- Per course and hole: `<course-id>-h<01..18>-tee-a`, `tee-b`, `green-a`, `green-b` (4 placements per hole).
- Each complete 18-hole course has **72 independent saleable slots**. Number of total course slots is `number_of_18_hole_courses × 72`, once the final matching course count is verified. **Do not infer the course count from UI placeholders.**
- Can sell: one board, the tee pair, the green pair, four-board hole exclusive, course-wide sponsorship, tournament sponsorship, multi-course seasonal package, series/event bundles. Advertisers get agreed IDs, exclusivity scope, time window, creative rights and reporting obligations.
- Sales price, commercial terms and invoicing are NOT fixed or invented by the game framework.

## Technical model implemented in v0.1
`SponsorInventory.kt` generates ID-stable, small, world-coordinate placements; `SponsorCampaign` carries advertiser name, 2–11-character sign wording, optional PNG logo filename, RGB face/letter colours, slot assignment, approval/family-safe booleans and UTC start/end seconds. Only active approved campaigns appear; default fallback is PIXTEE house boards, with global setting OFF removing all signs. Calendar expiry is evaluated locally and requires no network.

Approved creative manifest: `pixtee-android/app/src/main/assets/sponsors/placements.v1.json`, e.g.:
```json
{
  "schemaVersion": 1,
  "campaigns": [
    {
      "id": "brand-a-2027",
      "advertiser": "Example Sponsor",
      "boardText": "EXAMPLE",
      "slotIds": ["lakewood-h01-tee-a", "lakewood-h01-green-b"],
      "startUtcSeconds": 1800000000,
      "endUtcSeconds": 1810000000,
      "approved": true,
      "familySafe": true,
      "backgroundColor": "#132843",
      "foregroundColor": "#FFFFFF",
      "logoFile": "example.png"
    }
  ]
}
```
This is an illustrative example, NOT a paid contract or live campaign. For a logo, bundle authorised PNG at `app/src/main/assets/sponsors/logos/example.png`. Recommend an original **96×24 px** legible horizontal mark; assets are locally packaged, never fetched/tracked from a third party.

Before publishing: `python tools/validate_pixtee_sponsors.py`. It rejects unapproved/unsafe entries, invalid creative, missing artwork and overlapping leases of the same slot. Android unit tests cover lifetime eligibility, the off switch, inventory counts and unchanged ball trajectories.

## Advertising sales process (future back office)
1. Seller selects an available course/hole/position package in an inventory calendar.
2. Create written sponsorship contract/authorisation, price, start/end UTC, usage rights, creative delivery and category restrictions; record invoice/payment out of band.
3. Approve brand and artwork for visibility, suitability to children and local laws. Reject gambling, vaping, tobacco, alcohol, adult, deceptive or behaviour-targeted adverts.
4. Reserve slots exclusively, publish validated campaign manifest, release branded APK. Future secure sponsor portal may manage time windows, artwork, booking, invoicing and remote campaign sync; NOT yet built.
5. Expiry returns to PIXTEE. **A packaged ad cannot be instantly revoked across offline installed apps**; withdrawal currently needs an app update.

## Privacy, fairness, accessibility
- No SDK for ad targeting, advertising IDs, impression tracking, external click-through links, sponsorship-based player buffs or physics modifications.
- Paid sponsor face includes a small **AD** marking. If advertising disclosure is not adequately legible at runtime, revise board design before commercial deployment.
- Boards are non-colliding world decoration, not a new obstacle. Player can turn all signs OFF under Options.
- Before external commercial deployment obtain applicable child-directed advertising, UK GDPR and advertiser-brand/IP reviews. Sponsor approval is never automated from a public upload.

## Acceptance tests
- 4 world-space signs per hole (2 tee, 2 green) with unique stable IDs, across full course roster once built.
- A player can enable/disable signs without changing ball trajectory, collisions, score or shot timing.
- Expired/unapproved/unsafe sponsor never appears. Overlapping leases fail validation.
- Visible tee/green signs respect model/camera scale and avoid HUD, golfer, green/cup, putting line and Whack-o-meter on tall closed-phone and Fold portrait viewports.
- QA must verify actual screenshots and performance; unit test passing is not proof of final presentation quality.
