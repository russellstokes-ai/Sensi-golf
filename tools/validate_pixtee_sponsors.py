#!/usr/bin/env python3
"""Fail-closed sponsor campaign validation before publishing a Pixtee APK."""
from __future__ import annotations

import json
import re
from pathlib import Path

PATH = Path("pixtee-android/app/src/main/assets/sponsors/placements.v1.json")
IDENT = re.compile(r"^[a-z0-9][a-z0-9-]{1,49}$")
SLOT = re.compile(r"^[a-z0-9-]{3,50}-h(?:0[1-9]|1[0-8])-(?:tee|green)-[ab]$")
LOGO = re.compile(r"^[a-z0-9][a-z0-9_-]{0,49}[.]png$")
COLOR = re.compile(r"^#[0-9A-Fa-f]{6}$")

def validate(data: dict, assets: Path = PATH.parent) -> None:
    if not isinstance(data, dict) or data.get("schemaVersion") != 1:
        raise ValueError("Sponsor inventory schemaVersion must be 1")
    campaigns = data.get("campaigns")
    if not isinstance(campaigns, list):
        raise ValueError("campaigns must be an array")
    ids: set[str] = set()
    leases: dict[str, list[tuple[int, int, str]]] = {}
    for campaign in campaigns:
        if not isinstance(campaign, dict):
            raise ValueError("Campaign is not an object")
        known = {"id", "advertiser", "boardText", "slotIds",
                 "startUtcSeconds", "endUtcSeconds", "approved",
                 "familySafe", "backgroundColor", "foregroundColor", "logoFile"}
        unknown = set(campaign) - known
        if unknown:
            raise ValueError("Unrecognized creative keys: " + repr(sorted(unknown)))
        cid = campaign.get("id")
        if not isinstance(cid, str) or not IDENT.fullmatch(cid) or cid in ids:
            raise ValueError("Invalid/duplicate campaign ID")
        ids.add(cid)
        advertiser = campaign.get("advertiser")
        text = campaign.get("boardText")
        if not isinstance(advertiser, str) or not 2 <= len(advertiser) <= 80:
            raise ValueError(cid + ": invalid advertiser")
        if not isinstance(text, str) or not re.fullmatch(r"[a-zA-Z0-9 &+.-]{2,11}", text):
            raise ValueError(cid + ": invalid board copy")
        if campaign.get("approved") is not True or campaign.get("familySafe") is not True:
            raise ValueError(cid + ": cannot package unapproved/non-family-safe campaign")
        start = campaign.get("startUtcSeconds")
        end = campaign.get("endUtcSeconds")
        if type(start) is not int or type(end) is not int or start < 1600000000 or end <= start:
            raise ValueError(cid + ": invalid sponsorship lease")
        slots = campaign.get("slotIds")
        if (not isinstance(slots, list) or not slots or len(slots) != len(set(slots)) or
                not all(isinstance(slot, str) and SLOT.fullmatch(slot) for slot in slots)):
            raise ValueError(cid + ": invalid or duplicated slot IDs")
        logo = campaign.get("logoFile")
        if logo is not None:
            if not isinstance(logo, str) or not LOGO.fullmatch(logo):
                raise ValueError(cid + ": invalid logo filename")
            if not (assets / "logos" / logo).is_file():
                raise ValueError(cid + ": logo asset missing")
        for field in ["backgroundColor", "foregroundColor"]:
            colour = campaign.get(field, "#FFFFFF")
            if not isinstance(colour, str) or not COLOR.fullmatch(colour):
                raise ValueError(cid + ": invalid board colour")
        for slot in slots:
            existing = leases.setdefault(slot, [])
            for old_start, old_end, old_id in existing:
                if start < old_end and old_start < end:
                    raise ValueError(slot + ": double-booked: " + old_id + " / " + cid)
            existing.append((start, end, cid))

if __name__ == "__main__":
    data = json.loads(PATH.read_text("utf-8"))
    validate(data)
    print("PASS: {} approved sponsor campaigns, no overlapping slots.".format(len(data["campaigns"])))
