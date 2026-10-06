import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from gameplay_table_probe import analyse_trig, s16s, s32s


class GameplayTableProbeTests(unittest.TestCase):
    def test_integer_decoders(self):
        self.assertEqual(s16s(struct.pack("<hhh",-1,0,123)),[-1,0,123])
        self.assertEqual(s32s(struct.pack("<ii",-7,99)),[-7,99])

    def test_trig_metadata_shape(self):
        # Diagnostic test only; synthetic values are not game evidence.
        values=[0]*5120
        report=analyse_trig(values)
        self.assertEqual(report["cycle_words"],4096)
        self.assertEqual(report["quarter_cycle_words"],1024)
        self.assertEqual(report["min"],0)
        self.assertEqual(report["max"],0)
        self.assertTrue(report["second_lookup_is_same_table_plus_1024_words"])


if __name__=="__main__":
    unittest.main()
