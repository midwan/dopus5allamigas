#!/usr/bin/env python3
"""Regression checks for Custom Screen Title substitution codes."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[4]
CLOCK_TASK_C = ROOT / "source" / "Program" / "clock_task.c"
CONFIGOPUS_CD = ROOT / "source" / "Modules" / "configopus" / "configopus.cd"
STRING_DATA_H = ROOT / "source" / "Modules" / "configopus" / "string_data.h"
CATALOG_DIR = ROOT / "catalogs"


def read_source(path):
    return path.read_text(encoding="latin-1")


class ScreenTitleCodeTests(unittest.TestCase):
    def test_internet_time_is_not_rendered_as_a_screen_title_code(self):
        source = read_source(CLOCK_TASK_C)

        self.assertNotIn("// Internet time", source)
        self.assertNotIn("*(ptr + 1) == 'i' && *(ptr + 2) == 't'", source)
        self.assertNotIn("struct DateStamp *date", source)

    def test_clock_text_is_advertised_but_internet_time_is_not(self):
        cd_source = read_source(CONFIGOPUS_CD)
        string_source = read_source(STRING_DATA_H)

        self.assertIn("MSG_SCREENTITLE_CODE_21", cd_source)
        self.assertIn("*t\\tClock text", cd_source)
        self.assertIn("#define MSG_SCREENTITLE_CODE_21 23021", string_source)
        self.assertIn("#define MSG_SCREENTITLE_CODE_LAST 23022", string_source)
        self.assertIn('#define MSG_SCREENTITLE_CODE_21_STR "*t\\tClock text"', string_source)
        self.assertIn("MSG_SCREENTITLE_CODE_21, (CONST_STRPTR)MSG_SCREENTITLE_CODE_21_STR", string_source)
        self.assertNotIn("*it\\tInternet Time", cd_source)
        self.assertNotIn("*it\\tInternet Time", string_source)

    def test_translated_configopus_catalogs_do_not_advertise_internet_time(self):
        for catalog in CATALOG_DIR.glob("*/configopus.ct"):
            with self.subTest(catalog=catalog.relative_to(ROOT)):
                source = read_source(catalog)
                self.assertNotIn("*it\\tInternet Time", source)


if __name__ == "__main__":
    unittest.main()
