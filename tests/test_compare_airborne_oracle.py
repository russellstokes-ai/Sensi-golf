import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from compare_airborne_oracle import compare


def sample(**overrides):
    row = {
        "tick": 0,
        "x": 1,
        "y": 2,
        "height": 3,
        "vertical_force": 4,
        "horizontal_force": 5,
        "direction": 6,
        "swing_adjuster": 0,
        "adjusted_power": 100,
    }
    row.update(overrides)
    return row


class CompareAirborneOracleTests(unittest.TestCase):
    def test_exact_match_passes(self):
        a = sample()
        self.assertTrue(compare({"samples": [a]}, {"samples": [dict(a)]})["pass"])

    def test_any_field_difference_fails(self):
        a = sample()
        b = sample(y=99)
        report = compare({"samples": [a]}, {"samples": [b]})
        self.assertFalse(report["pass"])
        self.assertEqual(report["mismatch_count"], 1)

    def test_live_launch_fields_are_compared(self):
        a = sample()
        b = sample(swing_adjuster=1, adjusted_power=99)
        report = compare({"samples": [a]}, {"samples": [b]})
        self.assertFalse(report["pass"])
        self.assertEqual(report["mismatch_count"], 2)


if __name__ == "__main__":
    unittest.main()
