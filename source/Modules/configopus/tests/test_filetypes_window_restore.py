#!/usr/bin/env python3
"""Regression checks for File Types saved window size handling."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[4]
FILETYPES_C = ROOT / "source" / "Modules" / "configopus" / "config_filetypes.c"


def read_source():
    return FILETYPES_C.read_text(encoding="latin-1")


class FiletypesWindowRestoreTests(unittest.TestCase):
    def test_saved_size_is_used_only_for_matching_font_size(self):
        source = read_source()
        load_pos = source.index('LoadPos("dopus/windows/filetypes"')
        saved_width = source.index("dims.fine_dim.Width = pos.Width;", load_pos)
        guarded_block = source[load_pos:saved_width]

        self.assertIn("filetypes_saved_size_matches_font(screen, fontsize)", guarded_block)
        self.assertIn("screen->RastPort.TxHeight", source)

    def test_filetypes_window_size_is_still_persisted(self):
        source = read_source()

        self.assertIn('LoadPos("dopus/windows/filetypes"', source)
        self.assertIn('SavePos("dopus/windows/filetypes"', source)


if __name__ == "__main__":
    unittest.main()
