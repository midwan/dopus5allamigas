#!/usr/bin/env python3
"""Regression checks for the About requester copyright block."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[4]
PROGRAM_ABOUT_C = ROOT / "source" / "Program" / "about.c"
MODULE_ABOUT_C = ROOT / "source" / "Modules" / "about" / "about.c"
MODULE_ABOUT_H = ROOT / "source" / "Modules" / "about" / "about.h"
MODULE_ABOUT_DATA_C = ROOT / "source" / "Modules" / "about" / "about_data.c"


def read_source(path):
    return path.read_text(encoding="latin-1")


class AboutRequesterTests(unittest.TestCase):
    def test_program_about_lists_all_copyright_lines_and_repo_url(self):
        source = read_source(PROGRAM_ABOUT_C)

        self.assertIn('(c) 1993-2012 Jonathan Potter & GP Software', source)
        self.assertIn('(c) 2012-2015 DOpus 5 Open Source Team', source)
        self.assertIn('(c) 2023-2026 Dimitris Panokostas', source)
        self.assertIn('https://github.com/BlitterStudio/dopus5', source)
        self.assertNotIn('www.dopus5.org', source)

    def test_about_module_renders_four_header_lines_before_translator(self):
        source = read_source(MODULE_ABOUT_C)
        header_loop = re.search(
            r"for\s*\([^;]+;\s*a\s*<\s*(?P<count>\d+)\s*&&\s*node->ln_Succ;",
            source,
        )

        self.assertIsNotNone(header_loop)
        self.assertEqual(header_loop.group("count"), "4")

    def test_about_layout_defines_four_copyright_gadgets(self):
        header = read_source(MODULE_ABOUT_H)
        data = read_source(MODULE_ABOUT_DATA_C)

        self.assertIn("GAD_COPYRIGHT_4", header)
        self.assertEqual(len(re.findall(r"GAD_COPYRIGHT_[1-4],", data)), 4)
        self.assertEqual(
            re.findall(r"\{0, POS_PROPORTION \+ (\d+), SIZE_MAXIMUM, 1\}", data),
            ["4", "29", "54", "79"],
        )


if __name__ == "__main__":
    unittest.main()
