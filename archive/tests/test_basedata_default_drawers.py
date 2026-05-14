#!/usr/bin/env python3
"""Regression checks for defaults shipped in basedata.lha."""

from pathlib import Path
import shutil
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[2]
BASEDATA = ROOT / "archive" / "basedata.lha"

REQUIRED_PLACEHOLDERS = (
    "DOpus5/Commands/.dummy",
    "DOpus5/Desktop/.dummy",
    "DOpus5/Groups/.dummy",
    "DOpus5/Themes/.dummy",
    "DOpus5/Sounds/.dummy",
)


def archive_tool():
    tool = shutil.which("lha")
    if tool:
        return "lha", tool

    tool = shutil.which("7z")
    if tool:
        return "7z", tool

    raise unittest.SkipTest("lha or 7z is required to inspect basedata.lha")


def archive_listing():
    _name, tool = archive_tool()

    return subprocess.run(
        [tool, "l", str(BASEDATA)],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    ).stdout


def archive_member(member):
    name, tool = archive_tool()
    if name == "lha":
        command = [tool, "pq", str(BASEDATA), member]
    else:
        command = [tool, "x", "-so", str(BASEDATA), member]

    return subprocess.run(
        command,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    ).stdout


class BasedataDefaultDrawerTests(unittest.TestCase):
    def test_default_empty_drawers_have_placeholders(self):
        listing = archive_listing()

        for path in REQUIRED_PLACEHOLDERS:
            with self.subTest(path=path):
                self.assertIn(path, listing)


class BasedataDefaultScriptTests(unittest.TestCase):
    def test_disk_inserted_leftover_sound_command_is_removed(self):
        scripts = archive_member("DOpus5/Buttons/Scripts")

        self.assertNotIn(b"REM Play QUIET work2:32dohs!.SND", scripts)
        self.assertNotIn(b"32dohs", scripts)


class BasedataDefaultEnvironmentTests(unittest.TestCase):
    def test_workbench_environment_matches_default_environment(self):
        default = archive_member("DOpus5/Environment/default")
        workbench = archive_member("DOpus5/Environment/workbench")

        self.assertEqual(default, workbench)


if __name__ == "__main__":
    unittest.main()
