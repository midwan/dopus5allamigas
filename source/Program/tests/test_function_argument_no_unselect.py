#!/usr/bin/env python3
"""Static checks for source argument no-unselect expansion."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
PROGRAM = ROOT / "source" / "Program"


def read_program_source(name):
    return (PROGRAM / name).read_text(encoding="latin-1")


class FunctionArgumentNoUnselectTests(unittest.TestCase):
    def test_source_no_unselect_tokens_are_distinct(self):
        function_data_h = read_program_source("function_data.h")
        function_launch_h = read_program_source("function_launch.h")
        function_parse_c = read_program_source("function_parse.c")

        self.assertIn("#define FUNC_ONE_FILE_NO_UNSELECT", function_data_h)
        self.assertIn("#define FUNC_ONE_PATH_NO_UNSELECT", function_data_h)
        self.assertIn("#define FUNCENTF_NO_UNSELECT", function_launch_h)
        self.assertRegex(
            function_parse_c,
            r"(?s)case 'o':.*"
            r"string\[func_pos \+ 1\] == 'u'.*"
            r"buffer\[parse_pos\] = FUNC_ONE_FILE_NO_UNSELECT",
        )
        self.assertRegex(
            function_parse_c,
            r"(?s)case 'f':.*"
            r"string\[func_pos \+ 1\] == 'u'.*"
            r"buffer\[parse_pos\] = FUNC_ONE_PATH_NO_UNSELECT",
        )

    def test_source_no_unselect_tokens_do_not_mark_entries_for_deselect(self):
        function_files_c = read_program_source("function_files.c")
        function_parse_c = read_program_source("function_parse.c")
        source_one_entry_section = re.search(
            r"(?s)// Single pathname.*?"
            r"// Last pathname",
            function_parse_c,
        ).group(0)

        self.assertIn("case FUNC_ONE_PATH_NO_UNSELECT:", source_one_entry_section)
        self.assertIn("case FUNC_ONE_FILE_NO_UNSELECT:", source_one_entry_section)
        self.assertIn("entry->flags |= FUNCENTF_NO_UNSELECT;", source_one_entry_section)
        self.assertIn("function_end_entry(handle, entry, 1);", source_one_entry_section)
        self.assertIn("deselect && !(entry->flags & FUNCENTF_NO_UNSELECT)", function_files_c)
        self.assertIn("entry->flags &= ~FUNCENTF_NO_UNSELECT;", function_files_c)


if __name__ == "__main__":
    unittest.main()
