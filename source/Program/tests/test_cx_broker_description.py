#!/usr/bin/env python3
"""Regression checks for the Commodities Exchange broker text."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]
CX_C = ROOT / "source" / "Program" / "cx.c"


class CommoditiesBrokerDescriptionTests(unittest.TestCase):
    def test_broker_title_uses_ascii_copyright_text(self):
        source = CX_C.read_bytes()

        self.assertIn(
            b'cx->nb.nb_Title = "(c) 1998 Jonathan Potter & GPSoftware";',
            source,
        )
        self.assertNotIn(b"\xef\xbf\xbd", source)

    def test_broker_description_uses_localized_workbench_replacement_text(self):
        source = CX_C.read_text(encoding="utf-8")

        self.assertIn("cx->nb.nb_Descr = GetString(&locale, MSG_CX_DESC);", source)


if __name__ == "__main__":
    unittest.main()
