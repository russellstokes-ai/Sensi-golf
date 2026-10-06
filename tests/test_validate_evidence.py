import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from validate_evidence import EvidenceError, validate


def base():
    return {
        "id": "club-1w-distance",
        "status": "observed",
        "claim": "example claim",
        "sources": [
            {
                "kind": "manual",
                "source": "manual",
                "offset": None,
                "observation": "documented behaviour",
            }
        ],
        "tests": [],
    }


class EvidenceTests(unittest.TestCase):
    def test_observed_record_valid(self):
        validate(base())

    def test_cross_checked_requires_two_ports_or_trace(self):
        record = base()
        record["status"] = "cross-checked"
        record["sources"] = [
            {"kind": "windows-executable", "source": "GOLFWIN.EXE", "offset": 12}
        ]
        with self.assertRaises(EvidenceError):
            validate(record)

    def test_cross_checked_two_ports_valid(self):
        record = base()
        record["status"] = "cross-checked"
        record["sources"] = [
            {"kind": "windows-executable", "source": "GOLFWIN.EXE", "offset": 12},
            {"kind": "dos-executable", "source": "GOLFDOS.EXE", "offset": 34},
        ]
        validate(record)

    def test_parity_verified_requires_trace_and_test(self):
        record = base()
        record["status"] = "parity-verified"
        with self.assertRaises(EvidenceError):
            validate(record)

        record["sources"].append({"kind": "black-box-trace", "source": "shot-001"})
        record["tests"] = ["parity/shot-001"]
        validate(record)


if __name__ == "__main__":
    unittest.main()
