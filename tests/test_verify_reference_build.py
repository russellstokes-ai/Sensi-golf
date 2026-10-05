import sys
import tempfile
import unittest
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from verify_reference_build import verify


class VerifyReferenceBuildTests(unittest.TestCase):
    def test_match_and_mismatch(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            good = b"golf"
            bad = b"other"
            (root / "GOLFDOS.EXE").write_bytes(good)
            (root / "GOLF.EPF").write_bytes(bad)

            manifest = {
                "files": [
                    {
                        "path": "GOLFDOS.EXE",
                        "size": len(good),
                        "crc32": f"{zlib.crc32(good) & 0xffffffff:08x}",
                    },
                    {
                        "path": "GOLF.EPF",
                        "size": 999,
                        "crc32": "00000000",
                    },
                    {"path": "GOLFWIN.EXE", "size": 1, "crc32": "00000000"},
                ]
            }
            rows = verify(root, manifest)

        self.assertEqual([r["status"] for r in rows], ["match", "mismatch", "missing"])

    def test_unique_basename_can_match_nested_file(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            nested = root / "SENSGOLF"
            nested.mkdir()
            payload = b"x"
            (nested / "GOLFDOS.EXE").write_bytes(payload)
            manifest = {
                "files": [{
                    "path": "GOLFDOS.EXE",
                    "size": 1,
                    "crc32": f"{zlib.crc32(payload) & 0xffffffff:08x}",
                }]
            }
            rows = verify(root, manifest)
        self.assertEqual(rows[0]["status"], "match")


if __name__ == "__main__":
    unittest.main()
