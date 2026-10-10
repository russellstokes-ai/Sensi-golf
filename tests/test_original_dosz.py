import hashlib
import io
from pathlib import Path
import tempfile
import unittest
import zipfile

from tools import build_original_dosz as d

class POCGameArchiveTests(unittest.TestCase):
    def setUp(self):
        self.hashes=d.KNOWN_HASHES.copy()
        self.orig={key:(key*7).encode() for key in self.hashes}
        d.KNOWN_HASHES={key:hashlib.sha256(value).hexdigest()
                        for key,value in self.orig.items()}
    def tearDown(self): d.KNOWN_HASHES=self.hashes

    def test_single_executable_preserves_original_archive(self):
        with tempfile.TemporaryDirectory() as tmp:
            source=Path(tmp)/"src"
            source.mkdir()
            for name,data in self.orig.items(): (source/name).write_bytes(data)
            (source/"GOLFWIN.EXE").write_bytes(b"WIN")
            (source/"README.TXT").write_text("full game")
            (source/"ANOTHER.EXE").write_bytes(b"something")
            dst=Path(tmp)/"game.dosz"
            result=d.pack(source,dst)
            with zipfile.ZipFile(dst) as z:
                self.assertEqual(z.read("GOLF.EPF"), self.orig["GOLF.EPF"])
                self.assertEqual(z.read("GOLFDOS.EXE"), self.orig["GOLFDOS.EXE"])
                self.assertEqual([n for n in z.namelist() if n.endswith(".EXE")],
                                 ["GOLFDOS.EXE"])
                self.assertNotIn("GOLFWIN.EXE",z.namelist())
                self.assertIn("README.TXT",z.namelist())
            self.assertEqual(len(result),3)

    def test_rejects_broken_original(self):
        with self.assertRaisesRegex(ValueError,"missing"):
            d.collect([("GOLF.EPF",b"BAD")])
        with self.assertRaisesRegex(ValueError,"not the validated"):
            d.collect([(key,b"WRONG") for key in self.hashes])

    def test_original_zip_input(self):
        with tempfile.TemporaryDirectory() as tmp:
            src=Path(tmp)/"original.zip"
            with zipfile.ZipFile(src,"w") as z:
                for key,value in self.orig.items():
                    z.writestr("subdir/"+key,value)
            self.assertEqual(d.load(src),self.orig)

if __name__=="__main__":
    unittest.main()
