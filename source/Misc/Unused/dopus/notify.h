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

#ifndef _DOPUS_NOTIFY
#define _DOPUS_NOTIFY

/*****************************************************************************

 Notification

 *****************************************************************************/

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Notification message
typedef struct
{
	struct Message dn_Msg;		   // Message header
	ULONG dn_Type;				   // Type of message
	ULONG dn_UserData;			   // User-supplied data
	ULONG dn_Data;				   // Message-specific data
	ULONG dn_Flags;				   // Flags
	struct FileInfoBlock *dn_Fib;  // FIB for some messages
	char dn_Name[1];			   // Name for some messages
} DOpusNotify;

// dn_Msg.mn_Node.ln_Type
#define NT_DOPUS_NOTIFY 199

// dn_Type
#define DN_WRITE_ICON (1 << 0)		  // Icon written
#define DN_APP_ICON_LIST (1 << 1)	  // AppIcon added/removed
#define DN_APP_MENU_LIST (1 << 2)	  // AppMenu added/removed
#define DN_CLOSE_WORKBENCH (1 << 3)	  // Workbench closed
#define DN_OPEN_WORKBENCH (1 << 4)	  // Workbench opened
#define DN_RESET_WORKBENCH (1 << 5)	  // Workbench reset
#define DN_DISKCHANGE (1 << 6)		  // Disk inserted/removed
#define DN_OPUS_QUIT (1 << 7)		  // Main program quit
#define DN_OPUS_HIDE (1 << 8)		  // Main program hide
#define DN_OPUS_SHOW (1 << 9)		  // Main program show
#define DN_OPUS_START (1 << 10)		  // Main program start
#define DN_DOS_ACTION (1 << 11)		  // DOS action
#define DN_REXX_UP (1 << 12)		  // REXX started
#define DN_FLUSH_MEM (1 << 13)		  // Flush memory
#define DN_APP_WINDOW_LIST (1 << 14)  // AppWindow added/removed

// Flags with DN_WRITE_ICON
#define DNF_ICON_REMOVED (1 << 0)	 // Icon removed
#define DNF_WINDOW_REMOVED (1 << 0)	 // AppWindow removed
#define DNF_ICON_CHANGED (1 << 1)	 // Image changed

// Flags with DN_DOS_ACTION
#define DNF_DOS_CREATEDIR (1 << 0)		// CreateDir
#define DNF_DOS_DELETEFILE (1 << 1)		// DeleteFile
#define DNF_DOS_SETFILEDATE (1 << 2)	// SetFileDate
#define DNF_DOS_SETCOMMENT (1 << 3)		// SetComment
#define DNF_DOS_SETPROTECTION (1 << 4)	// SetProtection
#define DNF_DOS_RENAME (1 << 5)			// Rename
#define DNF_DOS_CREATE (1 << 6)			// Open file (create)
#define DNF_DOS_CLOSE (1 << 7)			// Close file
#define DNF_DOS_RELABEL (1 << 8)		// Relabel disk

APTR AddNotifyRequest(ULONG, ULONG, struct MsgPort *);
void RemoveNotifyRequest(APTR);
void ReplyFreeMsg(struct Message *);
void SetNotifyRequest(APTR, ULONG, ULONG);

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
