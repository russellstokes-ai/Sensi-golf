import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from compare_traces import TraceError, compare, validate_trace


def trace(offset=0.0):
    return {
        "version": 1,
        "source": {"build_sha256": "abc", "executable": "GOLFDOS.EXE", "course": "test", "hole": 1},
        "input": {"aim": 0, "club": "1W", "lie": "tee", "power_tick": 10, "accuracy_tick": 20},
        "samples": [
            {"tick": 0, "x": 0 + offset, "y": 0, "z": 0},
            {"tick": 1, "x": 10 + offset, "y": 5, "z": 3},
            {"tick": 2, "x": 20 + offset, "y": 8, "z": 0},
        ],
        "events": {
            "landing": {"x": 20 + offset, "y": 8},
            "rest": {"x": 24 + offset, "y": 9},
            "hazard": None,
            "holed": False,
        },
    }


class CompareTraceTests(unittest.TestCase):
    def test_identical_trace_passes(self):
        report = compare(trace(), trace(), 0.0, 0.0)
        self.assertTrue(report["pass"])
        self.assertEqual(report["max_xy_error"], 0.0)

    def test_small_offset_respects_tolerance(self):
        report = compare(trace(), trace(0.25), 0.3, 0.3)
        self.assertTrue(report["pass"])

    def test_large_offset_fails(self):
        report = compare(trace(), trace(1.0), 0.5, 0.5)
        self.assertFalse(report["pass"])

    def test_missing_tick_fails(self):
        candidate = trace()
        candidate["samples"].pop()
        report = compare(trace(), candidate, 0.0, 0.0)
        self.assertFalse(report["pass"])
        self.assertEqual(report["missing_reference_ticks"], [2])

    def test_non_monotonic_ticks_rejected(self):
        bad = trace()
        bad["samples"][2]["tick"] = 1
        with self.assertRaises(TraceError):
            validate_trace(bad)


if __name__ == "__main__":
    unittest.main()
