import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from epf_inspect import EPFError, parse_epf


def make_epf():
    payloads = [b"abc", b"12345"]
    fat_offset = 11 + sum(map(len, payloads))
    header = b"EPFS" + struct.pack("<I", fat_offset) + b"\x00" + struct.pack("<H", 2)
    fat = bytearray()
    for name, compressed, data, out_size in [
        (b"ONE.DAT", 0, payloads[0], 3),
        (b"TWO.MAP", 1, payloads[1], 12),
    ]:
        fat += name.ljust(13, b"\0")
        fat += bytes([compressed])
        fat += struct.pack("<II", len(data), out_size)
    return header + b"".join(payloads) + fat


class EPFTests(unittest.TestCase):
    def test_inventory_offsets_and_metadata(self):
        entries = parse_epf(make_epf())
        self.assertEqual([e.filename for e in entries], ["ONE.DAT", "TWO.MAP"])
        self.assertEqual([e.offset for e in entries], [11, 14])
        self.assertFalse(entries[0].compressed)
        self.assertTrue(entries[1].compressed)
        self.assertEqual(entries[1].decompressed_size, 12)

    def test_rejects_bad_signature(self):
        with self.assertRaises(EPFError):
            parse_epf(b"NOPE" + b"\0" * 20)


if __name__ == "__main__":
    unittest.main()
