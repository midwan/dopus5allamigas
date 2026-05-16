#!/usr/bin/env python3
"""Regression checks for AROS 64-bit DOpus ARexx message metadata."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]
REXX_C = ROOT / "source" / "Library" / "rexx.c"


def read_source():
    return REXX_C.read_text(encoding="latin-1")


class RexxAROS64TrackingTests(unittest.TestCase):
    def test_aros_does_not_store_pointer_in_rm_unused1(self):
        source = read_source()

        self.assertNotIn("#define rm_avail rm_Unused1", source)
        self.assertIn("struct DOpusRexxMsgTracker", source)
        self.assertIn("struct RexxMsg *msg;", source)
        self.assertIn("struct List *stems;", source)

    def test_dopus_stem_lists_are_tracked_outside_rexxmsg_on_aros(self):
        source = read_source()

        self.assertIn("static BOOL rexx_track_stem_list(struct RexxMsg *msg, struct List *list)", source)
        self.assertIn("static struct List *rexx_get_stem_list(struct RexxMsg *msg)", source)
        self.assertIn("static struct List *rexx_detach_stem_list(struct RexxMsg *msg)", source)
        self.assertIn("if (!rexx_track_stem_list(msg, list))", source)
        self.assertIn("if ((list = rexx_detach_stem_list(msg)))", source)
        self.assertIn("if (!(list = rexx_get_stem_list(msg)))", source)

    def test_free_rexx_msg_accepts_null(self):
        source = read_source()

        self.assertIn("if (!msg)\n\t\treturn;", source)


if __name__ == "__main__":
    unittest.main()
