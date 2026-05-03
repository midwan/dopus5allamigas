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

#ifndef _DOPUS_APPICON
#define _DOPUS_APPICON

/*****************************************************************************

 AppIcon, etc

 *****************************************************************************/

#ifndef WORKBENCH_WORKBENCH_H
	#include <workbench/workbench.h>
#endif

#ifndef __amigaos3__
	#pragma pack(2)
#endif

#define DAE_Local TAG_USER + 0			// Add to DOpus only, not WB
#define DAE_SnapShot TAG_USER + 1		// Supports snapshot
#define DAE_Menu TAG_USER + 2			// Menu item
#define DAE_Close TAG_USER + 3			// Close item
#define DAE_Background TAG_USER + 4		// Background colour
#define DAE_ToggleMenu TAG_USER + 5		// Toggle item
#define DAE_ToggleMenuSel TAG_USER + 6	// Toggle item (selected)
#define DAE_Info TAG_USER + 7			// Supports Information
#define DAE_Locked TAG_USER + 8			// Position locked
#define DAE_MenuBase TAG_USER + 9		// Menu ID base

APTR FindAppWindow(struct Window *);
struct MsgPort *AppWindowData(APTR, ULONG *, ULONG *);
struct Window *AppWindowWindow(APTR);

// Messages sent from AppIcons

#define MTYPE_APPSNAPSHOT 0x3812  // Snapshot message

struct AppSnapshotMsg
{
	struct AppMessage ap_msg;  // Message
	long position_x;		   // Icon x-position
	long position_y;		   // Icon y-position
	struct IBox window_pos;	   // Window position
	unsigned long flags;	   // Flags
	long id;				   // ID
};

#define APPSNAPF_UNSNAPSHOT (1 << 0)  // Set "no position"
#define APPSNAPF_WINDOW_POS (1 << 1)  // Window position supplied
#define APPSNAPF_MENU (1 << 2)		  // Menu operation
#define APPSNAPF_CLOSE (1 << 3)		  // Close command
#define APPSNAPF_HELP (1 << 4)		  // Help on a command
#define APPSNAPF_INFO (1 << 5)		  // Information command

// Change AppIcons

void ChangeAppIcon(APTR, struct Image *, struct Image *, char *, ULONG);
long SetAppIconMenuState(APTR, long, long);

#define CAIF_RENDER (1 << 0)
#define CAIF_SELECT (1 << 1)
#define CAIF_TITLE (1 << 2)
#define CAIF_LOCKED (1 << 3)
#define CAIF_SET (1 << 4)
#define CAIF_BUSY (1 << 5)
#define CAIF_UNBUSY (1 << 6)

// AppWindow messages

typedef struct _DOpusAppMessage
{
	struct AppMessage da_Msg;  // Message
	Point *da_DropPos;		   // Drop array
	Point da_DragOffset;	   // Mouse pointer offset
	ULONG da_Flags;			   // Flags
	ULONG da_Pad[2];
} DOpusAppMessage;

#define DAPPF_ICON_DROP (1 << 16)  // Dropped with icon

DOpusAppMessage *AllocAppMessage(APTR, struct MsgPort *, short);
BOOL CheckAppMessage(DOpusAppMessage *);
void FreeAppMessage(DOpusAppMessage *);
BOOL GetWBArgPath(struct WBArg *, char *, long);
void ReplyAppMessage(DOpusAppMessage *);
BOOL SetWBArg(DOpusAppMessage *, short, BPTR, char *, APTR);

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
