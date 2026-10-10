import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from hash_inputs import manifest


class HashInputTests(unittest.TestCase):
    def test_manifest_is_sorted_and_hashed(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            (root / "b.bin").write_bytes(b"b")
            (root / "a.bin").write_bytes(b"a")
            rows = manifest(root)

        self.assertEqual([r["path"] for r in rows], ["a.bin", "b.bin"])
        self.assertEqual(rows[0]["size"], 1)
        self.assertEqual(rows[0]["sha256"], hashlib.sha256(b"a").hexdigest())


if __name__ == "__main__":
    unittest.main()
