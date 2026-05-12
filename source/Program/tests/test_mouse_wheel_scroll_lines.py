#!/usr/bin/env python3
"""Regression checks for lister mouse wheel scroll-line handling."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[3]
COMMON_H = ROOT / "source" / "Include" / "dopus" / "common.h"
LISTER_IDCMP_C = ROOT / "source" / "Program" / "lister_idcmp.c"
LISTER_WINDOW_C = ROOT / "source" / "Program" / "lister_window.c"
PROGRAM_MISC_H = ROOT / "source" / "Program" / "misc.h"
READ_C = ROOT / "source" / "Modules" / "read" / "read.c"
READ_H = ROOT / "source" / "Modules" / "read" / "read.h"


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


def read_native_wheel_case():
    source = read_source(READ_C)
    match = re.search(
        r"#ifdef\s+IDCMP_EXTENDEDMOUSE(?P<case>.*?)#endif\s*\n\n\s*// Key press",
        source,
        re.S,
    )
    if not match:
        raise AssertionError("Could not find IDCMP_EXTENDEDMOUSE read handler")
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


def read_window_idcmp_flags():
    source = read_source(READ_C)
    match = re.search(
        r"WA_IDCMP,\n(?P<flags>.*?IDCMP_MOUSEMOVE,)",
        source,
        re.S,
    )
    if not match:
        raise AssertionError("Could not find read WA_IDCMP flags")
    return match.group("flags")


class MouseWheelScrollLinesTests(unittest.TestCase):
    def test_native_wheel_support_threshold_is_shared(self):
        common = read_source(COMMON_H)
        lister_idcmp = read_source(LISTER_IDCMP_C)
        lister_window = read_source(LISTER_WINDOW_C)
        window_flags = lister_window_idcmp_flags()

        self.assertIn("#define DOPUS_NATIVE_WHEEL_MIN_VERSION 47", common)
        self.assertIn("DOPUS_NATIVE_WHEEL_SUPPORTED()", common)
        self.assertNotIn("static BOOL lister_native_wheel_supported", lister_idcmp)
        self.assertNotIn("static BOOL lister_native_wheel_supported", lister_window)

        self.assertIn("DOPUS_NATIVE_WHEEL_SUPPORTED()", window_flags)
        self.assertIn("DOPUS_NATIVE_WHEEL_SUPPORTED()", native_wheel_case())

    def test_native_wheel_events_are_enabled_when_headers_expose_them(self):
        window_flags = lister_window_idcmp_flags()

        self.assertIn("#ifdef IDCMP_EXTENDEDMOUSE", window_flags)
        self.assertIn("#ifdef IDCMP_EXTENDEDMOUSE", read_source(LISTER_IDCMP_C))

    def test_native_wheel_modifier_events_execute_lister_scroll_actions(self):
        case = native_wheel_case()

        self.assertIn("lister_handle_vertical_scroll_key(lister, PAGEUP, 1)", case)
        self.assertIn("lister_handle_vertical_scroll_key(lister, HOME, 1)", case)
        self.assertIn("lister_handle_vertical_scroll_key(lister, PAGEDOWN, 1)", case)
        self.assertIn("lister_handle_vertical_scroll_key(lister, END, 1)", case)

    def test_native_wheel_scroll_uses_configured_lister_line_count(self):
        case = native_wheel_case()

        self.assertIn("lister_configured_wheel_scroll_lines()", case)
        self.assertIn("lister_scroll(lister, 0, scroll_line * iwd->WheelY)", case)
        self.assertNotIn("ABS(iwd->WheelY)", case)

    def test_rawkey_wheel_codes_remain_handled_for_native_os3_and_legacy_sources(self):
        common = read_source(COMMON_H)
        up, down = rawkey_wheel_cases()

        self.assertIn("#define RAWKEY_WHEEL_UP 0x7a", common)
        self.assertIn("#define RAWKEY_WHEEL_DOWN 0x7b", common)
        self.assertNotIn("DOPUS_NATIVE_WHEEL_SUPPORTED()", up)
        self.assertNotIn("DOPUS_NATIVE_WHEEL_SUPPORTED()", down)
        self.assertIn("lister_configured_wheel_scroll_lines()", up)
        self.assertIn("lister_configured_wheel_scroll_lines()", down)

    def test_read_native_wheel_uses_same_runtime_gate_and_scroll_lines(self):
        window_flags = read_window_idcmp_flags()
        case = read_native_wheel_case()

        self.assertIn("#ifdef IDCMP_EXTENDEDMOUSE", window_flags)
        self.assertIn("DOPUS_NATIVE_WHEEL_SUPPORTED()", window_flags)
        self.assertIn("DOPUS_NATIVE_WHEEL_SUPPORTED()", case)
        self.assertIn("msg_copy.Code == IMSGCODE_INTUIWHEELDATA", case)
        self.assertIn("struct IntuiWheelData *iwd = (struct IntuiWheelData *)msg_copy.IAddress", case)
        self.assertIn("msg_copy.Qualifier", case)
        self.assertIn("read_wheel_scroll_lines(data)", case)
        self.assertIn("read_update_text(data, 0, scroll_line * iwd->WheelY, 0)", case)
        self.assertIn("read_update_text(data, 0, -(data->v_visible - 1), 0)", case)
        self.assertIn("read_update_text(data, 0, -data->top, 0)", case)
        self.assertIn("read_update_text(data, 0, data->v_visible - 1, 0)", case)
        self.assertIn("read_update_text(data, 0, (data->lines - data->top) - data->v_visible, 0)", case)

    def test_read_rawkey_wheel_uses_configured_line_count(self):
        source = read_source(READ_C)

        self.assertIn("case RAWKEY_WHEEL_UP:", source)
        self.assertIn("case RAWKEY_WHEEL_DOWN:", source)
        self.assertIn("(msg_copy.Code == RAWKEY_WHEEL_UP) ? read_wheel_scroll_lines(data) : 1", source)
        self.assertIn("(msg_copy.Code == RAWKEY_WHEEL_DOWN) ? read_wheel_scroll_lines(data) : 1", source)

    def test_read_startup_carries_configured_wheel_scroll_lines(self):
        read_h = read_source(READ_H)
        misc_h = read_source(PROGRAM_MISC_H)
        program_sources = "\n".join(
            read_source(ROOT / path)
            for path in (
                "source/Program/function_show.c",
                "source/Program/function_script.c",
                "source/Program/function_search.c",
                "source/Program/misc.c",
                "source/Program/rexx_proc.c",
            )
        )

        self.assertIn("UWORD wheel_scroll_lines;", read_h)
        self.assertIn("UWORD wheel_scroll_lines;", misc_h)
        self.assertGreaterEqual(program_sources.count("wheel_scroll_lines = environment->env->env_wheel_scroll_lines"), 6)


if __name__ == "__main__":
    unittest.main()
