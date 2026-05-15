#!/usr/bin/env python3
"""Regression checks for warning-prone clock_task.c code."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]
CLOCK_TASK_C = ROOT / "source" / "Program" / "clock_task.c"


def read_source(path):
    return path.read_text(encoding="latin-1")


def read_custom_title_source():
    source = read_source(CLOCK_TASK_C)
    start = source.index("// Show custom title\n#ifdef __amigaos4__")
    end = source.index("static char mondays", start)
    return source[start:end]


class ClockTaskWarningTests(unittest.TestCase):
    def test_clock_time_building_avoids_divideu_remainder_storage(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("clock_build_title_text(&date, datebuf, titlebuf, show_seconds);", source)
        self.assertNotIn("DivideU(date.dat_Stamp.ds_Minute", source)
        self.assertNotIn("unsigned long minutes;", source)

    def test_custom_title_has_no_dead_format_variable(self):
        source = read_custom_title_source()

        self.assertIn("char *ptr, *title_buffer;", source)
        self.assertNotIn("char *ptr, *format, *title_buffer;", source)
        self.assertNotIn("format =", source)
        self.assertNotIn("lsprintf(buf,format,memval)", source)

    def test_custom_title_plain_memory_values_use_locale_separator(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("ItoaU(memval,", source)
        self.assertNotIn('lsprintf(buf, "%lu", memval);', source)


if __name__ == "__main__":
    unittest.main()
