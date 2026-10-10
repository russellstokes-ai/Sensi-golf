import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from compare_probes import compare


class CompareProbeTests(unittest.TestCase):
    def test_shared_strings_and_integer_hits(self):
        a = {
            "path": "GOLFDOS.EXE",
            "sha256": "a",
            "format": "LE",
            "strings": [
                {"offset": 10, "text": "1 WOOD"},
                {"offset": 20, "text": "DOS ONLY"},
            ],
            "integer_hits": [
                {"offset": 100, "width": 2, "value": 240},
            ],
        }
        b = {
            "path": "GOLFWIN.EXE",
            "sha256": "b",
            "format": "PE",
            "strings": [
                {"offset": 30, "text": "1 WOOD"},
                {"offset": 40, "text": "WIN ONLY"},
            ],
            "integer_hits": [
                {"offset": 200, "width": 2, "value": 240},
            ],
        }

        report = compare(a, b)
        self.assertEqual(report["common_strings"][0]["text"], "1 WOOD")
        self.assertEqual(report["common_integer_hits"][0]["value"], 240)
        self.assertIn("DOS ONLY", report["a_only_strings"])
        self.assertIn("WIN ONLY", report["b_only_strings"])


if __name__ == "__main__":
    unittest.main()
