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
#ifndef _PROGRESS_H
#define _PROGRESS_H

enum {
	PROG_NAME,
	PROG_SIZE,
	PROG_INFO,
	PROG_INFO2,
	PROG_INFO3,
	PROG_GRAPH,
	PROG_LAST,
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct _ProgressWindow
{
	struct Window *pw_Window;  // Window pointer
	IPCData *pw_IPC;		   // IPC pointer

	struct Window *pw_OwnerWindow;	// Owner window
	struct Screen *pw_Screen;		// Screen we're on

	char pw_Title[40];	// Window title

	unsigned long pw_Flags;	 // Flags

	char pw_FileName[80];  // Current filename
	long pw_FileSize;	   // Current filesize
	long pw_FileDone;	   // Amount of file done

	long pw_FileCount;	// Total number of files
	long pw_FileNum;	// Current file number

	char pw_Information[80];  // Current information line

	struct Task *pw_SigTask;  // Task to signal for abort
	long pw_SigBit;			  // Signal bit to use

	struct MyLibrary *pw_Lib;  // Library pointer

	Point pw_Offset;			   // Coordinate offset
	struct Gadget *pw_Abort;	   // Abort gadget
	struct DrawInfo *pw_DrawInfo;  // DrawInfo
	char *pw_SizeString;		   // Cache for size string

	Point pw_WindowPos;	 // Window position
	BOOL pw_PosValid;	 // Position is valid?

	short pw_ProgWidth;	 // Width of progress bar filled

	WindowID pw_ID;			   // Window ID
	struct Hook *pw_Backfill;  // Backfill hook

	char pw_TaskName[40];  // Task name

	char pw_Information2[80];  // Current information line
	char pw_Information3[80];  // Current information line

	struct IBox pw_Coords[PROG_LAST + 1];  // Coordinates
	BOOL pw_Scale;						   // Scale 64bit size?
} ProgressWindow;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define PWF_ALL (PWF_FILENAME | PWF_FILESIZE | PWF_INFO | PWF_GRAPH)

#define PWF_ABORTED (1 << 16)

#define PROGRESS_SET 1000
#define PROGRESS_GET 1001

#endif
