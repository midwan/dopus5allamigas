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
#ifndef _IFF_H
#define _IFF_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct _IFFHandle
{
	APTR iff_File;	  // File handle
	short iff_Mode;	  // Reading or writing
	short iff_Error;  // Any error
	APTR iff_Memory;  // Any memory used for the handle

	ULONG iff_Form;	 // IFF Form
	LONG iff_Size;	 // Total size

	ULONG iff_Chunk;	 // Current chunk
	LONG iff_ChunkSize;	 // Current chunk size
	LONG iff_ChunkPos;	 // Position of chunk in file
	LONG iff_ChunkOK;	 // Current chunk is OK?

	struct MinList iff_ChunkData;  // Current chunk data

	struct MsgPort *iff_ClipPort;	// Clipboard port
	struct IOClipReq *iff_ClipReq;	// Clipboard request

	BPTR iff_SafeFile;		// Safe file
	char iff_TempName[20];	// Temporary name
	LONG iff_Success;		// Indicates success
	BOOL iff_Async;			// Using AsyncIO

	char iff_Name[1];  // Filename
} IFFHandle;

typedef struct
{
	struct MinNode chk_Node;
	LONG chk_Size;
	char chk_Data[1];
} IFFChunkData;

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
