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

#ifndef _DOPUS_BUTTONS
#define _DOPUS_BUTTONS

#define BUTREFRESH_SELECTOR (1 << 0)
#define BUTREFRESH_FONT (1 << 1)
#define BUTREFRESH_RESIZE (1 << 2)
#define BUTREFRESH_REFRESH (1 << 3)

#define DRAG_GADGET 1000

enum {
	BUTTONS_OPEN,	  // Open button display
	BUTTONS_CLOSE,	  // Close button display
	BUTTONS_BUSY,	  // Make button bank busy
	BUTTONS_SAVE,	  // Save bank
	BUTTONS_SAVEAS,	  // Save as
	BUTTONS_ICONIFY,  // Iconify
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Defines a button window
typedef struct ButtonWindow
{
	IPCData *ipc;  // IPC data

	struct Window *window;		  // Window pointer
	struct IBox pos;			  // Window dimensions
	struct AppWindow *appwindow;  // AppWindow pointer
	struct AppIcon *appicon;	  // AppIcon pointer
	struct List boopsi_list;	  // List of BOOPSI objects
	struct TextFont *font;		  // Button font

	struct Gadget *vert_scroll;	  // Vertical scroller
	struct Gadget *horiz_scroll;  // Horizontal scroller
	struct Gadget *down_gadget;	  // Gadget being held down

	ULONG flags;  // Flags

	struct MsgPort *app_port;  // Application message port

	Cfg_ButtonBank *bank;  // Bank pointer

	UWORD button_width;	  // Width of each button
	UWORD button_height;  // Height of each button
	UWORD button_cols;	  // Number of button columns
	UWORD button_rows;	  // Number of button rows

	int button_top;	  // Button top offset
	int button_left;  // Button left offset

	UWORD button_sel_code;			   // Mouse button code
	Cfg_Button *button_sel_button;	   // Button selected
	short button_sel_col;			   // Button column
	short button_sel_row;			   // Button row
	struct Rectangle button_sel_rect;  // Button rectangle
	char button_sel_flag;			   // Button currently selected?
	char pad;

	struct Requester busy_req;	// Busy requester

	char buttons_file[256];	 // File to open from

	IPCData *editor;		// Editor pointer
	short editor_sel_row;	// Selected row
	short editor_sel_col;	// Selected column
	short editor_sel_flag;	// Button or row/column?

	struct TimerHandle *timer;	// Timer

	struct timerequest pad_timer;  // Padding (get rid of it!)

	ULONG old_seconds;	// Seconds for double-click
	ULONG old_micros;	// Micros for double-click

	WindowID id;  // Window ID data

	DragInfo *drag_info;  // Drag info for dragging buttons
	short drag_x_offset;
	short drag_y_offset;
	Cfg_Button *button_drag_button;	 // Button being dragged
	short drag_col;
	short drag_row;

	struct Gadget drag_gadget;	// Gadget for dragging

	Cfg_Button *button_last_click;

	APTR memory;					  // Memory handle
	PopUpMenu button_menu;			  // Popup menu
	struct DiskObject *appicon_icon;  // Iconify icon

	unsigned long tick_count;
	unsigned long last_tick;

	struct IBox last_position;	// Saved position and size

	unsigned long drag_click_secs;	// Click time on drag gadget
	unsigned long drag_click_micros;

	short click_x, click_y;	 // Position clicked at
	short button_sel_which;	 // Which function is selected?

	struct DrawInfo *drawinfo;	// Draw info

	char last_saved[256];	 // Last saved file
	APTR filereq;			 // File requester
	Cfg_ButtonBank *backup;	 // Backup bank

	ULONG border_type;	// Type of border

	struct IBox internal;  // Internal position

	ImageRemap remap;  // Remap information

	long icon_pos_x;  // Position of icon
	long icon_pos_y;

	struct _PopUpHandle *menu;	// PopUp menu

	PatternInstance pattern;  // Pattern
	PatternData pattern_data;

	ULONG old_flags;
} Buttons;

typedef struct
{
	Cfg_ButtonBank *bank;
	IPCData *ipc;
	long flag;
	unsigned long button;
} buttons_startup;

typedef struct
{
	Buttons *buttons;
	short col, row;
	Cfg_Button *edit;
	short can_start;
	struct AppMessage *appmsg;
} buttons_edit_packet;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define BUTTONF_RESIZED (1 << 0)		  // Window has been resized
#define BUTTONF_BUSY (1 << 1)			  // Window is busy
#define BUTTONF_CHANGED (1 << 2)		  // Buttons have been modified
#define BUTTONF_LOAD (1 << 3)			  // Need to load button file
#define BUTTONF_FAIL (1 << 4)			  // Fail if can't load file
#define BUTTONF_COORDS_SUPPLIED (1 << 5)  // Coordinates supplied
#define BUTTONF_HIGH_SHOWN (1 << 6)		  // Highlight is shown
#define BUTTONF_NO_FLASH (1 << 8)		  // Flash is disabled
#define BUTTONF_SKIP_SCROLL (1 << 9)	  // Skip a repeat scroll
#define BUTTONF_EDIT_REQUEST (1 << 10)	  // Pending edit request
#define BUTTONF_ICONIFIED (1 << 11)		  // Buttons iconified
#define BUTTONF_HAVE_RESIZED (1 << 12)	  // Have resized
#define BUTTONF_NO_ALTIMAGE (1 << 13)	  // No Alternate image
#define BUTTONF_FIRST_TIME (1 << 14)	  // First time we've opened
#define BUTTONF_BUTTON_HELD (1 << 15)	  // Button held down
#define BUTTONF_TOOLBAR (1 << 16)		  // This bank is a toolbar
#define BUTTONF_BORDERLESS (1 << 17)	  // No window borders
#define BUTTONF_NEW_BANK (1 << 18)		  // New bank
#define BUTTONF_UNDERMOUSE (1 << 19)	  // Open under mouse

extern MenuData button_toolbar_menu[];

// protos
Buttons *buttons_new(char *, Cfg_ButtonBank *, struct IBox *, short, ULONG);
ULONG buttons_command(Buttons *, ULONG, ULONG, APTR, APTR, struct MsgPort *);
void buttons_update(Buttons *buttons);
void buttons_get_font(Buttons *buttons);
BOOL buttons_iconify(Buttons *buttons);

IPC_EntryProto(buttons_code, extern);
void buttons_cleanup(Buttons *, BOOL);
BOOL buttons_open(Buttons *, struct Screen *, short);
void buttons_close(Buttons *, short);
void buttons_refresh(Buttons *, ULONG);
int buttons_process_msg(Buttons *, struct IntuiMessage *);
void buttons_busy(Buttons *);
void buttons_unbusy(Buttons *);

short buttons_draw_border(Buttons *, Cfg_Button *, short, short, short, short, struct RastPort *);
void buttons_get_rect(Buttons *buttons, int x, int y, struct Rectangle *rect);
void buttons_redraw_button(Buttons *buttons, Cfg_Button *button);
short buttons_draw_button(Buttons *, Cfg_Button *, short, short, short, short, struct RastPort *);
Cfg_Button *button_from_point(Buttons *, WORD *, WORD *);
Cfg_Button *button_from_pos(Buttons *buttons, short col, short row);
BOOL buttons_pos(Buttons *buttons, Cfg_Button *button, short *col, short *row);
Cfg_Function *button_valid(Cfg_Button *button, short which);
void buttons_show_highlight(Buttons *buttons);
void buttons_stop_highlight(Buttons *buttons);

Cfg_ButtonFunction *button_find_function(Cfg_Button *button, short which, APTR *);
void buttons_run_button(Buttons *, Cfg_Button *, short);
BOOL buttons_app_message(Buttons *, struct _DOpusAppMessage *msg);

int buttons_save(Buttons *buttons, char *name);
void buttons_saveas(Buttons *buttons);
int buttons_load(Buttons *buttons, struct Screen *screen, char *);
int buttons_check_change(Buttons *buttons, BOOL);

void buttons_edit_key(Buttons *buttons, UWORD code, UWORD qual);
BOOL buttons_visible_select(Buttons *buttons);
void buttons_start_drag(Buttons *, short, short, short, short);
void buttons_show_drag(Buttons *, short, short);
void buttons_stop_drag(Buttons *, short, short);
void buttons_edit(IPCData *my_ipc, buttons_edit_packet *);
BOOL buttons_edit_bank(Buttons *buttons, short col, short row, Cfg_Button *, struct AppMessage *, short);

short buttons_get_max_size(Buttons *, unsigned short *, unsigned short *);
void buttons_highlight_button(Buttons *buttons, short state, short);
void buttons_show_button(Buttons *, Cfg_Button *, short, short, short, struct RastPort *);

void buttons_do_popup(Buttons *, UWORD);
BOOL buttons_button_popup(Buttons *, UWORD);

int buttons_do_function(Buttons *buttons, IPTR func);
void buttons_new_bank(Buttons *buttons, short func, Cfg_ButtonBank *);

int buttons_request_file(Buttons *, char *, char *, char *, ULONG);

REF_CALLBACK_PROTO(VOID,
				   ASM SAVEDS buttons_refresh_callback,
				   d0,
				   ULONG,
				   type,
				   a0,
				   struct Window *,
				   window,
				   a1,
				   Buttons *,
				   buttons);

void buttons_edit_save(ULONG id);
void buttons_edit_defaults(ULONG id);

BOOL buttons_fix_drag(Buttons *buttons);
void buttons_fix_internal(Buttons *buttons);

void buttons_remap(Buttons *buttons, short remap);

#define BUTCLOSEF_SCRIPT (1 << 0)
#define BUTCLOSEF_NO_REMAP (1 << 1)

#define BUTOPENF_REMAP (1 << 0)

void buttons_update_icon(Buttons *buttons);

UWORD button_border_popup(Buttons *buttons);

#define DRAG_WIDTH 8
#define DRAG_HI_HEIGHT 8
#define DRAG_LO_HEIGHT 4

#endif
