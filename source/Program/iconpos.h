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

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	struct Node ip_Node;
	unsigned long ip_Flags;
	struct IBox ip_Area;
	struct Window *ip_Window;
	PopUpHandle *ip_Menu;
	PopUpItem *ip_Item[10];
	char ip_Buf[5][4];
} iconpos_data;

typedef struct
{
	WindowID ip_ID;
	iconpos_data ip_Data;
} iconpos_info;

#ifndef __amigaos3__
	#pragma pack()
#endif

void iconpos_openwindow(iconpos_info *, struct MsgPort *);
void iconpos_closewindow(iconpos_info *);

enum

{ IPITEM_APPICON,
  IPITEM_DISKS,
  IPITEM_LISTERS,
  IPITEM_GROUPS,
  IPITEM_LEFTOUT,
  IPITEM_PRIORITY };
