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

#ifndef _DOPUS_IFF
#define _DOPUS_IFF

/*****************************************************************************

 IFF routines

 *****************************************************************************/

// File modes
#define IFF_READ MODE_OLDFILE				   // Reading
#define IFF_WRITE MODE_NEWFILE				   // Writing
#define IFF_CLIP 0x8000						   // Clipboard flag
#define IFF_CLIP_READ (IFF_CLIP | IFF_READ)	   // Read clipboard
#define IFF_CLIP_WRITE (IFF_CLIP | IFF_WRITE)  // Write clipboard
#define IFF_SAFE 0x4000						   // Safe write

ULONG IFFChunkID(APTR);
long IFFChunkRemain(APTR);
long IFFChunkSize(APTR);
void IFFClose(APTR);
void IFFFailure(APTR);
ULONG IFFGetFORM(APTR);
ULONG IFFNextChunk(APTR, ULONG);
APTR IFFOpen(char *, unsigned short, ULONG);
long IFFPopChunk(APTR);
long IFFPushChunk(APTR, ULONG);
long IFFReadChunkBytes(APTR, APTR, long);
long IFFWriteChunkBytes(APTR, APTR, long);
long IFFWriteChunk(APTR, APTR, ULONG, ULONG);

#endif
