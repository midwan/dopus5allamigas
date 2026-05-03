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

#ifndef _DOPUS_ICON
#define _DOPUS_ICON

/*****************************************************************************

 Icon support

 *****************************************************************************/

// Copy icon from one file to another
void CopyFileIcon(char *, char *);

// Special icon flags
#define ICONF_POSITION_OK (1 << 31)	 // Opus position stored in icon
#define ICONF_NO_BORDER (1 << 28)	 // No icon border
#define ICONF_NO_LABEL (1 << 27)	 // No label

ULONG GetIconFlags(struct DiskObject *);
void SetIconFlags(struct DiskObject *, ULONG);
void GetIconPosition(struct DiskObject *, short *, short *);
void SetIconPosition(struct DiskObject *, short, short);

// Icon caching
struct DiskObject *GetCachedDefDiskObject(long);
void FreeCachedDiskObject(struct DiskObject *);
struct DiskObject *GetCachedDiskObject(char *, long);
struct DiskObject *GetCachedDiskObjectNew(char *, long);

#endif
