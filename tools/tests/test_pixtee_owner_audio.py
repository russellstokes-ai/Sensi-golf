import hashlib
import json
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


class OwnerAudioTests(unittest.TestCase):
    def test_owner_recordings_are_complete_and_unchanged(self):
        folder = ROOT / 'pixtee-godot/audio/owner-pack-v1'
        manifest = json.loads((folder / 'manifest.json').read_text())
        self.assertEqual(manifest['zip_sha256'], '2c1331b09ec0df2ca3dbbad514fb1da7ecb9294c8ae3827a528087511b5342f1')
        self.assertEqual(len(manifest['files']), 20)
        for entry in manifest['files']:
            data = (folder / entry['file']).read_bytes()
            self.assertEqual(len(data), entry['bytes'])
            self.assertEqual(hashlib.sha256(data).hexdigest(), entry['sha256'])
        paths = manifest['cues']
        self.assertIn('bird_impact', paths)
        self.assertIn('flag_hit', paths)
        self.assertIn('menu_music', paths)
        self.assertNotIn('gameplay_music', paths)
        self.assertEqual(set(paths.values()), {entry['file'] for entry in manifest['files']})


if __name__ == '__main__':
    unittest.main()
