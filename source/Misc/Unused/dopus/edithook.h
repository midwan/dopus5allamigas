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

#ifndef _DOPUS_EDITHOOK
#define _DOPUS_EDITHOOK

/*****************************************************************************

 String edit-hook

 *****************************************************************************/

#define EDITF_NO_SELECT_NEXT (1 << 0)  // Don't select next field
#define EDITF_PATH_FILTER (1 << 1)	   // Filter path characters
#define EDITF_SECURE (1 << 2)		   // Hidden password field

#define HOOKTYPE_STANDARD 0

#define EH_History TAG_USER + 33		// History list pointer
#define EH_ChangeSigTask TAG_USER + 46	// Task to signal on change
#define EH_ChangeSigBit TAG_USER + 47	// Signal bit to use

void FreeEditHook(struct Hook *);
struct Hook *GetEditHook(ULONG, ULONG, struct TagItem *tags);
struct Hook *__stdargs GetEditHookTags(ULONG, ULONG, Tag, ...);
char *GetSecureString(struct Gadget *);

#endif
