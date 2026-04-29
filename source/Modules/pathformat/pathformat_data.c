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

#include "pathformat.h"

ModuleInfo module_info = {1, "pathformat.module", "pathformat.catalog", 0, 0, {{0}}};

// Display window dimensions
ConfigWindow pathformat_window = {{POS_CENTER, POS_CENTER, 40, 12}, {0, 0, 56, 62}};

UWORD
mode_labels[] = {MSG_MODE_NONE, MSG_MODE_NAME, MSG_MODE_ICON, MSG_MODE_ICON_ACTION, 0};

// Tags for gadgets
struct TagItem

	// Relative to area tags[]={
	path_relative_taglist[] = {{GTCustom_LayoutRel, GAD_PATHFORMAT_LAYOUT}, {TAG_END, 0}},

	// Path lister
	path_lister_taglist[] = {{DLV_ShowSelected, 0}, {TAG_MORE, (IPTR)path_relative_taglist}},

	// Path folder
	path_folder_taglist[] = {{GTCustom_Control, GAD_PATHFORMAT_PATH}, {TAG_MORE, (IPTR)path_relative_taglist}},

	// Key
	path_key_taglist[] = {{GTST_MaxChars, 80}, {TAG_MORE, (IPTR)path_relative_taglist}},

	// Path
	path_path_taglist[] = {{GTST_MaxChars, 256}, {TAG_MORE, (IPTR)path_relative_taglist}},

	// Mode
	path_mode_taglist[] = {{GTCustom_LocaleLabels, (IPTR)mode_labels}, {TAG_MORE, (IPTR)path_relative_taglist}};

// Path window gadgets
ObjectDef pathformat_objects[] = {

	// Edit area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {3, 3, -3, -12},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_PATHFORMAT_LAYOUT,
	 0},

	// Path list
	{OD_GADGET,
	 MY_LISTVIEW_KIND,
	 {0, 0, SIZE_MAXIMUM, 7},
	 {4, 4, -4, 4},
	 0,
	 LISTVIEWFLAG_CURSOR_KEYS,
	 GAD_PATHFORMAT_PATHS,
	 path_lister_taglist},

	// Path folder button
	{OD_GADGET,
	 DIR_BUTTON_KIND,
	 {6, 7, 0, 1},
	 {4, 10, 28, 6},
	 MSG_PATHFORMAT_PATH,
	 PLACETEXT_LEFT,
	 GAD_PATHFORMAT_PATH_FOLDER,
	 path_folder_taglist},

	// Path
	{OD_GADGET, STRING_KIND, {6, 7, SIZE_MAXIMUM, 1}, {32, 10, -4, 6}, 0, 0, GAD_PATHFORMAT_PATH, path_path_taglist},

	// Key
	{OD_GADGET,
	 HOTKEY_KIND,
	 {6, 8, SIZE_MAXIMUM, 1},
	 {4, 18, -4, 6},
	 MSG_PATHFORMAT_KEY,
	 PLACETEXT_LEFT,
	 GAD_PATHFORMAT_KEY,
	 path_key_taglist},

	// Mode
	{OD_GADGET,
	 CYCLE_KIND,
	 {6, 9, 12, 1},
	 {4, 26, 28, 6},
	 MSG_PATHFORMAT_NEW_LISTER,
	 PLACETEXT_RIGHT,
	 GAD_PATHFORMAT_NEW_LISTER,
	 path_mode_taglist},

	// Add
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {4, -4, 12, 6},
	 MSG_PATHFORMAT_ADD,
	 0,
	 GAD_PATHFORMAT_ADD,
	 path_relative_taglist},

	// Remove
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_CENTER, POS_RIGHT_JUSTIFY, 12, 1},
	 {0, -4, 12, 6},
	 MSG_PATHFORMAT_REMOVE,
	 0,
	 GAD_PATHFORMAT_REMOVE,
	 path_relative_taglist},

	// Edit
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-4, -4, 12, 6},
	 MSG_PATHFORMAT_EDIT,
	 0,
	 GAD_PATHFORMAT_EDIT,
	 path_relative_taglist},

	// Save
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {3, -3, 12, 6},
	 MSG_PATHFORMAT_SAVE,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_PATHFORMAT_SAVE,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-3, -3, 12, 6},
	 MSG_PATHFORMAT_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_PATHFORMAT_CANCEL,
	 0},

	{OD_END}};
