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

#ifndef _DOPUS_RESELECT
#define _DOPUS_RESELECT

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	struct DirectoryBuffer *buffer;	 // Buffer these came from
	APTR memory;					 // Memory pool
	struct MinList files;			 // List of files
	char *top_name;					 // Top file displayed
	long v_offset;					 // Vertical offset
	long h_offset;					 // Horizontal offset
	unsigned long flags;			 // Flags
} ReselectionData;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define RESELF_SAVE_FILETYPES (1 << 0)

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	struct MinNode node;
	unsigned short flags;	// Selection state
	char *filetype;			// Saved filetype pointer
	struct DateStamp date;	// File date
	char name[1];			// Filename
} ReselectionFile;

#ifndef __amigaos3__
	#pragma pack()
#endif

void InitReselect(ReselectionData *);
void MakeReselect(ReselectionData *, struct DirectoryBuffer *, ULONG);
void DoReselect(ReselectionData *, struct ListerWindow *, BOOL);
void GetReselectFiletypes(ReselectionData *, struct MinList *);
void FreeReselect(ReselectionData *);

#endif
