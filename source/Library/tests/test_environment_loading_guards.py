#!/usr/bin/env python3
"""Regression checks for defensive environment loading paths."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]
CONFIG_OPEN_C = ROOT / "source" / "Library" / "config_open.c"
ENVIRONMENT_C = ROOT / "source" / "Program" / "environment.c"


def read_source(path):
    return path.read_text(encoding="latin-1")


def slice_between(source, start, end):
    start_at = source.index(start)
    end_at = source.index(end, start_at)
    return source[start_at:end_at]


class EnvironmentLoadingGuardTests(unittest.TestCase):
    def test_environment_open_uses_loaded_env_for_max_filename(self):
        source = read_source(ENVIRONMENT_C)
        block = slice_between(source, "// Get maximum filename length", "// Successful?")

        self.assertIn("GUI->def_filename_length = env->env->settings.max_filename;", block)
        self.assertNotIn("environment->env->settings.max_filename", block)
        self.assertIn("else if (GUI->def_filename_length > MAX_FILENAME_LEN)", block)

    def test_failed_old_lister_conversion_frees_lister_def(self):
        source = read_source(CONFIG_OPEN_C)
        function = slice_between(source, "Cfg_Lister *LIBFUNC L_ReadListerDef", "// Read a button bank")

        self.assertRegex(
            function,
            r"if \(!convert_open_lister\(iff, &lister->lister\)\)\s*\{\s*"
            r"L_FreeListerDef\(lister\);\s*return NULL;\s*\}",
        )

    def test_failed_old_env_conversion_closes_iff(self):
        source = read_source(CONFIG_OPEN_C)
        function = slice_between(source, "BOOL LIBFUNC L_OpenEnvironment", "// Read info on a button bank")

        self.assertRegex(
            function,
            r"if \(!convert_env\(iff, &data->env\)\)\s*\{\s*"
            r"L_IFFClose\(iff\);\s*iff_file_id = 0;\s*return 0;\s*\}",
        )

    def test_epus_env_read_zeros_destination_for_short_chunks(self):
        source = read_source(CONFIG_OPEN_C)
        function = slice_between(source, "BOOL LIBFUNC L_OpenEnvironment", "// Read info on a button bank")

        self.assertRegex(
            function,
            r"else\s*\{\s*memset\(&data->env, 0, sizeof\(data->env\)\);\s*"
            r"L_IFFReadChunkBytes\(iff, &data->env, sizeof\(CFG_ENVR\)\);\s*\}",
        )

    def test_sndx_read_stays_within_saved_payload_and_terminates_strings(self):
        source = read_source(CONFIG_OPEN_C)
        sndx_case = slice_between(source, "case ID_SNDX:", "break;\n\t\t}")

        self.assertIn("offsetof(Cfg_SoundEntry, dse_Random)", sndx_case)
        self.assertIn("offsetof(Cfg_SoundEntry, dse_Name)", sndx_case)
        self.assertIn("memset(sound, 0, sizeof(Cfg_SoundEntry));", sndx_case)
        self.assertIn("sound->dse_Name[sizeof(sound->dse_Name) - 1] = 0;", sndx_case)
        self.assertIn("sound->dse_Sound[sizeof(sound->dse_Sound) - 1] = 0;", sndx_case)

    def test_old_env_conversion_copies_both_lister_priorities(self):
        source = read_source(CONFIG_OPEN_C)
        function = slice_between(
            source,
            "int convert_env(struct _IFFHandle *iff, CFG_ENVR *env)\n{",
            "void convert_list_format",
        )

        self.assertIn("env->settings.pri_lister[0] = oldenv->settings.pri_lister[0];", function)
        self.assertIn("env->settings.pri_lister[1] = oldenv->settings.pri_lister[1];", function)
        self.assertEqual(
            function.count("env->settings.pri_lister[0] = oldenv->settings.pri_lister[0];"),
            1,
        )


if __name__ == "__main__":
    unittest.main()
