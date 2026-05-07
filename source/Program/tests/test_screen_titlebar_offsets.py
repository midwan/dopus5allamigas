#!/usr/bin/env python3
"""Regression checks for OS3 screen title-bar vertical centering."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]
CLOCK_TASK_C = ROOT / "source" / "Program" / "clock_task.c"
PROTOS_H = ROOT / "source" / "Program" / "protos.h"


def read_source(path):
    return path.read_text(encoding="latin-1")


class ScreenTitleBarOffsetTests(unittest.TestCase):
    def test_os3_v47_centers_text_in_extra_height_titlebar(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("struct ClockTitleBarMetrics", source)
        self.assertIn("clock_titlebar_set_metrics(struct Screen *screen, struct RastPort *rp)", source)
        self.assertIn("((struct Library *)IntuitionBase)->lib_Version >= 47", source)
        self.assertIn("height = screen->BarHeight + 1", source)
        self.assertIn("((height - rp->TxHeight) >> 1) + rp->TxBaseline", source)
        self.assertNotIn("screen->BarVBorder + rp->TxBaseline", source)

    def test_titlebar_metrics_are_cached_from_screen_height(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("struct ClockTitleBarMetrics clock_titlebar_metrics", source)
        self.assertIn("clock_titlebar_set_metrics(screen, &clock_rp)", source)
        self.assertIn("clock_titlebar_metrics.height = height", source)
        self.assertIn("clock_titlebar_metrics.text_y = text_y", source)
        self.assertIn("clock_titlebar_metrics.fill_bottom = fill_bottom", source)
        self.assertIn("fill_bottom = height - 2", source)
        self.assertNotIn("clock_titlebar_height(struct Screen *screen, struct RastPort *rp)", source)
        self.assertNotIn("clock_titlebar_fill_height", source)

    def test_screen_titlebar_draw_paths_use_centered_helpers(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("Move(&clock_rp, clock_x, clock_titlebar_metrics.text_y)", source)
        self.assertIn("Move(rp, 5, clock_titlebar_metrics.text_y)", source)
        self.assertIn("clock_titlebar_image_y(size)", source)
        self.assertIn("RectFill(rp, rp->cp_x, 0, clock_x - 1, clock_titlebar_metrics.fill_bottom)", source)

    def test_screen_titlebar_draw_paths_do_not_clear_full_width_on_refresh(self):
        source = read_source(CLOCK_TASK_C)

        self.assertNotIn("clock_clear_titlebar_region", source)

    def test_os3_v47_erase_preserves_titlebar_trim_line(self):
        source = read_source(CLOCK_TASK_C)

        self.assertIn("short fill_bottom = height - 1", source)
        self.assertIn("fill_bottom = height - 2", source)

    def test_titlebar_render_functions_do_not_recompute_screen_metrics(self):
        source = read_source(CLOCK_TASK_C)
        protos = read_source(PROTOS_H)

        self.assertIn("void clock_show_memory(struct RastPort *, long, long, char *);", protos)
        self.assertIn("clock_show_memory(&clock_rp", source)
        self.assertIn("&clock_rp", source)
        self.assertIn("clock_show_memory(struct RastPort *rp, long msg, long clock_x, char *error)", source)
        self.assertNotIn("clock_show_memory(struct Screen *screen", source)


if __name__ == "__main__":
    unittest.main()
