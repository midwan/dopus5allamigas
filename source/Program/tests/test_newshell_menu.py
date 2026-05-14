#!/usr/bin/env python3
"""Regression checks for the Opus/NewShell menu entry."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
PROGRAM = ROOT / "source" / "Program"


def read_text(path):
    return path.read_text(encoding="latin-1")


class NewShellMenuTests(unittest.TestCase):
    def test_newshell_menu_is_below_execute_command(self):
        menu_data = read_text(PROGRAM / "menu_data.c")

        self.assertRegex(
            menu_data,
            r"\{NM_ITEM, MENU_EXECUTE, MSG_EXECUTE_MENU,[^}]+\},\n"
            r"\t\{NM_ITEM, MENU_NEWSHELL, MSG_NEWSHELL_MENU, 0\},",
        )

    def test_newshell_dispatches_through_misc_proc(self):
        event_loop = read_text(PROGRAM / "event_loop.c")

        self.assertIn("case MENU_NEWSHELL:", event_loop)
        self.assertIn('misc_startup("dopus_newshell", MENU_NEWSHELL, window, 0, 1);', event_loop)

    def test_newshell_uses_cli_launching_settings(self):
        misc_proc = read_text(PROGRAM / "misc_proc.c")
        match = re.search(r"case MENU_NEWSHELL: \{(?P<body>.*?)\n\t\t\}", misc_proc, re.S)
        if not match:
            raise AssertionError("Could not find MENU_NEWSHELL handler")

        body = match.group("body")
        for expected in (
            "environment->env->output_device",
            "environment->env->output_window",
            "get_our_pubscreen()",
            "NewShell",
            "Open(\"nil:\", MODE_OLDFILE)",
            "CLI_Launch(command",
            "environment->env->default_stack",
        ):
            self.assertIn(expected, body)

    def test_builtin_strings_and_guide_are_updated(self):
        strings = read_text(PROGRAM / "string_data.h")
        guide = read_text(ROOT / "documents" / "DOpus5.guide")

        self.assertIn("#define MSG_NEWSHELL_MENU 2708", strings)
        self.assertIn('#define MSG_NEWSHELL_MENU_STR "NewShell..."', strings)
        self.assertIn('link "Project - NewShell"', guide)
        self.assertIn('@node "Project - NewShell"', guide)


if __name__ == "__main__":
    unittest.main()
