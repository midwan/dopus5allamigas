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

#define CATCOMP_NUMBERS
#include "fixicons.strings"
#include "module_deps.h"

extern char *arg_template;

enum { FI_FILE, FI_ANB, FI_ANL, FI_NFO, FI_SWO, FI_SOW, FI_REPORT };

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct _FunctionEntry
{
	struct MinNode node;
	char *name;
	struct DirEntry *entry;
	short type;
	short flags;
} FunctionEntry;
#ifndef __amigaos3__
	#pragma pack()
#endif
