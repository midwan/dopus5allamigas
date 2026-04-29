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

#include "read.h"

const ModuleInfo module_info = {1, "read.module", "read.catalog", 0, 1, {{0, "Read", MSG_READ_DESCRIPTION, 0, 0}}};

const MenuData read_menus[] = {
	{NM_TITLE, 0, MSG_MENU_FILE, 0},
	{NM_ITEM, MENU_NEXT, MSG_MENU_NEXT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('N')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_SEARCH, MSG_MENU_SEARCH, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('S')},
	{NM_ITEM, MENU_REPEAT, MSG_MENU_REPEAT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('R')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_PRINT, MSG_MENU_PRINT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('P')},
	{NM_ITEM, MENU_SAVE_AS, MSG_MENU_SAVE_AS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('A')},
	{NM_ITEM, MENU_TO_EDITOR, MSG_MENU_TO_EDITOR, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('T')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_QUIT, MSG_MENU_QUIT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('Q')},

	{NM_TITLE, 0, MSG_MENU_SETTINGS, 0},
	{NM_ITEM, 0, MSG_MENU_TAB, 0},
	{NM_SUB, MENU_TAB_1, (IPTR) "1", MENUFLAG_COMM_SEQ | MENUFLAG_TEXT_STRING | CHECKIT | MENUFLAG_AUTO_MUTEX},
	{NM_SUB, MENU_TAB_2, (IPTR) "2", MENUFLAG_COMM_SEQ | MENUFLAG_TEXT_STRING | CHECKIT | MENUFLAG_AUTO_MUTEX},
	{NM_SUB, MENU_TAB_4, (IPTR) "4", MENUFLAG_COMM_SEQ | MENUFLAG_TEXT_STRING | CHECKIT | MENUFLAG_AUTO_MUTEX},
	{NM_SUB, MENU_TAB_8, (IPTR) "8", MENUFLAG_COMM_SEQ | MENUFLAG_TEXT_STRING | CHECKIT | MENUFLAG_AUTO_MUTEX},
	{NM_ITEM, 0, MSG_MENU_MODE, 0},
	{NM_SUB,
	 MENU_MODE_NORMAL,
	 MSG_MENU_MODE_NORMAL,
	 CHECKIT | MENUFLAG_AUTO_MUTEX | MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('O')},
	{NM_SUB,
	 MENU_MODE_ANSI,
	 MSG_MENU_MODE_ANSI,
	 CHECKIT | MENUFLAG_AUTO_MUTEX | MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('I')},
	{NM_SUB,
	 MENU_MODE_HEX,
	 MSG_MENU_MODE_HEX,
	 CHECKIT | MENUFLAG_AUTO_MUTEX | MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('H')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_USE_SCREEN, MSG_MENU_USE_SCREEN, CHECKIT | MENUTOGGLE | MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('U')},
	{NM_ITEM, MENU_SCREEN_MODE, MSG_MENU_SCREEN_MODE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('M')},
	{NM_ITEM, MENU_SELECT_FONT, MSG_MENU_SELECT_FONT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('F')},
	{NM_ITEM, 0, MSG_MENU_SELECT_EDITOR, 0},
	{NM_SUB, MENU_SELECT_EDITOR_NORMAL, MSG_MENU_MODE_NORMAL, 0},
	{NM_SUB, MENU_SELECT_EDITOR_ANSI, MSG_MENU_MODE_ANSI, 0},
	{NM_SUB, MENU_SELECT_EDITOR_HEX, MSG_MENU_MODE_HEX, 0},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_SAVE, MSG_MENU_SAVE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('V')},

	{NM_END}};

const ConfigWindow search_window = {{POS_CENTER, POS_CENTER, 48, 5}, {0, 0, 0, 48}};

const struct TagItem search_layout[] = {{GTCustom_LayoutRel, GAD_SEARCH_LAYOUT}, {TAG_DONE}},

					 search_text_tags[] = {{GTST_MaxChars, 80}, {TAG_MORE, (ULONG)search_layout}};

const ObjectDef search_objects[] = {

	// Search layout area
	{OD_AREA, 0, {0, 0, SIZE_MAXIMUM, 4}, {3, 3, -3, 33}, 0, AREAFLAG_RECESSED | AREAFLAG_ERASE, GAD_SEARCH_LAYOUT, 0},

	// Search string
	{OD_GADGET,
	 STRING_KIND,
	 {0, 1, SIZE_MAXIMUM, 1},
	 {4, 10, -4, 4},
	 MSG_ENTER_SEARCH_STRING,
	 PLACETEXT_ABOVE,
	 GAD_SEARCH_TEXT,
	 search_text_tags},

	// Case insensitive
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {0, 2, 0, 1},
	 {4, 17, 26, 4},
	 MSG_SEARCH_NO_CASE,
	 PLACETEXT_RIGHT,
	 GAD_SEARCH_CASE,
	 search_layout},

	// Wildcards
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {0, 3, 0, 1},
	 {4, 23, 26, 4},
	 MSG_SEARCH_WILD,
	 PLACETEXT_RIGHT,
	 GAD_SEARCH_WILD,
	 search_layout},

	// Whole words
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {POS_CENTER, 2, 0, 1},
	 {4, 17, 26, 4},
	 MSG_SEARCH_ONLYWORD,
	 PLACETEXT_RIGHT,
	 GAD_SEARCH_ONLYWORD,
	 search_layout},

	// Reverse search
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {POS_CENTER, 3, 0, 1},
	 {4, 23, 26, 4},
	 MSG_SEARCH_REVERSE,
	 PLACETEXT_RIGHT,
	 GAD_SEARCH_REVERSE,
	 search_layout},

	// Okay
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {3, -3, 4, 6},
	 MSG_OK_BUTTON,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_SEARCH_OKAY,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-3, -3, 4, 6},
	 MSG_CANCEL_BUTTON,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_SEARCH_CANCEL,
	 0},

	{OD_END}};

// ANSI palette
const ULONG ansi_palette[PEN_COUNT] = {0x009999,  /* Cyan */
									   0x000000,  /* Black */
									   0xffffff,  /* White */
									   0xaa0000,  /* Red */
									   0x00aa00,  /* Green */
									   0xaa00aa,  /* Purple */
									   0x0000aa,  /* Blue */
									   0xeebb00}; /* Yellow */
