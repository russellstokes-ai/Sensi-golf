import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from executable_probe import extract_ascii_strings, find_integer, parse_executable


def synthetic_pe():
    data = bytearray(0x240)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", data, 0x84, 0x014C, 1, 123, 0, 0, 0xE0, 0x010F)
    optional = 0x98
    struct.pack_into("<H", data, optional, 0x10B)
    struct.pack_into("<I", data, optional + 16, 0x1234)
    struct.pack_into("<I", data, optional + 28, 0x00400000)
    section = optional + 0xE0
    data[section:section+8] = b".text\0\0\0"
    struct.pack_into("<IIII", data, section + 8, 0x1000, 0x1000, 0x200, 0x200)
    return bytes(data)


class ExecutableProbeTests(unittest.TestCase):
    def test_parses_pe_header(self):
        result = parse_executable(synthetic_pe())
        self.assertEqual(result["format"], "PE")
        self.assertEqual(result["machine"], "i386")
        self.assertEqual(result["number_sections"], 1)
        self.assertEqual(result["entry_point_rva"], 0x1234)
        self.assertEqual(result["sections"][0]["name"], ".text")

    def test_identifies_le_signature(self):
        data = bytearray(256)
        data[:2] = b"MZ"
        struct.pack_into("<I", data, 0x3C, 0x80)
        data[0x80:0x82] = b"LE"
        self.assertEqual(parse_executable(bytes(data))["format"], "LE")

    def test_extracts_strings_with_offsets(self):
        rows = extract_ascii_strings(b"\0HELLO\0x\0WORLD!\0", 5)
        self.assertEqual(rows[0], {"offset": 1, "text": "HELLO"})
        self.assertEqual(rows[1]["text"], "WORLD!")

    def test_finds_16_and_32_bit_integer(self):
        data = b"xx" + struct.pack("<H", 240) + b"yy" + struct.pack("<I", 240)
        hits = find_integer(data, 240)
        self.assertTrue(any(h["offset"] == 2 and h["width"] == 2 for h in hits))
        self.assertTrue(any(h["offset"] == 6 and h["width"] == 4 for h in hits))


if __name__ == "__main__":
    unittest.main()
