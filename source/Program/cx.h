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

#ifndef _DOPUS_CX
#define _DOPUS_CX

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct _CxData
{
	struct NewBroker nb;

	CxObj *broker;

	CxObj *popup_filter;
	CxObj *rmb_hook;

	unsigned short right_down;
	unsigned short qual_down;
	struct Window *button_window;
	struct Window *popup_window;
	short start_popped;
} CxData;

#ifndef __amigaos3__
	#pragma pack()
#endif

enum {
	CXCODE_POPUP,
	CXCODE_NEW_LISTER,

	CXCODE_HOTKEY_BASE
};

void cx_install(CxData *);
void cx_remove(CxData *);
void cx_hotkey(long);
void cx_install_popup(CxData *);

BOOL cx_mouse_outside(struct Window *window, short, short);

#endif
