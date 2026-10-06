import sys
import unittest
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from debug_pointer_probe import find_code_refs, pointer_after_label


class DebugPointerProbeTests(unittest.TestCase):
    def test_reads_pointer_after_zero_terminated_label(self):
        data=b"xxHDBall X coord\0"+bytes.fromhex("d6874200")+b"yy"
        rows=pointer_after_label(data,"HDBall X coord")
        self.assertEqual(len(rows),1)
        self.assertEqual(rows[0]["pointer"],0x004287D6)

    def test_finds_code_reference(self):
        lines=[
            "00401000: a1 d6 87 42 00 mov eax,ds:0x4287d6",
            "00401005: 90 nop",
        ]
        hits=find_code_refs(lines,0x4287D6)
        self.assertEqual(len(hits),1)
        self.assertEqual(hits[0]["instruction_address"],0x401000)


if __name__=="__main__":
    unittest.main()
