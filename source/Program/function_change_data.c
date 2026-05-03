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

#include "dopus.h"
#include "function_change.h"

// Protection bits
ConfigWindow _protect_window = {{POS_CENTER, POS_CENTER, 38, 5}, {0, 0, 54, 58}};

struct TagItem _protect_layout[] = {{GTCustom_LayoutRel, GAD_PROTECT_LAYOUT}, {TAG_DONE}};

ObjectDef
	_protect_button_template =
		{OD_GADGET, BUTTON_KIND, {0, 0, 2, 1}, {0, 0, 4, 6}, 0, BUTTONFLAG_TOGGLE_SELECT, 0, _protect_layout},

	_protect_objects[] = {

		// Layout
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {3, 3, -3, -13},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_PROTECT_LAYOUT,
		 0},

		// Filename
		{OD_AREA,
		 TEXTPEN,
		 {2, 0, SIZE_MAX_LESS - 2, 1},
		 {5, 5, -5, 4},
		 0,
		 AREAFLAG_RECESSED | TEXTFLAG_CENTER,
		 GAD_PROTECT_FILENAME,
		 _protect_layout},

		// Old
		{OD_TEXT,
		 TEXTPEN,
		 {10, 1, 0, 1},
		 {5, 17, 0, 0},
		 MSG_SET_PROTECT_OLD,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Set
		{OD_TEXT,
		 TEXTPEN,
		 {10, 2, 0, 1},
		 {5, 25, 0, 0},
		 MSG_SET_PROTECT_SET,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Clear
		{OD_TEXT,
		 TEXTPEN,
		 {10, 3, 0, 1},
		 {5, 33, 0, 0},
		 MSG_SET_PROTECT_CLEAR,
		 TEXTFLAG_RIGHT_JUSTIFY,
		 0,
		 _protect_layout},

		// Ok
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 8, 1},
		 {3, -3, 12, 6},
		 MSG_OK_BUTTON,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_PROTECT_OK,
		 0},

		// All
		{OD_GADGET, BUTTON_KIND, {10, POS_RIGHT_JUSTIFY, 8, 1}, {15, -3, 12, 6}, MSG_ALL_BUTTON, 0, GAD_PROTECT_ALL, 0},

		// Skip
		{OD_GADGET,
		 BUTTON_KIND,
		 {20, POS_RIGHT_JUSTIFY, 8, 1},
		 {27, -3, 12, 6},
		 MSG_SKIP_BUTTON,
		 0,
		 GAD_PROTECT_SKIP,
		 0},

		// Abort
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 8, 1},
		 {-3, -3, 12, 6},
		 MSG_ABORT_BUTTON,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_PROTECT_ABORT,
		 0},

		{OD_END}};
