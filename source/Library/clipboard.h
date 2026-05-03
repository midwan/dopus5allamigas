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
#ifndef _CLIPBOARD_H
#define _CLIPBOARD_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct ClipData
{
	char *string;
	long length;
	long result;
} ClipData;

#define CLIP_PUTSTRING 2000
#define CLIP_GETSTRING 2001

typedef struct ClipHandle
{
	struct MsgPort *clip_Port;
	struct IOClipReq *clip_Req;
	ULONG clip_Header[5];
	char clip_Buffer[128];
	BOOL clip_Write;
} ClipHandle;

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
