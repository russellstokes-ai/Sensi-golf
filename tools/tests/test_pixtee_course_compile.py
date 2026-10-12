import importlib.util
from pathlib import Path
import unittest

MODULE = Path(__file__).parents[1] / 'pixtee_course_compile.py'

class CourseCompileTests(unittest.TestCase):
    def test_original_mapi_preserves_all_32_subcells(self):
        self.assertTrue(MODULE.exists(), 'Approved polygon compiler is missing')
        spec = importlib.util.spec_from_file_location('compiler', MODULE)
        compiler = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(compiler)
        samples = [1, 3, 5, 6] * 8
        descriptors, selectors = compiler.encode_tile(samples)
        self.assertEqual(len(descriptors), 8)
        self.assertEqual(len(selectors), 8)
        for i, expected in enumerate(samples):
            row, col = divmod(i, 8)
            mask = 0x80 >> col
            index = bool(selectors[row] & mask) + 2 * bool(selectors[row + 4] & mask)
            self.assertEqual(int.from_bytes(descriptors[index*2:index*2+2], 'big'), expected)
        with self.assertRaises(ValueError):
            compiler.encode_tile([1, 2, 3, 4, 5] + [1] * 27)

if __name__ == '__main__':
    unittest.main()
