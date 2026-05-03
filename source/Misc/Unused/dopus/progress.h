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

#ifndef _DOPUS_PROGRESS
#define _DOPUS_PROGRESS

/*****************************************************************************

 Progress window

 *****************************************************************************/

#ifndef UTILITY_TAGITEM_H
	#include <utility/tagitem.h>
#endif

#define PW_Screen TAG_USER + 0	   // Screen to open on
#define PW_Window TAG_USER + 1	   // Owner window
#define PW_Title TAG_USER + 2	   // Window title
#define PW_SigTask TAG_USER + 3	   // Task to signal
#define PW_SigBit TAG_USER + 4	   // Signal bit
#define PW_Flags TAG_USER + 5	   // Flags
#define PW_FileName TAG_USER + 6   // File name
#define PW_FileSize TAG_USER + 7   // File size
#define PW_FileDone TAG_USER + 8   // File done
#define PW_FileCount TAG_USER + 9  // Number of files
#define PW_FileNum TAG_USER + 10   // Current number
#define PW_Info TAG_USER + 11	   // Information line 1
#define PW_Info2 TAG_USER + 12	   // Information line 2
#define PW_Info3 TAG_USER + 13	   // Information line 3

#define PWF_FILENAME (1 << 0)	// Filename display
#define PWF_FILESIZE (1 << 1)	// Filesize display
#define PWF_INFO (1 << 2)		// Information line 1
#define PWF_GRAPH (1 << 3)		// Bar graph display
#define PWF_NOABORT (1 << 4)	// No abort gadget
#define PWF_INVISIBLE (1 << 5)	// Open invisibly
#define PWF_ABORT (1 << 6)		// Want abort gadget
#define PWF_SWAP (1 << 7)		// Swap bar and size displays
#define PWF_INFO2 (1 << 10)		// Information line 2
#define PWF_INFO3 (1 << 11)		// Information line 3

BOOL CheckProgressAbort(APTR);
void CloseProgressWindow(APTR);
void GetProgressWindow(APTR, struct TagItem *);
void HideProgressWindow(APTR);
APTR OpenProgressWindow(struct TagItem *);
void SetProgressWindow(APTR, struct TagItem *);
void ShowProgressWindow(APTR, struct Screen *, struct Window *);

APTR __stdargs OpenProgressWindowTags(Tag, ...);
void __stdargs SetProgressWindowTags(APTR, Tag, ...);
void __stdargs GetProgressWindowTags(APTR, Tag, ...);

#endif
