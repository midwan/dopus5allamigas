#!/usr/bin/env python3
"""Regression checks for Environment panel vertical spacing."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[4]
ENVIRONMENT_DATA_C = ROOT / "source" / "Modules" / "configopus" / "config_environment_data.c"
CONFIG_ENVIRONMENT_C = ROOT / "source" / "Modules" / "configopus" / "config_environment.c"
CONFIGOPUS_ENUMS_H = ROOT / "source" / "Modules" / "configopus" / "enums.h"
CONFIGOPUS_DIR = ROOT / "source" / "Modules" / "configopus"
CONFIGOPUS_STRINGS_H = CONFIGOPUS_DIR / "string_data.h"
DOPUS5_H = ROOT / "source" / "Include" / "libraries" / "dopus5.h"
DOPUS_CONFIG_H = ROOT / "source" / "Program" / "dopus_config.h"


def read_layout_source(layout_name):
    source = ENVIRONMENT_DATA_C.read_text(encoding="latin-1")
    pattern = re.escape(layout_name) + r"\[\]\s*=\s*\{(?P<body>.*?)\n\s*\{OD_END\}\}"
    match = re.search(pattern, source, re.S)
    if not match:
        raise AssertionError(f"Could not find {layout_name} layout")
    return match.group("body")


def read_function_source(function_name):
    source = CONFIG_ENVIRONMENT_C.read_text(encoding="latin-1")
    start = source.find(f"void {function_name}")
    if start == -1:
        raise AssertionError(f"Could not find {function_name}")

    end = source.find("\nvoid ", start + 1)
    if end == -1:
        end = len(source)
    return source[start:end]


def read_case_source(function_name, case_name):
    source = read_function_source(function_name)
    pattern = r"\n\tcase " + re.escape(case_name) + r":(?P<body>.*?)(?=\n\tcase |\n\tdefault:|\n\t})"
    match = re.search(pattern, source, re.S)
    if not match:
        raise AssertionError(f"Could not find {case_name} in {function_name}")
    return match.group("body")


class ListerOptionsLayoutTests(unittest.TestCase):
    def test_dual_lister_default_option_uses_next_lister_option_bit(self):
        for header in (DOPUS5_H, DOPUS_CONFIG_H):
            with self.subTest(header=header.name):
                source = header.read_text(encoding="latin-1")
                self.assertRegex(source, r"#define\s+LISTEROPTF_DUAL_DEFAULT\s+\(1 << 14\)")

    def test_dual_lister_default_uses_lister_display_panel(self):
        lister_display_layout = read_layout_source("_environment_lister_gadgets")
        defaults_layout = read_layout_source("_environment_listersize")
        options_layout = read_layout_source("_environment_listeroptions")
        enums = CONFIGOPUS_ENUMS_H.read_text(encoding="latin-1")
        data_source = ENVIRONMENT_DATA_C.read_text(encoding="latin-1")

        self.assertIn("GAD_ENVIRONMENT_DUAL_DEFAULT", enums)
        self.assertNotIn("ENVIRONMENT_DUAL_LISTER", enums)
        self.assertNotIn("MSG_ENVIRONMENT_SUB_DUAL_LISTER", data_source)
        self.assertNotIn("_environment_dual_lister", data_source)
        self.assertIn("GAD_ENVIRONMENT_DUAL_DEFAULT", lister_display_layout)
        self.assertIn("MSG_ENVIRONMENT_DUAL_DEFAULT", lister_display_layout)
        self.assertRegex(
            lister_display_layout,
            r"(?s)MSG_ENVIRONMENT_DUAL_DEFAULT,\s*\n\s*PLACETEXT_RIGHT,\s*\n\s*GAD_ENVIRONMENT_DUAL_DEFAULT",
        )
        self.assertNotIn("GAD_ENVIRONMENT_DUAL_DEFAULT", defaults_layout)
        self.assertNotIn("MSG_ENVIRONMENT_DUAL_DEFAULT", defaults_layout)
        self.assertNotIn("GAD_ENVIRONMENT_DUAL_DEFAULT", options_layout)
        self.assertNotIn("MSG_ENVIRONMENT_DUAL_DEFAULT", options_layout)

    def test_lister_display_panel_saves_and_loads_dual_default_flag(self):
        lister_display_load = read_case_source("_config_env_set", "ENVIRONMENT_LISTER_DISPLAY")
        lister_display_store = read_case_source("_config_env_store", "ENVIRONMENT_LISTER_DISPLAY")
        lister_defaults_load = read_case_source("_config_env_set", "ENVIRONMENT_LISTER_SIZE")
        lister_defaults_store = read_case_source("_config_env_store", "ENVIRONMENT_LISTER_SIZE")
        lister_options_load = read_case_source("_config_env_set", "ENVIRONMENT_LISTER_OPTIONS")
        lister_options_store = read_case_source("_config_env_store", "ENVIRONMENT_LISTER_OPTIONS")

        self.assertRegex(
            lister_display_load,
            r"SetGadgetValue\([^;]*GAD_ENVIRONMENT_DUAL_DEFAULT[^;]*LISTEROPTF_DUAL_DEFAULT[^;]*\);",
        )
        self.assertRegex(
            lister_display_store,
            r"GetGadgetValue\([^)]*GAD_ENVIRONMENT_DUAL_DEFAULT[^)]*\)[^;{]*\)\s*"
            r"data->config->lister_options \|= LISTEROPTF_DUAL_DEFAULT;",
        )
        self.assertIn("data->config->lister_options &= ~LISTEROPTF_DUAL_DEFAULT;", lister_display_store)
        self.assertNotIn("GAD_ENVIRONMENT_DUAL_DEFAULT", lister_defaults_load)
        self.assertNotIn("GAD_ENVIRONMENT_DUAL_DEFAULT", lister_defaults_store)
        self.assertNotIn("GAD_ENVIRONMENT_DUAL_DEFAULT", lister_options_load)
        self.assertNotIn("GAD_ENVIRONMENT_DUAL_DEFAULT", lister_options_store)

    def test_environment_suboption_initialisation_requires_object_list(self):
        source = CONFIG_ENVIRONMENT_C.read_text(encoding="latin-1")

        self.assertRegex(
            source,
            r"(?s)if \(\(data->option_list =\s*"
            r"AddObjectList\(data->window, _environment_options\[data->option_node->data\]\.objects\)\)\)\s*\{\s*"
            r".*?_config_env_set\(data, data->option\);",
        )
        self.assertRegex(
            source,
            r"(?s)data->option_list =\s*"
            r"AddObjectList\(data->window, _environment_options\[data->option_node->data\]\.objects\);\s*"
            r"#ifndef FUNKOFF\s*"
            r".*?"
            r"if \(data->option_list\)\s*"
            r"_config_env_set\(data, data->option\);",
        )

    def test_lister_options_controls_are_lifted_from_panel_bottom_edge(self):
        layout = read_layout_source("_environment_listeroptions")

        self.assertIn("{5, -4, 0, 0}", layout)
        self.assertIn("{5, 0, 26, 4}", layout)
        self.assertIn("{5, 48, 26, 4}", layout)
        self.assertIn("{5, 54, 24, 6}", layout)
        self.assertNotIn("{5, 58, 24, 6}", layout)
        self.assertNotIn("{5, 60, 24, 6}", layout)

    def test_configopus_builds_recompile_builtin_catalog_when_strings_change(self):
        for makefile in ("makefile.os3", "makefile.os4", "makefile.mos", "makefile.aros"):
            with self.subTest(makefile=makefile):
                source = (CONFIGOPUS_DIR / makefile).read_text(encoding="latin-1")
                self.assertRegex(source, r"\$\(OBJDIR\)/string_data\.o\s*:\s*string_data\.h")

    def test_icon_layout_title_keeps_panel_title_ellipsis(self):
        cd_source = (CONFIGOPUS_DIR / "configopus.cd").read_text(encoding="latin-1")
        string_source = CONFIGOPUS_STRINGS_H.read_text(encoding="latin-1")

        self.assertIn("Desktop Icon Arrangement...", cd_source)
        self.assertIn(
            '#define MSG_ENVIRONMENT_ICON_LAYOUT_TITLE_STR "Desktop Icon Arrangement..."',
            string_source,
        )
        self.assertRegex(
            string_source,
            r'"\\x00\\x00\\x59\\xF4\\x00\\x1D" MSG_ENVIRONMENT_ICON_LAYOUT_TITLE_STR\s*\n'
            r'\s*"\\x00\\x00"\s*\n'
            r'\s*"\\x00\\x00\\x59\\xF5\\x00\\x12" MSG_ENVIRONMENT_ICON_SPACE_X_STR',
        )

    def test_dual_lister_default_catalog_block_has_even_string_padding(self):
        source = CONFIGOPUS_STRINGS_H.read_text(encoding="latin-1")

        self.assertRegex(
            source,
            r'"\\x00\\x00\\x08\\x06\\x00\\x20" MSG_ENVIRONMENT_DUAL_DEFAULT_STR\s*\n'
            r'\s*"\\x00\\x00"\s*\n'
            r'\s*"\\x00\\x00\\x08\\x07\\x00\\x0E" MSG_ENVIRONMENT_LISTER_STATUS_STR',
        )

    def test_misc_controls_are_lifted_from_panel_bottom_edge(self):
        layout = read_layout_source("_environment_misc_gadgets")

        self.assertIn("{5, 3, 0, 0}", layout)
        self.assertIn("{5, 6, 26, 4}", layout)
        self.assertIn("{5, 48, 8, 6}", layout)
        self.assertIn("{5, 62, 28, 6}", layout)
        self.assertIn("{33, 62, -8, 6}", layout)
        self.assertNotIn("{5, 70, 28, 6}", layout)


if __name__ == "__main__":
    unittest.main()
