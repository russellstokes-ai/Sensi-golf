import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from spt_probe import SPTError, analyse_directory, parse_spt


class SPTProbeTests(unittest.TestCase):
    def test_parses_five_big_endian_records(self):
        rows=[(13,0,100,700,1),(29,0,110,702,2),(45,0,120,704,3),(61,0,130,706,4),(65,0,300,100,5)]
        data=b"".join(struct.pack(">5H",*row) for row in rows)
        self.assertEqual(parse_spt(data),rows)

    def test_rejects_wrong_size(self):
        with self.assertRaises(SPTError):
            parse_spt(b"x"*49)

    def test_directory_invariants(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td)
            for n,shift in ((1,0),(2,10)):
                rows=[
                    (13,0,100+shift,700,1),
                    (29,0,110+shift,702,2),
                    (45,0,120+shift,704,3),
                    (61,0,130+shift,706,4),
                    (65,0,300+shift,100,5),
                ]
                (root/f"MAPM{n:02}.SPT").write_bytes(
                    b"".join(struct.pack(">5H",*row) for row in rows)
                )
            report=analyse_directory(root)
            self.assertEqual(report["file_count"],2)
            self.assertEqual(report["unique_sequences"]["word0"],[[13,29,45,61,65]])
            self.assertEqual(report["unique_sequences"]["word4"],[[1,2,3,4,5]])


if __name__=="__main__":
    unittest.main()
