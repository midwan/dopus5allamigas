#!/usr/bin/env python3
"""Regression checks for the format.module LNFS availability gate."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[4]
FORMAT_C = ROOT / "source" / "Modules" / "format" / "format.c"


def read_format_source():
    return FORMAT_C.read_text(encoding="latin-1")


class LnfsGateTests(unittest.TestCase):
    def test_lnfs_requires_v46_filesystem(self):
        source = read_format_source()

        self.assertIn("#define FORMAT_LNFS_MIN_FS_VERSION 46", source)
        self.assertIn("format_filesystem_supports_lnfs", source)
        self.assertIn("GetFileVersion(handler_name, &version, &revision, 0, 0)", source)

    def test_unsupported_lnfs_is_cleared_before_disable(self):
        source = read_format_source()

        self.assertIn("lnfs_supported = format_filesystem_supports_lnfs(dos_type, handler_name)", source)
        self.assertIn("SetGadgetValue(data->list, GAD_FORMAT_LNFS, 0)", source)
        self.assertIn("DisableObject(data->list, GAD_FORMAT_LNFS, not_dos || !lnfs_supported)", source)


if __name__ == "__main__":
    unittest.main()
