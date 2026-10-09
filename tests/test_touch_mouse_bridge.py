"""Fast CI guard for the pinned LibretroDroid touchscreen mouse patch."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from patch_libretrodroid_touch_mouse import BRIDGE, patch

PINNED_FRAGMENT = """int16_t Input::getInputState(unsigned port, unsigned device, unsigned index, unsigned id) {
    switch (device) {
        case RETRO_DEVICE_POINTER: {
            switch (id) {
                case RETRO_DEVICE_ID_POINTER_PRESSED: return 1;
            }
        }
        default: return 0;
    }
}
"""


class TouchBridgeTests(unittest.TestCase):
    def test_injects_primary_mouse_click_before_pointer(self):
        result, changed = patch(PINNED_FRAGMENT)
        self.assertTrue(changed)
        self.assertIn(BRIDGE, result)
        self.assertLess(result.index("case RETRO_DEVICE_MOUSE:"),
                        result.index("case RETRO_DEVICE_POINTER:"))
        self.assertIn("id != RETRO_DEVICE_ID_MOUSE_LEFT", result)
        self.assertIn("pointerScreenXAxis >= 0", result)
        self.assertIn("pointerScreenYAxis >= 0", result)

    def test_idempotent(self):
        once, changed = patch(PINNED_FRAGMENT)
        self.assertTrue(changed)
        twice, changed = patch(once)
        self.assertFalse(changed)
        self.assertEqual(once, twice)

    def test_unexpected_pinned_source_fails_closed(self):
        with self.assertRaises(ValueError):
            patch("unrecognized upstream source")
        with self.assertRaises(ValueError):
            patch("        case RETRO_DEVICE_MOUSE: {  /* foreign implementation */ }\n" + PINNED_FRAGMENT)


if __name__ == "__main__":
    unittest.main()
