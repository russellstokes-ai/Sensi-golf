import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parents[1] / "tools"))
from validate_pixtee_sponsors import validate

class SponsorValidatorTests(unittest.TestCase):
    def example(self, **updates):
        c = dict(id="demo", advertiser="Example", boardText="EXAMPLE",
                 slotIds=["lakewood-h01-tee-a"], startUtcSeconds=1800000000,
                 endUtcSeconds=1810000000, approved=True, familySafe=True)
        c.update(updates)
        return c

    def test_empty_manifest_safe(self):
        validate({"schemaVersion": 1, "campaigns": []})
    def test_approved_slot_sale(self):
        validate({"schemaVersion": 1, "campaigns": [self.example()]})
    def test_unapproved_rejected(self):
        for bad in [self.example(approved=False), self.example(familySafe=False),
                    self.example(boardText="<script>"), self.example(logoFile="../p.png")]:
            with self.subTest(bad=bad):
                with self.assertRaises(ValueError):
                    validate({"schemaVersion": 1, "campaigns": [bad]})
    def test_exclusive_date_leases(self):
        collision=self.example(id="other", startUtcSeconds=1805000000,
                               endUtcSeconds=1815000000)
        with self.assertRaisesRegex(ValueError, "double-booked"):
            validate({"schemaVersion": 1, "campaigns": [self.example(), collision]})
        collision["startUtcSeconds"]=1810000000
        validate({"schemaVersion": 1, "campaigns": [self.example(), collision]})

if __name__ == "__main__":
    unittest.main()
