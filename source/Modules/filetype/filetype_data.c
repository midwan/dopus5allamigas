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

#include "filetype.h"

/* This structure is used to identify the module */

ModuleInfo_2 module_info = {0,					/* Version */
							"filetype.module",	/* Module name */
							"filetype.catalog", /* Catalog name */
							0,					/* Flags (0 for now) */
							2,					/* Number of functions in module */
							{{0, "FindFileType", MSG_FIND_DESC, FUNCF_NEED_FILES, 0},
							 {1, "CreateFileType", MSG_CREATE_DESC, FUNCF_NEED_ENTRIES, 0}}};

#define GAP (2)
#define GAP2 (4)
#define BUTTONX (12)
#define BUTTONY (6)
#define CHECKX (26)
#define CHECKY (4)
#define STRINGY (4)

#define FLISTVIEW_W1 (29)
#define FLISTVIEW_W2 (BUTTONX * 3)

#define FRHS_W1 (31)
#define FRHS_W2 (BUTTONX * 3)

#define BUTW (12)
#define BUTSPACE (2)
#define BUTGAP1 (BUTSPACE + BUTW)

#define FWINW1 (BUTW * 5 + BUTSPACE * 4)
#define FWINW2 (BUTTONX * 5 + GAP * 2)
#define FWINH1 (12)
#define FWINH2 (GAP + 1 + GAP2 + GAP2 + 1 + GAP + BUTTONY + GAP)

// Tags for gadgets
struct TagItem

	finder_layout[] = {{GTCustom_LayoutRel, GAD_FIND_LAYOUT}, {TAG_END}},

	finder_listview_tags[] = {{DLV_ShowSelected, 0},
							  {DLV_ShowChecks, 2},
							  {DLV_ReadOnly, 1},
							  {TAG_MORE, (IPTR)finder_layout}};

ConfigWindow _finder_window = {{POS_CENTER, POS_CENTER, FWINW1, FWINH1}, {0, 0, FWINW2, FWINH2}};

ObjectDef _finder_objects[] = {

	// Information area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {GAP, GAP, -GAP, -GAP - BUTTONY - GAP},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_FIND_LAYOUT,
	 0},

	// Pri
	{OD_TEXT, TEXTPEN, {0, 0, 0, 1}, {GAP2 + 25, GAP2, 0, 0}, MSG_FIND_PRI, 0, 0, finder_layout},

	// ID
	{OD_TEXT, TEXTPEN, {5, 0, 0, 1}, {GAP2 + 25, GAP2, 0, 0}, MSG_FIND_ID, 0, 0, finder_layout},

	// Filetype
	{OD_TEXT, TEXTPEN, {15, 0, 0, 1}, {GAP2 + 25, GAP2, 0, 0}, MSG_FIND_FILETYPE, 0, 0, finder_layout},

	// Listview
	{OD_GADGET,
	 MY_LISTVIEW_KIND,
	 {0, 1, FLISTVIEW_W1, SIZE_MAXIMUM},
	 {GAP2, GAP2, FLISTVIEW_W2, -GAP2},
	 0,
	 PLACETEXT_ABOVE,
	 GAD_FIND_LISTVIEW,
	 finder_listview_tags},

	// Right-hand side stuff
	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 0, SIZE_MAXIMUM, 1},
	 {GAP2 + FLISTVIEW_W2 + GAP2, GAP2, 0, 0},
	 MSG_FIND_FILE,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 0,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 1, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT1,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 3, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT2,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 4, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT3,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 5, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT4,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 6, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT5,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 7, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT6,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 8, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT7,
	 finder_layout},

	{OD_AREA,
	 TEXTPEN,
	 {FLISTVIEW_W1, 9, SIZE_MAXIMUM, 1},
	 {FLISTVIEW_W2 + 2 * GAP2, GAP2 + 2, 0, 0},
	 0,
	 AREAFLAG_ERASE | TEXTFLAG_CENTER,
	 GAD_FIND_TEXT8,
	 finder_layout},

	// Use button
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, BUTW, 1},
	 {GAP, -GAP, BUTTONX, BUTTONY},
	 MSG_FIND_USE,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_FIND_USE,
	 0},

	// Install button
	{OD_GADGET,
	 BUTTON_KIND,
	 {BUTGAP1, POS_RIGHT_JUSTIFY, BUTW, 1},
	 {GAP + BUTTONX, -GAP, BUTTONX, BUTTONY},
	 MSG_FIND_INSTALL,
	 0,
	 GAD_FIND_INSTALL,
	 0},

	// Create button
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_CENTER, POS_RIGHT_JUSTIFY, BUTW, 1},
	 {0, -GAP, BUTTONX, BUTTONY},
	 MSG_FIND_CREATE,
	 0,
	 GAD_FIND_CREATE,
	 0},

	// Edit button
	{OD_GADGET,
	 BUTTON_KIND,
	 {BUTGAP1 * 3, POS_RIGHT_JUSTIFY, BUTW, 1},
	 {GAP + BUTTONX * 3, -GAP, BUTTONX, BUTTONY},
	 MSG_FIND_EDIT,
	 0,
	 GAD_FIND_EDIT,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, BUTW, 1},
	 {-GAP, -GAP, BUTTONX, BUTTONY},
	 MSG_FIND_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_FIND_CANCEL,
	 0},

	// End
	{OD_END}};

#define LISTVIEW_W1 (18)
#define LISTVIEW_W2 (BUTTONX * 3)

// Labels
UWORD
creator_cycle_labels[] = {MSG_CREATE_CASE, MSG_CREATE_NOCASE, 0};

// Tags for gadgets
struct TagItem

	creator_layout[] = {{GTCustom_LayoutRel, GAD_CREATE_LAYOUT}, {TAG_END}},

	creator_listview_tags[] = {{DLV_ShowSelected, 0}, {TAG_MORE, (IPTR)creator_layout}},

	creator_cycle_tags[] = {{GTCustom_LocaleLabels, (IPTR)creator_cycle_labels}, {TAG_MORE, (IPTR)creator_layout}},

	creator_filetype_name_tags[] = {{GTST_MaxChars, FILETYPE_MAXLEN},
									{GTST_String, (IPTR) "Untitled"},
									{TAG_MORE, (IPTR)creator_layout}};

ConfigWindow _creator_window = {{POS_CENTER, POS_CENTER, LISTVIEW_W1 * 3, 8},
								{0,
								 0,
								 GAP * 2 + 2 * 2 + LISTVIEW_W2 * 2 + GAP2 * 3,
								 GAP + 1 + GAP2 + STRINGY + 6 * CHECKY + 5 * GAP + GAP2 + 1 + GAP + BUTTONY + GAP}};

ObjectDef _creator_objects[] = {

	// Information area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {GAP, GAP, -GAP, -GAP - BUTTONY - GAP},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_CREATE_LAYOUT,
	 0},

	// Listview
	{OD_GADGET,
	 MY_LISTVIEW_KIND,
	 {0, 1, LISTVIEW_W1, 5},
	 {GAP2, GAP2 + STRINGY, LISTVIEW_W2, 5 * CHECKY + 4 * GAP},
	 MSG_CREATE_FILES,
	 PLACETEXT_ABOVE,
	 GAD_CREATE_LISTVIEW,
	 creator_listview_tags},

	{OD_GADGET,
	 BUTTON_KIND,
	 {0, 6, 6, 1},
	 {GAP2, GAP2 + STRINGY + 5 * CHECKY + 4 * GAP, BUTTONX, BUTTONY},
	 MSG_CREATE_ADD,
	 0,
	 GAD_CREATE_ADD,
	 creator_layout},

	{OD_GADGET,
	 BUTTON_KIND,
	 {6, 6, 6, 1},
	 {GAP2 + BUTTONX, GAP2 + STRINGY + 5 * CHECKY + 4 * GAP, BUTTONX, BUTTONY},
	 MSG_CREATE_DELETE,
	 0,
	 GAD_CREATE_DELETE,
	 creator_layout},

	{OD_GADGET,
	 BUTTON_KIND,
	 {12, 6, 6, 1},
	 {GAP2 + BUTTONX * 2, GAP2 + STRINGY + 5 * CHECKY + 4 * GAP, BUTTONX, BUTTONY},
	 MSG_CREATE_CLEAR,
	 0,
	 GAD_CREATE_CLEAR,
	 creator_layout},

	// Right-hand side stuff
	{OD_TEXT,
	 TEXTPEN,
	 {LISTVIEW_W1, 0, 0, 1},
	 {GAP2 + LISTVIEW_W2 + GAP2, GAP2 + 2, 0, 0},
	 MSG_CREATE_FILETYPE,
	 0,
	 0,
	 creator_layout},

	// Filetype name
	{OD_GADGET,
	 STRING_KIND,
	 {LISTVIEW_W1 + 12, 0, SIZE_MAXIMUM, 1},
	 {GAP2 + LISTVIEW_W2 + GAP2, GAP2, -GAP2, STRINGY},
	 0,
	 0,
	 GAD_CREATE_FILETYPE,
	 creator_filetype_name_tags},

	// Filename
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {LISTVIEW_W1 + 0, 1, 0, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY, CHECKX, CHECKY},
	 MSG_CREATE_NAME,
	 PLACETEXT_RIGHT,
	 GAD_CREATE_NAME,
	 creator_layout},

	{OD_AREA,
	 TEXTPEN,
	 {LISTVIEW_W1 + 12, 1, SIZE_MAXIMUM, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + 2, -GAP2, 0},
	 0,
	 AREAFLAG_ERASE,
	 GAD_CREATE_NAME_FIELD,
	 creator_layout},

	// Filetype IFF FORM type
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {LISTVIEW_W1 + 0, 2, 0, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY + GAP, CHECKX, CHECKY},
	 MSG_CREATE_IFF,
	 PLACETEXT_RIGHT,
	 GAD_CREATE_IFF,
	 creator_layout},

	{OD_AREA,
	 TEXTPEN,
	 {LISTVIEW_W1 + 12, 2, SIZE_MAXIMUM, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY + GAP + 2, -GAP2, 0},
	 0,
	 AREAFLAG_ERASE,
	 GAD_CREATE_IFF_FIELD,
	 creator_layout},

	// Filetype datatypes group
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {LISTVIEW_W1 + 0, 3, 0, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY * 2 + GAP * 2, CHECKX, CHECKY},
	 MSG_CREATE_GROUP,
	 PLACETEXT_RIGHT,
	 GAD_CREATE_GROUP,
	 creator_layout},

	{OD_AREA,
	 TEXTPEN,
	 {LISTVIEW_W1 + 12, 3, SIZE_MAXIMUM, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY * 2 + GAP * 2 + 2, -GAP2, 0},
	 0,
	 AREAFLAG_ERASE,
	 GAD_CREATE_GROUP_FIELD,
	 creator_layout},

	// Filetype datatypes ID
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {LISTVIEW_W1 + 0, 4, 0, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY * 3 + GAP * 3, CHECKX, CHECKY},
	 MSG_CREATE_ID,
	 PLACETEXT_RIGHT,
	 GAD_CREATE_ID,
	 creator_layout},

	{OD_AREA,
	 TEXTPEN,
	 {LISTVIEW_W1 + 12, 4, SIZE_MAXIMUM, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY * 3 + GAP * 3 + 2, -GAP2, 0},
	 0,
	 AREAFLAG_ERASE,
	 GAD_CREATE_ID_FIELD,
	 creator_layout},

	// Filetypes bytes
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {LISTVIEW_W1 + 0, 5, 0, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY * 4 + GAP * 4, CHECKX, CHECKY},
	 MSG_CREATE_BYTES,
	 PLACETEXT_RIGHT,
	 GAD_CREATE_BYTES,
	 creator_layout},

	// Case-sensitivity cycle gadget
	{OD_GADGET,
	 CYCLE_KIND,
	 //			{POS_RIGHT_JUSTIFY,5,10,1},
	 {LISTVIEW_W1 + 12, 5, 10, 1},
	 //			{-GAP2,GAP2+STRINGY+CHECKY*4+GAP*4,CHECKX,CHECKY},
	 {GAP2 + LISTVIEW_W2 + GAP2, GAP2 + STRINGY + CHECKY * 4 + GAP * 4, CHECKX, CHECKY},
	 0,
	 0,
	 GAD_CREATE_CYCLE,
	 creator_cycle_tags},

	{OD_AREA,
	 TEXTPEN,
	 {LISTVIEW_W1, 6, SIZE_MAXIMUM, 1},
	 {LISTVIEW_W2 + 2 * GAP2, GAP2 + STRINGY + CHECKY * 5 + GAP * 5 + 2, -GAP2, 0},
	 0,
	 AREAFLAG_ERASE,
	 GAD_CREATE_BYTES_FIELD,
	 creator_layout},

	// Edit button
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {GAP, -GAP, BUTTONX, BUTTONY},
	 MSG_CREATE_EDIT,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_CREATE_EDIT,
	 0},

	// Save button
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_CENTER, POS_RIGHT_JUSTIFY, 12, 1},
	 {0, -GAP, BUTTONX, BUTTONY},
	 MSG_CREATE_SAVE,
	 0,
	 GAD_CREATE_SAVE,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-GAP, -GAP, BUTTONX, BUTTONY},
	 MSG_CREATE_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_CREATE_CANCEL,
	 0},

	// End
	{OD_END}};
