"""No production art may be added, altered or shipped without owner approval."""
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from tools.validate_pixtee_art import (
    validate, CATEGORY_IDS, ASSET_PATH, LEDGER_PATH
)


class PixteeArtApprovalTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / LEDGER_PATH).parent.mkdir(parents=True)
        self.data = {
            "schema": "pixtee.art-review.v1",
            "ownerApprovalRequired": True,
            "categories": [
                {"id": category, "status": "awaiting_art_proposal"}
                for category in sorted(CATEGORY_IDS)
            ],
            "approvedAssets": []
        }
        self.save()

    def save(self):
        (self.root / LEDGER_PATH).write_text(
            json.dumps(self.data), encoding="utf-8"
        )

    def add_asset(self, name="golfer_idle.png", approval="owner-review-001"):
        path = self.root / ASSET_PATH / name
        path.parent.mkdir(parents=True, exist_ok=True)
        raw = b"\x89PNG\r\n\x1a\n" + bytes(range(80))
        path.write_bytes(raw)
        self.data["approvedAssets"].append({
            "file": name,
            "sha256": hashlib.sha256(raw).hexdigest(),
            "category": "golfer_and_ball",
            "ownerApprovalReference": approval,
            "revision": "golfer-final-v1",
        })
        self.save()
        return path

    def test_debug_without_any_art_is_valid_but_release_is_blocked(self):
        self.assertEqual([],validate(self.root))
        self.assertTrue(validate(self.root,release=True))

    def test_every_production_file_must_have_approved_entry(self):
        p=self.root / ASSET_PATH / "golfer_idle.png"
        p.parent.mkdir(parents=True,exist_ok=True)
        p.write_bytes(b"not approved")
        self.assertTrue(any("Unapproved artwork" in e for e in validate(self.root)))

    def test_sha256_locks_exact_approved_file_bytes(self):
        p=self.add_asset()
        self.assertEqual([],validate(self.root))
        p.write_bytes(p.read_bytes()+b"changed")
        self.assertTrue(any("changed without reapproval" in e
                            for e in validate(self.root)))

    def test_owner_reference_is_mandatory(self):
        self.add_asset(approval="placeholder")
        self.assertTrue(any("owner-approval reference" in e
                            for e in validate(self.root)))

    def test_rejected_extra_image_is_detected_even_with_approved_asset(self):
        self.add_asset()
        rogue=self.root / ASSET_PATH / "copied_tree.png"
        rogue.write_bytes(b"bad art")
        self.assertTrue(any("copied_tree" in e for e in validate(self.root)))

    def test_untrusted_parent_path_rejected(self):
        self.data["approvedAssets"]=[{
            "file": "../something.png",
            "sha256": "0"*64,
            "category": "golfer_and_ball",
            "ownerApprovalReference": "owner-review-99",
            "revision": "test"
        }]
        self.save()
        self.assertTrue(any("Unsafe artwork" in e for e in validate(self.root)))

    def test_release_requires_six_categories_and_all_sprite_frames(self):
        self.add_asset()
        self.assertTrue(any("Release blocked" in e
                            for e in validate(self.root,release=True)))
        self.data["categories"][0]["status"]="approved"
        self.save()
        self.assertTrue(validate(self.root,release=True))


if __name__ == "__main__":
    unittest.main()
