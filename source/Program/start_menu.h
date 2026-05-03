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

#define STARTMENUF_POPPED (1 << 0)

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct _StartMenu
{
	CFG_STRT data;
	char buttons[256];
	short x;
	short y;

	IPCData *ipc;
	struct Window *window;
	APTR imptr;
	Cfg_ButtonBank *bank;
	struct SignalSemaphore lock;

	struct Gadget drag;

	WindowID id;
	struct DrawInfo *drawinfo;

	struct Rectangle button;
	ImageRemap remap;

	PopUpHandle *menu;
	APTR memory;

	short im_width;
	short im_height;
	short label_width;
	short no_alt;

	short changed;

	struct AppWindow *app_window;
	struct MsgPort *app_port;

	struct TextFont *font;
	struct TextFont *label_font;

	ULONG flags;

	PatternInstance pattern;
	PatternData pattern_data;

	short last_id;
} StartMenu;

#ifndef __amigaos3__
	#pragma pack()
#endif

IPCData *start_new(char *, char *, char *, short, short);
void start_cleanup(StartMenu *);
void start_show(StartMenu *, struct Screen *);
void start_hide(StartMenu *);
BOOL start_popup(StartMenu *);
void start_draw_button(StartMenu *, short, short);
void start_toggle_drag(StartMenu *, short);
void start_toggle_border(StartMenu *);
void start_fix_button(StartMenu *);
BOOL start_edit(StartMenu *);
BOOL start_save(StartMenu *);
void start_change_image(StartMenu *);
void start_change_label(StartMenu *);
void start_create_new(BOOL);
void start_add_item(StartMenu *, char *);
void start_remap_bank(StartMenu *, struct List *, BOOL);
void start_change_font(StartMenu *, short);
void start_change_picture(StartMenu *);
