import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from epf_extract import decompress_epfs_lzw, extract_epf


def pack_codes(codes_and_widths):
    bits = "".join(f"{code:0{width}b}" for code, width in codes_and_widths)
    bits += "0" * ((8 - len(bits) % 8) % 8)
    return int(bits, 2).to_bytes(len(bits) // 8, "big") if bits else b""


def pack_codes_9bit(codes):
    return pack_codes((code, 9) for code in codes)


def make_one_file_epf(name: bytes, payload: bytes, compressed: bool, output_size: int):
    fat_offset = 11 + len(payload)
    header = b"EPFS" + struct.pack("<I", fat_offset) + b"\0" + struct.pack("<H", 1)
    fat = (
        name.ljust(13, b"\0")
        + bytes([1 if compressed else 0])
        + struct.pack("<II", len(payload), output_size)
    )
    return header + payload + fat


class EPFExtractTests(unittest.TestCase):
    def test_literal_lzw_stream(self):
        # At 9 bits the EPFS EOF code is 511.
        stream = pack_codes_9bit([ord("A"), ord("B"), ord("C"), 511])
        self.assertEqual(decompress_epfs_lzw(stream, 3), b"ABC")

    def test_kwkwk_dictionary_case(self):
        # First code A, then code 256 before it formally exists: classic LZW KwKwK.
        stream = pack_codes_9bit([ord("A"), 256, 511])
        self.assertEqual(decompress_epfs_lzw(stream, 3), b"AAA")

    def test_code_width_expands_from_9_to_10_bits(self):
        # 255 literal codes cause 254 dictionary additions. After the final
        # 9-bit literal the next dictionary code reaches 510, so EPFS moves
        # to 10-bit codes. EOF is therefore 1023.
        codes = [(0, 9)] * 255 + [(1023, 10)]
        stream = pack_codes(codes)
        self.assertEqual(decompress_epfs_lzw(stream, 255), bytes(255))

    def test_dictionary_reset_keeps_current_bit_width(self):
        # Grow into 10-bit mode, reset using the second-highest 10-bit code
        # (1022), then emit a literal and EOF. EPFS deliberately does not
        # revert the width to 9 bits after reset.
        codes = [(0, 9)] * 255 + [(1022, 10), (1, 10), (1023, 10)]
        stream = pack_codes(codes)
        expected = bytes(255) + b"\x01"
        self.assertEqual(decompress_epfs_lzw(stream, 256), expected)

    def test_extracts_compressed_entry(self):
        stream = pack_codes_9bit([ord("A"), ord("B"), ord("C"), 511])
        archive = make_one_file_epf(b"TEST.DAT", stream, True, 3)
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            source = root / "GOLF.EPF"
            target = root / "out"
            source.write_bytes(archive)
            written = extract_epf(source, target)
            self.assertEqual(len(written), 1)
            self.assertEqual((target / "TEST.DAT").read_bytes(), b"ABC")

    def test_extracts_raw_entry(self):
        archive = make_one_file_epf(b"RAW.DAT", b"xyz", False, 3)
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            source = root / "GOLF.EPF"
            target = root / "out"
            source.write_bytes(archive)
            extract_epf(source, target)
            self.assertEqual((target / "RAW.DAT").read_bytes(), b"xyz")


if __name__ == "__main__":
    unittest.main()
