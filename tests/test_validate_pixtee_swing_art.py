"""Pixel-accurate export guard tests, using valid PNG bytes built in-memory."""
import struct
import tempfile
import unittest
import zlib
from pathlib import Path

from tools.validate_pixtee_swing_art import decode_alpha, validate_folder

def png_chunk(kind, data):
    return (struct.pack(">I",len(data))+kind+data+
            struct.pack(">I",zlib.crc32(kind+data)&0xFFFFFFFF))

def example_png(*, feet=True, contact=True, width=128, rgba=True):
    rows=[]
    for y in range(64):
        row=bytearray()
        for x in range(width):
            alpha=255 if (feet and x==40 and y==58) or (
                contact and x==54 and y==58) else 0
            if rgba: row.extend((230,40,25,alpha))
            else: row.extend((230,40,25))
        rows.append(b"\x00"+bytes(row))
    mode=6 if rgba else 2
    return (b"\x89PNG\r\n\x1a\n"+
            png_chunk(b"IHDR",struct.pack(">IIBBBBB",width,64,8,mode,0,0,0))+
            png_chunk(b"IDAT",zlib.compress(b"".join(rows)))+
            png_chunk(b"IEND",b""))

class SwingArtExportTest(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path=Path(self.tmp.name)

    def test_correct_golfer_png_has_registered_feet_and_contact(self):
        p=self.path/"golfer_impact.png"
        p.write_bytes(example_png())
        self.assertEqual(64,len(decode_alpha(p)))
        self.assertEqual([],validate_folder(self.path))

    def test_export_rejects_contact_missed_by_even_a_few_pixels(self):
        (self.path/"golfer_impact.png").write_bytes(example_png(contact=False))
        self.assertTrue(any("clubface" in issue
                            for issue in validate_folder(self.path)))

    def test_export_rejects_sprite_with_no_registered_feet(self):
        (self.path/"golfer_follow.png").write_bytes(example_png(feet=False))
        self.assertTrue(any("feet pivot" in issue
                            for issue in validate_folder(self.path)))

    def test_export_rejects_wrong_size_or_non_alpha_images(self):
        p=self.path/"golfer_idle.png"
        p.write_bytes(example_png(width=127))
        self.assertTrue(validate_folder(self.path))
        p.write_bytes(example_png(rgba=False))
        self.assertTrue(validate_folder(self.path))

    def test_missing_unapproved_sprites_do_not_generate_fake_frames(self):
        self.assertEqual([],validate_folder(self.path))

if __name__ == "__main__":
    unittest.main()
