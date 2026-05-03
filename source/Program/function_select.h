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

#ifndef _DOPUS_FUNCTION_SELECT
#define _DOPUS_FUNCTION_SELECT

extern ObjectDef _function_select_objects[];
extern struct TagItem _select_layout_tags[];

enum {
	GAD_SELECT_LAYOUT_AREA,
	GAD_SELECT_NAME_STRING,
	GAD_SELECT_NAME_CYCLE,
	GAD_SELECT_DATE_FROM,
	GAD_SELECT_DATE_TO,
	GAD_SELECT_DATE_CYCLE,
	GAD_SELECT_PROT_CYCLE,
	GAD_SELECT_COMPARE_TYPE_CYCLE,
	GAD_SELECT_COMPARE_CYCLE,
	GAD_SELECT_INCLUDE_EXCLUDE,
	GAD_SELECT_ENTRY_TYPE,
	GAD_SELECT_CANCEL,
	GAD_SELECT_OK,
	GAD_SELECT_SIMPLE,
	GAD_SELECT_PROTECTION,
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	char type;		  // Simple or complex
	char entry_type;  // Type of entry to match

	char name[60];	// Filename pattern
	short name_match;
	char date_from[10];	 // Date from
	char date_to[10];	 // Date to
	short date_match;
	unsigned short bits;  // Protection bits
	short bits_match;
	short compare;	// Compare destination
	short compare_match;
	short include;	// Include/exclude
} SelectData;

#ifndef __amigaos3__
	#pragma pack()
#endif

enum { SELECT_SIMPLE, SELECT_COMPLEX };

#define SELECT_TYPE_MASK 1
#define SELECTF_MAKE_VISIBLE (1 << 2)
#define SELECTF_EXCLUSIVE (1 << 3)

enum { SELECT_MATCH_IGNORE, SELECT_MATCH_MATCH, SELECT_MATCH_NO_MATCH };

enum { SELECT_COMPARE_NEWER, SELECT_COMPARE_OLDER, SELECT_COMPARE_DIFFERENT };

enum { SELECT_INCLUDE, SELECT_EXCLUDE };

enum { SELECT_ENTRY_BOTH, SELECT_ENTRY_FILES, SELECT_ENTRY_DIRS };

enum {
	SELECT_VAR_NAME,
	SELECT_VAR_FROM,
	SELECT_VAR_TO,
	SELECT_VAR_BITSON,
	SELECT_VAR_BITSOFF,
	SELECT_VAR_COMPARE,
	SELECT_VAR_MATCHNAME,
	SELECT_VAR_NOMATCHNAME,
	SELECT_VAR_IGNORENAME,
	SELECT_VAR_MATCHDATE,
	SELECT_VAR_NOMATCHDATE,
	SELECT_VAR_IGNOREDATE,
	SELECT_VAR_MATCHBITS,
	SELECT_VAR_NOMATCHBITS,
	SELECT_VAR_IGNOREBITS,
	SELECT_VAR_MATCHCOMPARE,
	SELECT_VAR_NOMATCHCOMPARE,
	SELECT_VAR_IGNORECOMPARE,
	SELECT_VAR_BOTH,
	SELECT_VAR_FILESONLY,
	SELECT_VAR_DIRSONLY,
	SELECT_VAR_EXCLUDE,
	SELECT_VAR_INCLUDE
};

#endif
