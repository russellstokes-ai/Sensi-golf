import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from pe_string_xrefs import file_offset_to_va, find_disassembly_references


class PEStringXrefTests(unittest.TestCase):
    def test_maps_file_offset_to_runtime_va(self):
        probe = {
            "image_base": 0x400000,
            "sections": [{
                "raw_offset": 1000,
                "raw_size": 500,
                "virtual_address": 0x2000,
            }],
        }
        self.assertEqual(file_offset_to_va(probe, 1123), 0x40207B)
        self.assertIsNone(file_offset_to_va(probe, 999))

    def test_finds_reference_and_context(self):
        lines = [
            "00401000: 90 nop",
            "00401001: 68 54 db 41 00 push 0x41db54",
            "00401006: e8 00 00 00 00 call 0x40100b",
        ]
        hits = find_disassembly_references(lines, 0x41DB54, context=1)
        self.assertEqual(len(hits), 1)
        self.assertEqual(hits[0]["instruction_address"], 0x401001)
        self.assertEqual(len(hits[0]["context"]), 3)


if __name__ == "__main__":
    unittest.main()
