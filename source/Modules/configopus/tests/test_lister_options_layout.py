#!/usr/bin/env python3
"""Regression checks for Environment panel vertical spacing."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[4]
ENVIRONMENT_DATA_C = ROOT / "source" / "Modules" / "configopus" / "config_environment_data.c"


def read_layout_source(layout_name):
    source = ENVIRONMENT_DATA_C.read_text(encoding="latin-1")
    pattern = re.escape(layout_name) + r"\[\]\s*=\s*\{(?P<body>.*?)\n\s*\{OD_END\}\}"
    match = re.search(pattern, source, re.S)
    if not match:
        raise AssertionError(f"Could not find {layout_name} layout")
    return match.group("body")


class ListerOptionsLayoutTests(unittest.TestCase):
    def test_lister_options_controls_are_lifted_from_panel_bottom_edge(self):
        layout = read_layout_source("_environment_listeroptions")

        self.assertIn("{5, -4, 0, 0}", layout)
        self.assertIn("{5, 0, 26, 4}", layout)
        self.assertIn("{5, 48, 26, 4}", layout)
        self.assertIn("{5, 54, 24, 6}", layout)
        self.assertNotIn("{5, 58, 24, 6}", layout)

    def test_misc_controls_are_lifted_from_panel_bottom_edge(self):
        layout = read_layout_source("_environment_misc_gadgets")

        self.assertIn("{5, 3, 0, 0}", layout)
        self.assertIn("{5, 6, 26, 4}", layout)
        self.assertIn("{5, 48, 8, 6}", layout)
        self.assertIn("{5, 62, 28, 6}", layout)
        self.assertIn("{33, 62, -8, 6}", layout)
        self.assertNotIn("{5, 70, 28, 6}", layout)


if __name__ == "__main__":
    unittest.main()
