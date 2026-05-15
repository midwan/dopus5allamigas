#!/usr/bin/env python3
"""Regression checks for locale-aware date parsing fallback."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
DATES_C = ROOT / "source" / "Library" / "dates.c"


def read_source(path):
    return path.read_text(encoding="latin-1")


class DateLocaleParsingTests(unittest.TestCase):
    def test_date_parser_retries_with_localised_month_normalised(self):
        source = read_source(DATES_C)
        function = source[source.index("BOOL LIBFUNC L_DateFromStringsNew") :]

        self.assertIn("ret = StrToDate(&datetime);", function)
        self.assertRegex(
            function,
            re.compile(
                r"if \(!ret && date_normalise_locale_month"
                r"\(date, tempdate, sizeof\(tempdate\)\)\)\s*\{\s*"
                r"(?:#if defined\(__amigaos4__\)\s*)?"
                r"datetime\.dat_StrDate = \([^)]*\)tempdate;\s*"
                r"(?:#else\s*datetime\.dat_StrDate = \([^)]*\)tempdate;\s*#endif\s*)?"
                r"ret = StrToDate\(&datetime\);",
                re.S,
            ),
        )

    def test_locale_month_normaliser_uses_locale_month_names(self):
        source = read_source(DATES_C)

        self.assertIn("date_english_months[12]", source)
        self.assertIn("GetLocaleStr(locale, locale_id)", source)
        self.assertIn("MON_1 + month", source)
        self.assertIn("ABMON_1 + month", source)
        self.assertIn("date_is_token_boundary", source)


if __name__ == "__main__":
    unittest.main()
