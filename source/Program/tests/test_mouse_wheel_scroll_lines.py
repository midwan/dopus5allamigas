#!/usr/bin/env python3
"""Regression checks for lister mouse wheel scroll-line handling."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
LISTER_IDCMP_C = ROOT / "source" / "Program" / "lister_idcmp.c"
LISTER_WINDOW_C = ROOT / "source" / "Program" / "lister_window.c"


def read_source(path):
    return path.read_text(encoding="latin-1")


def native_wheel_case():
    source = read_source(LISTER_IDCMP_C)
    match = re.search(
        r"#ifdef\s+IDCMP_EXTENDEDMOUSE(?P<case>.*?)#endif\s*\n\n\t// Key press",
        source,
        re.S,
    )
    if not match:
        raise AssertionError("Could not find IDCMP_EXTENDEDMOUSE lister handler")
    return match.group("case")


def rawkey_wheel_cases():
    source = read_source(LISTER_IDCMP_C)
    up = re.search(
        r"else if \(msg->Code == RAWKEY_WHEEL_UP\)(?P<case>.*?)\n\t\t\telse if \(msg->Code == RAWKEY_WHEEL_DOWN\)",
        source,
        re.S,
    )
    down = re.search(
        r"else if \(msg->Code == RAWKEY_WHEEL_DOWN\)(?P<case>.*?)\n\n\t\t\t// Look at key",
        source,
        re.S,
    )
    if not up or not down:
        raise AssertionError("Could not find RAWKEY wheel handlers")
    return up.group("case"), down.group("case")


def lister_window_idcmp_flags():
    source = read_source(LISTER_WINDOW_C)
    match = re.search(
        r"WA_IDCMP,\n(?P<flags>.*?IDCMP_NEWSIZE \| IDCMP_REFRESHWINDOW \| IDCMP_RAWKEY,)",
        source,
        re.S,
    )
    if not match:
        raise AssertionError("Could not find lister WA_IDCMP flags")
    return match.group("flags")


class MouseWheelScrollLinesTests(unittest.TestCase):
    def test_native_wheel_support_is_runtime_gated(self):
        source = read_source(LISTER_IDCMP_C)
        window_flags = lister_window_idcmp_flags()

        self.assertIn("lister_native_wheel_supported", source)
        self.assertIn("#ifdef IDCMP_EXTENDEDMOUSE", source)
        self.assertIn("lib_Version >= 47", source)

        self.assertIn("lister_native_wheel_supported()", window_flags)

    def test_native_wheel_events_are_enabled_when_headers_expose_them(self):
        window_flags = lister_window_idcmp_flags()

        self.assertIn("#ifdef IDCMP_EXTENDEDMOUSE", window_flags)
        self.assertIn("#ifdef IDCMP_EXTENDEDMOUSE", read_source(LISTER_IDCMP_C))

        self.assertNotIn("__amigaos4__", window_flags)
        self.assertNotIn("__amigaos4__", native_wheel_case())

    def test_native_wheel_scroll_uses_configured_lister_line_count(self):
        case = native_wheel_case()

        self.assertIn("env_wheel_scroll_lines", case)
        self.assertNotRegex(case, r"lister_scroll\(lister,\s*0,\s*-1\)")
        self.assertNotRegex(case, r"lister_scroll\(lister,\s*0,\s*1\)")

    def test_rawkey_wheel_codes_remain_handled_for_native_os3_and_legacy_sources(self):
        source = read_source(LISTER_IDCMP_C)
        up, down = rawkey_wheel_cases()

        self.assertIn("#define RAWKEY_WHEEL_UP 0x7a", source)
        self.assertIn("#define RAWKEY_WHEEL_DOWN 0x7b", source)
        self.assertNotIn("lister_native_wheel_supported()", up)
        self.assertNotIn("lister_native_wheel_supported()", down)
        self.assertIn("env_wheel_scroll_lines", up)
        self.assertIn("env_wheel_scroll_lines", down)


if __name__ == "__main__":
    unittest.main()
