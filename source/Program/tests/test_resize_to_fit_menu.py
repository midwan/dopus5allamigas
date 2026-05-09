#!/usr/bin/env python3
"""Regression checks for the Resize to Fit menu entry."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
MENUS_C = ROOT / "source" / "Program" / "menus.c"


def read_source(path):
    return path.read_text(encoding="latin-1")


def allowed_windows_for(menu_id):
    source = read_source(MENUS_C)
    pattern = (
        r"static ULONG menu_disable_keys\[\]\s*=\s*\{(?P<table>.*?)\n"
        r"\s*0,\s*\n\s*0\};"
    )
    match = re.search(pattern, source, re.S)
    if not match:
        raise AssertionError("Could not find menu_disable_keys table")

    rows = [
        row.strip()
        for row in match.group("table").splitlines()
        if row.strip() and not row.strip().startswith("//")
    ]

    for index, row in enumerate(rows):
        if row == f"{menu_id},":
            return rows[index - 1].rstrip(",")

    raise AssertionError(f"Could not find {menu_id} in menu_disable_keys")


class ResizeToFitMenuTests(unittest.TestCase):
    def test_resize_to_fit_is_not_available_on_main_backdrop_menu(self):
        allowed_windows = allowed_windows_for("MENU_LISTER_RESIZE_FIT")

        self.assertIn("WINDOW_LISTER_ICONS", allowed_windows)
        self.assertIn("WINDOW_GROUP", allowed_windows)
        self.assertNotIn("WINDOW_BACKDROP", allowed_windows)


if __name__ == "__main__":
    unittest.main()
