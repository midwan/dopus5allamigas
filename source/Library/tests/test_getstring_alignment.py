#!/usr/bin/env python3
"""Regression checks for 68000-safe built-in locale string parsing."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
LAYOUT_SUPPORT_C = ROOT / "source" / "Library" / "layout_support.c"


def read_source(path):
    return path.read_text(encoding="latin-1")


class GetStringAlignmentTests(unittest.TestCase):
    def test_builtin_catalog_parser_uses_byte_reads(self):
        source = read_source(LAYOUT_SUPPORT_C)
        get_string = source[source.index("STRPTR LIBFUNC L_GetString") : source.index("FindKeyEquivalent")]

        self.assertIn("locale_read_be_long", source)
        self.assertIn("locale_read_be_word", source)
        self.assertNotIn("LONG *l;", get_string)
        self.assertNotIn("UWORD *w;", get_string)
        self.assertNotRegex(get_string, re.compile(r"\*\s*l\s*!="))
        self.assertNotRegex(get_string, re.compile(r"\*\s*w\b"))


if __name__ == "__main__":
    unittest.main()
