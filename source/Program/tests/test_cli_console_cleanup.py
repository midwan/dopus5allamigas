#!/usr/bin/env python3
"""Regression checks for the internal CLI console setup failure paths."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
FUNCTION_CLI_C = ROOT / "source" / "Program" / "function_cli.c"


def read_source():
    return FUNCTION_CLI_C.read_text(encoding="latin-1")


def function_body(source, name):
    pattern = r"\n(?:BOOL|void)\s+" + re.escape(name) + r"\([^)]*\)\s*\{(?P<body>.*?)\n\}"
    match = re.search(pattern, source, re.S)
    if not match:
        raise AssertionError(f"Could not find {name}")
    return match.group("body")


class CLIConsoleCleanupTests(unittest.TestCase):
    def test_cli_data_starts_zeroed(self):
        source = read_source()

        self.assertIn("CLIData data = {0};", source)

    def test_cli_window_name_avoids_rawdofmt_string_varargs(self):
        source = read_source()

        self.assertIn("static void cli_build_window_name(char *buffer, int buffer_size)", source)
        self.assertIn("cli_build_window_name(handle->temp_buffer, sizeof(handle->temp_buffer));", source)
        self.assertIn('StrConcat(buffer, "/512/150/DOpus 5 CLI/CLOSE/SCREEN ", buffer_size);', source)
        self.assertIn("char *screen = get_our_pubscreen();", source)
        self.assertNotIn('"%s0/%ld/512/150/DOpus 5 CLI/CLOSE/SCREEN %s"', source)

    def test_cli_top_edge_conversion_avoids_overlapping_copy(self):
        source = read_source()

        self.assertNotIn("strcpy(buffer, buffer + pos);", source)
        self.assertIn("buffer[len - pos - 1]", source)

    def test_input_open_failure_restores_console_state(self):
        source = read_source()
        body = function_body(source, "cli_open")
        failure = body[body.index("if (!(data->input = Open(\"console:\", MODE_OLDFILE)))") :]

        for expected in (
            "SelectOutput(data->old_output);",
            "SelectInput(data->old_input);",
            "SetConsoleTask(data->old_console);",
            "Close(data->output);",
        ):
            self.assertIn(expected, failure)

    def test_cli_close_ignores_unopened_handles(self):
        source = read_source()
        body = function_body(source, "cli_close")

        self.assertIn("if (data->input)", body)
        self.assertIn("if (data->output)", body)


if __name__ == "__main__":
    unittest.main()
