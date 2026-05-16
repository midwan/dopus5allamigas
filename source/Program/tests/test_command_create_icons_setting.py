#!/usr/bin/env python3
"""Regression checks for New Command icon creation."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
PROGRAM = ROOT / "source" / "Program"
GUIDE = ROOT / "documents" / "DOpus5.guide"


def read_text(path):
    return path.read_text(encoding="latin-1")


def command_icon_block():
    source = read_text(PROGRAM / "commands.c")
    match = re.search(
        r"\s*// Write the default command icon if .*there isn't one already\n"
        r"(?P<block>.*?)"
        r"\n\s*// Only add as a leftout if not editing an existing function",
        source,
        re.S,
    )
    if not match:
        raise AssertionError("Could not find command icon creation block")
    return match.group("block")


def guide_node(name):
    source = read_text(GUIDE)
    match = re.search(rf'@node "{re.escape(name)}".*?(?=\n@endnode)', source, re.S)
    if not match:
        raise AssertionError(f"Could not find guide node {name}")
    return match.group(0)


class CommandCreateIconsSettingTests(unittest.TestCase):
    def test_new_command_only_writes_default_icon_when_create_icons_is_enabled(self):
        block = command_icon_block()

        self.assertIn("GUI->flags & GUIF_SAVE_ICONS", block)
        self.assertIn('StrCombine(info->buffer, buffer, ".info", sizeof(info->buffer));', block)
        self.assertIn("Lock(info->buffer, ACCESS_READ)", block)
        self.assertIn("icon_write(ICONTYPE_PROJECT, buffer, 0, 0, 0, 0)", block)
        self.assertRegex(
            block,
            r"(?s)if \(GUI->flags & GUIF_SAVE_ICONS\)\s*\{.*"
            r"if \(\(icon_lock = Lock\(info->buffer, ACCESS_READ\)\)\).*"
            r"else\s*icon_write\(ICONTYPE_PROJECT, buffer, 0, 0, 0, 0\);",
        )

    def test_new_command_documentation_mentions_create_icons_setting(self):
        node = guide_node("Icons - New Command")

        self.assertIn("left-out is still created", node)
        self.assertIn("Create Icons", node)
        self.assertIn("on-disk .info", node)


if __name__ == "__main__":
    unittest.main()
