#!/usr/bin/env python3
"""Patch generated AROS inline headers for ABIv11 void-return calls."""

from pathlib import Path
import re


DEFINE_DIR = Path(__file__).resolve().parent
HEADERS = (
    "dopus5.h",
    "newicon.h",
    "music.h",
    "multiuser.h",
    "SysInfo.h",
)
VOIDCALL_INCLUDE = "#include <defines/aros_voidcall.h>"


def remove_dopus_local_helpers(text):
    start = text.find("#define DOPUS_LC0NR")
    if start == -1:
        return text

    end_marker = "#define ActivateStrGad"
    end = text.find(end_marker, start)
    if end == -1:
        return text

    return text[:start] + text[end:]


def insert_voidcall_include(text):
    if VOIDCALL_INCLUDE in text or "AROS_VOID_LC" not in text:
        return text

    cast_include = "#include <aros/preprocessor/variadic/cast2iptr.hpp>"
    if cast_include in text:
        return text.replace(cast_include, f"{cast_include}\n{VOIDCALL_INCLUDE}", 1)

    libcall_guard = "#endif /* !AROS_LIBCALL_H */"
    return text.replace(libcall_guard, f"{libcall_guard}\n\n{VOIDCALL_INCLUDE}", 1)


def patch_header(path):
    text = path.read_text(encoding="utf-8")
    text = text.replace("\r\n", "\n")
    text = remove_dopus_local_helpers(text)
    text = re.sub(r"\bDOPUS_LC([0-9]+)NR\(", r"AROS_VOID_LC\1(", text)
    text = re.sub(r"\bAROS_LC([0-9]+)\(\s*void\s*,\s*", r"AROS_VOID_LC\1(", text)
    text = insert_voidcall_include(text)
    path.write_text(text, encoding="utf-8", newline="\n")


def main():
    for header in HEADERS:
        patch_header(DEFINE_DIR / header)


if __name__ == "__main__":
    main()
