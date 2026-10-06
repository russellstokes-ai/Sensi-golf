import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from compare_airborne_oracle import compare


class CompareAirborneOracleTests(unittest.TestCase):
    def test_exact_match_passes(self):
        sample = {
            "tick": 0, "x": 1, "y": 2, "height": 3,
            "vertical_force": 4, "horizontal_force": 5, "direction": 6,
        }
        self.assertTrue(compare({"samples": [sample]}, {"samples": [dict(sample)]})["pass"])

    def test_any_field_difference_fails(self):
        a = {"tick": 0, "x": 1, "y": 2, "height": 3,
             "vertical_force": 4, "horizontal_force": 5, "direction": 6}
        b = dict(a)
        b["y"] = 99
        report = compare({"samples": [a]}, {"samples": [b]})
        self.assertFalse(report["pass"])
        self.assertEqual(report["mismatch_count"], 1)


if __name__ == "__main__":
    unittest.main()
