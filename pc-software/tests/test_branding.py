import os
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from backend.config_store import config_directory


class BrandingCompatibilityTests(unittest.TestCase):
    def test_profile_directory_compatibility(self):
        root = Path(__file__).resolve().parents[1]
        cases = [
            ({}, root / "HoverBelt"),
            ({"BTOW_CONFIG_DIR": str(root / "new")}, root / "new"),
            ({"HOVERBELT_CONFIG_DIR": str(root / "old")}, root / "old"),
            ({"BTOW_CONFIG_DIR": str(root / "new"), "HOVERBELT_CONFIG_DIR": str(root / "old")}, root / "new"),
        ]
        for overrides, expected in cases:
            with self.subTest(overrides=overrides), patch.dict(os.environ, {
                "LOCALAPPDATA": str(root), "BTOW_CONFIG_DIR": "", "HOVERBELT_CONFIG_DIR": "", **overrides
            }):
                self.assertEqual(config_directory(), expected)

    def test_display_names(self):
        root = Path(__file__).resolve().parents[1]
        html = (root / "frontend/index.html").read_text(encoding="utf-8")
        self.assertIn("BTOW — Belt Tensioner Dashboard", html)
        self.assertIn("BTOW · Belt Tensioner", html)
        self.assertIn("btow-error.log", (root / "app.py").read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
