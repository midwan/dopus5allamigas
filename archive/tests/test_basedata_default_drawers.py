#!/usr/bin/env python3
"""Regression checks for default drawers shipped in basedata.lha."""

from pathlib import Path
import shutil
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[2]
BASEDATA = ROOT / "archive" / "basedata.lha"

REQUIRED_PLACEHOLDERS = (
    "DOpus5/Commands/.dummy",
    "DOpus5/Desktop/.dummy",
    "DOpus5/Groups/.dummy",
    "DOpus5/Themes/.dummy",
    "DOpus5/Sounds/.dummy",
)


class BasedataDefaultDrawerTests(unittest.TestCase):
    @unittest.skipIf(shutil.which("lha") is None, "lha is required to inspect basedata.lha")
    def test_default_empty_drawers_have_placeholders(self):
        listing = subprocess.run(
            ["lha", "l", str(BASEDATA)],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        ).stdout

        for path in REQUIRED_PLACEHOLDERS:
            with self.subTest(path=path):
                self.assertIn(path, listing)


if __name__ == "__main__":
    unittest.main()
