#!/usr/bin/env python3
"""Regression checks for the Icons/Arrange Icons menu entries."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
PROGRAM = ROOT / "source" / "Program"


def read_program_source(name):
    return (PROGRAM / name).read_text(encoding="latin-1")


def icons_menu_block():
    source = read_program_source("menu_data.c")
    match = re.search(r"\t// Icons\n(?P<block>.*?)\n\n\t// Buttons", source, re.S)
    if not match:
        raise AssertionError("Could not find Icons menu block")
    return match.group("block")


def allowed_windows_for(menu_id):
    source = read_program_source("menus.c")
    pattern = (
        r"static ULONG menu_disable_keys\[\]\s*=\s*\{(?P<table>.*?)\n"
        r"\s*0,\s*\n\s*0\};"
    )
    match = re.search(pattern, source, re.S)
    if not match:
        raise AssertionError("Could not find menu_disable_keys table")

    rows = [
        row.strip()
        for row in match.group("table").splitlines()
        if row.strip() and not row.strip().startswith("//")
    ]

    for index, row in enumerate(rows):
        if row == f"{menu_id},":
            return rows[index - 1].rstrip(",")

    raise AssertionError(f"Could not find {menu_id} in menu_disable_keys")


class ArrangeIconsMenuTests(unittest.TestCase):
    def test_icons_menu_has_arrange_icons_submenu_before_cleanup(self):
        block = icons_menu_block()

        self.assertRegex(
            block,
            r"\{NM_ITEM, MENU_ICON_SELECT_ALL, MSG_ICON_SELECT_ALL_MENU,[^}]+\},\n"
            r"\t\{NM_ITEM, MENU_ICON_ARRANGE, MSG_LISTER_ARRANGE_ICONS, 0\},\n"
            r"\t\{NM_SUB, MENU_LISTER_ARRANGE_NAME, MSG_LISTER_ARRANGE_NAME, 0\},\n"
            r"\t\{NM_SUB, MENU_LISTER_ARRANGE_TYPE, MSG_LISTER_ARRANGE_TYPE, 0\},\n"
            r"\t\{NM_SUB, MENU_LISTER_ARRANGE_SIZE, MSG_LISTER_ARRANGE_SIZE, 0\},\n"
            r"\t\{NM_SUB, MENU_LISTER_ARRANGE_DATE, MSG_LISTER_ARRANGE_DATE, 0\},\n"
            r"\t\{NM_ITEM, MENU_ICON_CLEANUP, MSG_ICON_CLEANUP,",
        )

    def test_arrange_icons_menu_items_are_icon_window_only(self):
        expected = "WINDOW_BACKDROP | WINDOW_GROUP | WINDOW_LISTER_ICONS"

        for menu_id in (
            "MENU_ICON_ARRANGE",
            "MENU_LISTER_ARRANGE_NAME",
            "MENU_LISTER_ARRANGE_TYPE",
            "MENU_LISTER_ARRANGE_SIZE",
            "MENU_LISTER_ARRANGE_DATE",
        ):
            self.assertEqual(expected, allowed_windows_for(menu_id))

    def test_arrange_icons_dispatches_for_desktop_group_and_lister_windows(self):
        event_loop = read_program_source("event_loop.c")
        group = read_program_source("backdrop_groups.c")
        lister = read_program_source("lister_function.c")

        for menu_id in (
            "MENU_LISTER_ARRANGE_NAME",
            "MENU_LISTER_ARRANGE_TYPE",
            "MENU_LISTER_ARRANGE_SIZE",
            "MENU_LISTER_ARRANGE_DATE",
        ):
            self.assertIn(f"case {menu_id}:", event_loop)
            self.assertIn(f"case {menu_id}:", group)
            self.assertIn(f"case {menu_id}:", lister)

        self.assertIn("backdrop_cleanup(GUI->backdrop, BSORT_NAME + (id - MENU_LISTER_ARRANGE_NAME), CLEANUPF_ALIGN_OK);", event_loop)
        self.assertIn("backdrop_cleanup(group->info, BSORT_NAME + (id - MENU_LISTER_ARRANGE_NAME), 0);", group)
        self.assertIn("backdrop_cleanup(lister->backdrop_info, BSORT_NAME + (func - MENU_LISTER_ARRANGE_NAME), 0);", lister)


if __name__ == "__main__":
    unittest.main()
