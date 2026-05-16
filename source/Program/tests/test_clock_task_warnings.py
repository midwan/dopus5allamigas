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

    def test_titlebar_other_memory_uses_fast_pool_on_aros(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("static IPTR clock_other_free_memory(IPTR graphics_mem)", source)
        self.assertIn("#ifdef __AROS__", source)
        self.assertIn("fast_mem = AvailMem(MEMF_FAST);", source)
        self.assertIn("clock_other_free_memory(graphics_mem)", source)
        self.assertNotIn("AvailMem(MEMF_ANY) - chipmem", source)

    def test_custom_title_memory_values_use_64_bit_formatters(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("ItoaU64(&memval,", source)
        self.assertIn("clock_memory_percent_string(buf, sizeof(buf), memval, memtotal);", source)
        self.assertIn("DivideToString64(", source)
        self.assertIn("BytesToString64(&memval,", source)
        self.assertNotIn("ItoaU(memval,", source)
        self.assertNotIn('lsprintf(buf, "%lu", memval);', source)

    def test_custom_title_used_memory_values_do_not_wrap_under_zero(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("static UQUAD clock_used_memory(UQUAD total_mem, UQUAD free_mem)", source)
        self.assertIn("memval = clock_used_memory(memtotal, AvailMem(MEMF_ANY));", source)
        self.assertIn("memval = clock_used_memory(memtotal, AvailMem(MEMF_CHIP));", source)
        self.assertIn("memval = clock_used_memory(memtotal, AvailMem(MEMF_FAST));", source)
        self.assertNotIn("memval = (memtotal - AvailMem", source)


if __name__ == "__main__":
    unittest.main()
