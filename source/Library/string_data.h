/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
AROS Public License for more details.

The release of Directory Opus 5 under the GPL in NO WAY affects
the existing commercial status of Directory Opus for Windows.

For more information on Directory Opus for Windows please see:

				 http://www.gpsoft.com.au

*/

#ifndef STRING_DATA_H
#define STRING_DATA_H

/****************************************************************************/

/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
	#undef CATCOMP_NUMBERS
	#undef CATCOMP_STRINGS
	#define CATCOMP_NUMBERS
	#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
	#undef CATCOMP_STRINGS
	#define CATCOMP_STRINGS
#endif

/****************************************************************************/

#ifdef CATCOMP_NUMBERS

	#define MSG_OK 100
	#define MSG_SELECT_FILE 101
	#define MSG_SELECT_DIRECTORY 102
	#define MSG_SELECT_FONT 103
	#define MSG_FONT_DRAWMODE 104
	#define MSG_FONT_TEXT 105
	#define MSG_FONT_FIELD_TEXT 106
	#define MSG_SIZE_FIT 107
	#define MSG_SIZE 108
	#define MSG_ABORT 109
	#define MSG_FILE_FIT 110
	#define MSG_FILE 111
	#define MSG_DIRECTORY_OPUS_REQUEST 112
	#define MSG_UNABLE_TO_OPEN_TOOL 1000
	#define MSG_SICK_OF_WAITING 1001
	#define MSG_WAIT_CANCEL 1002

#endif /* CATCOMP_NUMBERS */

/****************************************************************************/

#ifdef CATCOMP_STRINGS

	#define MSG_OK_STR "OK"
	#define MSG_SELECT_FILE_STR "Select File"
	#define MSG_SELECT_DIRECTORY_STR "Select Directory"
	#define MSG_SELECT_FONT_STR "Select Font"
	#define MSG_FONT_DRAWMODE_STR "Mode:"
	#define MSG_FONT_TEXT_STR "Text"
	#define MSG_FONT_FIELD_TEXT_STR "Text+Field"
	#define MSG_SIZE_FIT_STR "9999%%"
	#define MSG_SIZE_STR "%ld%%"
	#define MSG_ABORT_STR "Abort"
	#define MSG_FILE_FIT_STR "9999 of 9999"
	#define MSG_FILE_STR "%ld of %ld"
	#define MSG_DIRECTORY_OPUS_REQUEST_STR "Directory Opus Request"
	#define MSG_UNABLE_TO_OPEN_TOOL_STR "Unable to open your tool '%s'"
	#define MSG_SICK_OF_WAITING_STR "Opus is sick of waiting for '%s' to return.\nWait some more?"
	#define MSG_WAIT_CANCEL_STR "Wait|Cancel"

#endif /* CATCOMP_STRINGS */

/****************************************************************************/

#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
	LONG cca_ID;
	STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] = {
	{MSG_OK, (STRPTR)MSG_OK_STR},
	{MSG_SELECT_FILE, (STRPTR)MSG_SELECT_FILE_STR},
	{MSG_SELECT_DIRECTORY, (STRPTR)MSG_SELECT_DIRECTORY_STR},
	{MSG_SELECT_FONT, (STRPTR)MSG_SELECT_FONT_STR},
	{MSG_FONT_DRAWMODE, (STRPTR)MSG_FONT_DRAWMODE_STR},
	{MSG_FONT_TEXT, (STRPTR)MSG_FONT_TEXT_STR},
	{MSG_FONT_FIELD_TEXT, (STRPTR)MSG_FONT_FIELD_TEXT_STR},
	{MSG_SIZE_FIT, (STRPTR)MSG_SIZE_FIT_STR},
	{MSG_SIZE, (STRPTR)MSG_SIZE_STR},
	{MSG_ABORT, (STRPTR)MSG_ABORT_STR},
	{MSG_FILE_FIT, (STRPTR)MSG_FILE_FIT_STR},
	{MSG_FILE, (STRPTR)MSG_FILE_STR},
	{MSG_DIRECTORY_OPUS_REQUEST, (STRPTR)MSG_DIRECTORY_OPUS_REQUEST_STR},
	{MSG_UNABLE_TO_OPEN_TOOL, (STRPTR)MSG_UNABLE_TO_OPEN_TOOL_STR},
	{MSG_SICK_OF_WAITING, (STRPTR)MSG_SICK_OF_WAITING_STR},
	{MSG_WAIT_CANCEL, (STRPTR)MSG_WAIT_CANCEL_STR},
};

#endif /* CATCOMP_ARRAY */

/****************************************************************************/

#ifdef CATCOMP_BLOCK

static const char CatCompBlock[] = {"\x00\x00\x00\x64\x00\x04" MSG_OK_STR
									"\x00\x00"
									"\x00\x00\x00\x65\x00\x0C" MSG_SELECT_FILE_STR
									"\x00"
									"\x00\x00\x00\x66\x00\x12" MSG_SELECT_DIRECTORY_STR
									"\x00\x00"
									"\x00\x00\x00\x67\x00\x0C" MSG_SELECT_FONT_STR
									"\x00"
									"\x00\x00\x00\x68\x00\x06" MSG_FONT_DRAWMODE_STR
									"\x00"
									"\x00\x00\x00\x69\x00\x06" MSG_FONT_TEXT_STR
									"\x00\x00"
									"\x00\x00\x00\x6A\x00\x0C" MSG_FONT_FIELD_TEXT_STR
									"\x00\x00"
									"\x00\x00\x00\x6B\x00\x08" MSG_SIZE_FIT_STR
									"\x00\x00"
									"\x00\x00\x00\x6C\x00\x06" MSG_SIZE_STR
									"\x00"
									"\x00\x00\x00\x6D\x00\x06" MSG_ABORT_STR
									"\x00"
									"\x00\x00\x00\x6E\x00\x0E" MSG_FILE_FIT_STR
									"\x00\x00"
									"\x00\x00\x00\x6F\x00\x0C" MSG_FILE_STR
									"\x00\x00"
									"\x00\x00\x00\x70\x00\x18" MSG_DIRECTORY_OPUS_REQUEST_STR
									"\x00\x00"
									"\x00\x00\x03\xE8\x00\x1E" MSG_UNABLE_TO_OPEN_TOOL_STR
									"\x00"
									"\x00\x00\x03\xE9\x00\x3C" MSG_SICK_OF_WAITING_STR
									"\x00"
									"\x00\x00\x03\xEA\x00\x0C" MSG_WAIT_CANCEL_STR "\x00"};

#endif /* CATCOMP_BLOCK */

/****************************************************************************/

struct LocaleInfo
{
	APTR li_LocaleBase;
	APTR li_Catalog;
};

#ifdef CATCOMP_CODE

STRPTR GetString(struct LocaleInfo *li, LONG stringNum)
{
	LONG *l;
	UWORD *w;
	STRPTR builtIn;

	l = (LONG *)CatCompBlock;

	while (*l != stringNum)
	{
		w = (UWORD *)((ULONG)l + 4);
		l = (LONG *)((ULONG)l + (ULONG)*w + 6);
	}
	builtIn = (STRPTR)((ULONG)l + 6);

	#define XLocaleBase LocaleBase
	#define LocaleBase li->li_LocaleBase

	if (LocaleBase)
		return (GetCatalogStr(li->li_Catalog, stringNum, builtIn));
	#define LocaleBase XLocaleBase
	#undef XLocaleBase

	return (builtIn);
}

#endif /* CATCOMP_CODE */

/****************************************************************************/

#endif /* STRING_DATA_H */
