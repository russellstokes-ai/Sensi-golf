import json
import struct
import sys
import tempfile
import unittest
import zlib
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from ingest_pc_build import ingest


def make_epf(name=b"COURSE.DAT", payload=b"course"):
    fat_offset = 11 + len(payload)
    header = b"EPFS" + struct.pack("<I", fat_offset) + b"\0" + struct.pack("<H", 1)
    fat = (
        name.ljust(13, b"\0")
        + b"\0"
        + struct.pack("<II", len(payload), len(payload))
    )
    return header + payload + fat


class IngestTests(unittest.TestCase):
    def test_zip_pipeline(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            game = root / "game"
            game.mkdir()
            golfdos = b"MZ" + b"\0" * 126
            epf = make_epf()
            (game / "GOLFDOS.EXE").write_bytes(golfdos)
            (game / "GOLF.EPF").write_bytes(epf)

            archive = root / "SENSEGOLF.ZIP"
            with zipfile.ZipFile(archive, "w") as zf:
                zf.write(game / "GOLFDOS.EXE", "SensGolf/GOLFDOS.EXE")
                zf.write(game / "GOLF.EPF", "SensGolf/GOLF.EPF")

            reference = {
                "files": [
                    {
                        "path": "GOLFDOS.EXE",
                        "size": len(golfdos),
                        "crc32": f"{zlib.crc32(golfdos) & 0xffffffff:08x}",
                    },
                    {
                        "path": "GOLF.EPF",
                        "size": len(epf),
                        "crc32": f"{zlib.crc32(epf) & 0xffffffff:08x}",
                    },
                ]
            }
            ref_path = root / "reference.json"
            ref_path.write_text(json.dumps(reference), encoding="utf-8")

            workspace = root / "analysis"
            report = ingest(archive, workspace, ref_path)

            self.assertEqual(report["reference_summary"]["matches"], 2)
            self.assertEqual(report["epf"]["entry_count"], 1)
            self.assertEqual(report["epf"]["extracted_count"], 1)
            self.assertTrue((workspace / "extracted" / "epf" / "COURSE.DAT").exists())
            self.assertTrue((workspace / "reports" / "step2_report.md").exists())

    def test_rejects_zip_traversal(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            archive = root / "bad.zip"
            with zipfile.ZipFile(archive, "w") as zf:
                zf.writestr("../escape.bin", b"x")
            ref_path = root / "reference.json"
            ref_path.write_text('{"files":[]}', encoding="utf-8")

            with self.assertRaises(ValueError):
                ingest(archive, root / "analysis", ref_path)


if __name__ == "__main__":
    unittest.main()
