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

#ifndef _DOPUS_DISKINFO
#define _DOPUS_DISKINFO

extern ObjectDef _diskinfo_objects[];
extern unsigned short diskinfo_labels[];
extern struct TagItem diskinfo_info_tags[];

#define DISKINFO_ITEM_WIDTH 60

enum {
	GAD_DISKINFO_DEVICE,
	GAD_DISKINFO_NAME,
	GAD_DISKINFO_SIZE,
	GAD_DISKINFO_USED,
	GAD_DISKINFO_FREE,
	GAD_DISKINFO_PERCENT,
	GAD_DISKINFO_DENSITY,
	GAD_DISKINFO_ERRORS,
	GAD_DISKINFO_STATUS,
	GAD_DISKINFO_DATE,
	GAD_DISKINFO_FILESYSTEM,

	DISKINFO_ITEM_COUNT,

	DISKINFO_LAYOUT,
	DISKINFO_CONTINUE,
};

// Prototypes
void show_disk_info(FunctionHandle *, PathNode *, char *);

#endif
