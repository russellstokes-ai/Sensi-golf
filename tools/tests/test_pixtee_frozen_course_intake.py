"""Unit tests: no Work files or approved production media needed for CI."""
from __future__ import annotations
import hashlib
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path
import zipfile

SOURCE = Path(__file__).resolve().parents[1] / "pixtee_frozen_course_intake.py"
spec = importlib.util.spec_from_file_location("pixtee_frozen_course_intake", SOURCE)
assert spec and spec.loader
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class TestFrozenPixteeIntake(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.work = self.root / "frozen-work-hole"
        self.work.mkdir()
        (self.work / "index.html").write_text("<html><body>owner freeze</body></html>", encoding="utf-8")
        (self.work / "COURSE_PRODUCTION_STANDARD.md").write_text("# Production standards", encoding="utf-8")
        (self.work / "COURSE_CATALOGUE_50.csv").write_text(
            "id,course\n" + "".join(f"{i},Course {i}\n" for i in range(1, 51)),
            encoding="utf-8",
        )
        nested = self.work / "images"
        nested.mkdir()
        (nested / "approved.png").write_bytes(b"not production pixels; unit fixture only")
        self.assets = self.root / "fixture-source.zip"
        with zipfile.ZipFile(self.assets, "w") as archive:
            archive.writestr("art/a.png", b"fixture test bytes")
            archive.writestr("art/b.png", b"second fixture bytes")
        expected_hash = module.sha256(self.assets)
        self.canonical = self.root / "fake-approved-source-manifest.json"
        self.canonical.write_text(json.dumps({
            "original_sha256": expected_hash,
            "size_bytes": self.assets.stat().st_size,
            "zip_member_count": 2,
            "zip_members": ["art/a.png", "art/b.png"],
        }), encoding="utf-8")
        self.old_canonical = module.CANONICAL_ART
        module.CANONICAL_ART = self.canonical
        self.addCleanup(lambda: setattr(module, "CANONICAL_ART", self.old_canonical))

    def freeze_hashes(self) -> Path:
        manifest = self.root / "trusted-original-hashes.json"
        manifest.write_text(json.dumps({
            "files": [
                {"path": x["path"], "sha256": x["sha256"]}
                for x in module.scan_directory(self.work)
            ]
        }), encoding="utf-8")
        return manifest

    def test_read_only_intake_hashes_every_file_and_has_no_production_claim(self):
        independent_hashes = self.freeze_hashes()
        output = self.root / "audit" / "session-001.json"
        before = {x["path"]: x["sha256"] for x in module.scan_directory(self.work)}
        result = module.run(self.work, self.assets, None, independent_hashes, output)
        after = {x["path"]: x["sha256"] for x in module.scan_directory(self.work)}
        self.assertEqual(before, after)
        self.assertEqual(result["source_file_count"], 4)
        self.assertTrue(result["work_independent_hash_manifest_verified"])
        self.assertEqual(result["catalogue"]["data_rows"], 50)
        self.assertTrue(result["catalogue"]["catalogue_50_rows_confirmed"])
        self.assertFalse(result["native_collision_geometry_extracted"])
        self.assertFalse(result["three_playable_holes_completed"])
        self.assertFalse(result["commercial_rights_cleared"])
        self.assertFalse(result["input_files_modified"])
        self.assertTrue(output.is_file())
        with self.assertRaises(module.IntakeError):
            module.run(self.work, self.assets, None, independent_hashes, output)

    def test_changed_work_file_fails_trusted_hashes(self):
        frozen = self.freeze_hashes()
        (self.work / "images" / "approved.png").write_bytes(b"changed!")
        with self.assertRaisesRegex(module.IntakeError, "modified"):
            module.run(self.work, self.assets, None, frozen, None)

    def test_missing_work_standard_rejected(self):
        (self.work / "COURSE_PRODUCTION_STANDARD.md").unlink()
        with self.assertRaisesRegex(module.IntakeError, "missing immutable"):
            module.run(self.work, self.assets, None, None, None)

    def test_wrong_asset_sha_rejected(self):
        with zipfile.ZipFile(self.assets, "a") as archive:
            archive.writestr("new.png", b"different")
        with self.assertRaisesRegex(module.IntakeError, "SHA-256"):
            module.run(self.work, self.assets, None, None, None)

    def test_menu_intake_records_hash_without_approving_menus(self):
        menu = self.root / "menus"
        menu.mkdir()
        (menu / "main-menu.html").write_text("work-in-progress", encoding="utf-8")
        report = module.run(self.work, self.assets, menu, None, None)
        self.assertEqual(report["menu_status"], "HASH_RECORDED_NOT_VISUALLY_APPROVED")
        self.assertEqual(len(report["menu_files"]), 1)

    def test_prevent_writing_inside_frozen_source(self):
        with self.assertRaisesRegex(module.IntakeError, "inside a frozen source"):
            module.run(self.work, self.assets, None, None, self.work / "intake.json")

    def test_output_cannot_overwrite_approved_art(self):
        with self.assertRaisesRegex(module.IntakeError, "overwrite"):
            module.run(self.work, self.assets, None, None, self.assets)

    def test_symlinks_never_silently_pass(self):
        link = self.work / "linked_art.bin"
        try:
            link.symlink_to(self.assets)
        except (OSError, NotImplementedError):
            self.skipTest("Fixture OS disallows symlinks")
        with self.assertRaisesRegex(module.IntakeError, "Symlinks"):
            module.run(self.work, self.assets, None, None, None)


if __name__ == "__main__":
    unittest.main()
