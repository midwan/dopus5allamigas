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
#ifndef _POPUP_H
#define _POPUP_H

#define POPUPMF_GET_PEN(x) (((x) >> 24) & 0xff)

#define ITEM_V_PAD 6
#define ITEM_H_PAD 24
#define SCROLL_V_SIZE 4

#define SCROLLIMAGE_HEIGHT 3
#define SCROLLIMAGE_WIDTH 6

#define SMALL_HI_BULLET_WIDTH 4
#define SMALL_HI_BULLET_HEIGHT 4
#define SMALL_LO_BULLET_WIDTH 5
#define SMALL_LO_BULLET_HEIGHT 4

#define SMALL_HI_SUB_WIDTH 3
#define SMALL_HI_SUB_HEIGHT 5
#define SMALL_LO_SUB_WIDTH 6
#define SMALL_LO_SUB_HEIGHT 5

#define BIG_HI_BULLET_WIDTH 5
#define BIG_HI_BULLET_HEIGHT 5
#define BIG_LO_BULLET_WIDTH 8
#define BIG_LO_BULLET_HEIGHT 5

#define BIG_HI_SUB_WIDTH 5
#define BIG_HI_SUB_HEIGHT 10
#define BIG_LO_SUB_WIDTH 8
#define BIG_LO_SUB_HEIGHT 7

#define MENU_DEPTH 3

enum {
	BULLET_SMALL_LO,
	BULLET_BIG_LO,
	BULLET_SMALL_HI,
	BULLET_BIG_HI,
	SUB_SMALL_LO,
	SUB_BIG_LO,
	SUB_SMALL_HI,
	SUB_BIG_HI,
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	struct MinList *item_list;	// Item list
	struct Window *window;		// Our menu window
	struct IBox pos;			// Menu position and size

	short item_x;		 // Item x-offset
	short item_y;		 // Item y-offset
	short item_width;	 // Width of each item
	short item_height;	 // Height of each item
	short total_height;	 // Total visible height

	short top_item;		// Top item displayed in the list
	short bottom_item;	// Bottom item in the list
	short item_count;	// Number of items in the list
	short sep_count;	// Number of separators
	short scroll_dir;	// Current scroll direction
	short bottom_y;		// Bottom of last item

	PopUpItem *sel_item;  // Currently selected item
	short sel_item_y;	  // Y position

	short scroll;	 // Is list currently scrolling?
	short o_x, o_y;	 // original position

	WindowID id;  // WindowID data

	struct BitMap *right_bitmap;  // Shadow bitmaps
	struct BitMap *bottom_bitmap;

	short h_pad_left;
	short h_pad_right;
	short h_pad_left_extra;
} PopUpList;

typedef struct
{
	PopUpMenu *menu;			   // Pointer to menu data
	struct TextFont *font;		   // Font we use
	struct Window *parent_window;  // Window we're attached to
	struct MsgPort *port;		   // Our IDCMP port
	struct MsgPort *old_port;	   // Old window IDCMP port
	struct DrawInfo *drawinfo;	   // DrawInfo cache
	ULONG old_idcmp;			   // Old window IDCMP flags
	ULONG old_flags;			   // Old window flags
	short fpen;					   // Foreground pen
	short bpen;					   // Background pen
	short fpen_sel;				   // Foreground selected pen
	short bpen_sel;				   // Background selected pen
	short sep_height;			   // Height of separators
	short item_h_pad;			   // Horizontal padding
	BOOL was_active;			   // Was window active?
	TimerHandle *timer;			   // A timer

	UWORD *bullet_data;	 // Bullet image
	short bullet_width;
	short bullet_height;

	UWORD *sub_data;  // Submenu image
	short sub_width;
	short sub_height;

	UWORD *scroll_image[2];

	PopUpList menu_list[MENU_DEPTH];  // Menu info
	short menu_current;				  // Current active one
	short tick_count;				  // Number of ticks on current selection

	ULONG wait_bits;
	BOOL locked;  // Locked open
} PopUpData;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define TICK_TIME 20000
#define STILL_TICKS 10

void popup_cleanup(PopUpData *);
void popup_init_display(PopUpData *, short);
void popup_display(PopUpData *, short);
short popup_show_item(PopUpData *, short, PopUpItem *, short, BOOL, BOOL);
BOOL popup_select(PopUpData *, short, short, short);
BOOL popup_init_list(PopUpData *data, short which);
void popup_close_display(PopUpList *);

#endif
