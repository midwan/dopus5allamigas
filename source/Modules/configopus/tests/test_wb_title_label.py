#!/usr/bin/env python3
"""Regression checks for the WB title-bar compatibility option label."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[4]
CONFIGOPUS_CD = ROOT / "source" / "Modules" / "configopus" / "configopus.cd"
STRING_DATA_H = ROOT / "source" / "Modules" / "configopus" / "string_data.h"
GUIDE = ROOT / "documents" / "DOpus5.guide"


def read_source(path):
    return path.read_text(encoding="latin-1")


class WBTitleLabelTests(unittest.TestCase):
    def test_option_label_describes_workbench_title_bar_compatibility(self):
        expected = "Workbench-Compatible Title Bar"

        self.assertIn(f"_{expected}", read_source(CONFIGOPUS_CD))
        self.assertIn(f"_{expected}", read_source(STRING_DATA_H))
        self.assertIn(expected, read_source(GUIDE))

    def test_option_is_not_presented_as_a_clock_only_toggle(self):
        misleading = "Suppress DOpus Title Bar Clock"

        self.assertNotIn(misleading, read_source(CONFIGOPUS_CD))
        self.assertNotIn(misleading, read_source(STRING_DATA_H))
        self.assertNotIn(misleading, read_source(GUIDE))


if __name__ == "__main__":
    unittest.main()
