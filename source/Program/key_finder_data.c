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
#include "key_finder.h"

ConfigWindow keyfinder_window = {{POS_CENTER, POS_CENTER, 60, 10}, {0, 0, 0, 0}};

struct TagItem _keyfinder_key_taglist[] = {{GTST_MaxChars, 80}, {TAG_END}},

			   _keyfinder_func_taglist[] = {{DLV_ReadOnly, TRUE}, {DLV_ShowSeparators, TRUE}, {TAG_END}};

ObjectDef keyfinder_objects[] = {

	// Key field
	{OD_GADGET,
	 HOTKEY_KIND,
	 {8, 0, SIZE_MAX_LESS - 1, 1},
	 {4, 4, -4, 4},
	 MSG_KEYFINDER_KEY,
	 PLACETEXT_LEFT,
	 GAD_KEYFINDER_KEY,
	 _keyfinder_key_taglist},

	// Found in display
	{OD_GADGET,
	 TEXT_KIND,
	 {8, 1, SIZE_MAX_LESS - 1, 1},
	 {4, 14, -4, 6},
	 MSG_KEYFINDER_FOUND,
	 PLACETEXT_LEFT,
	 GAD_KEYFINDER_FOUND,
	 0},

	// Function display
	{OD_GADGET,
	 MY_LISTVIEW_KIND,
	 {8, 2, SIZE_MAX_LESS - 1, SIZE_MAXIMUM},
	 {4, 22, -4, -4},
	 0,
	 0,
	 GAD_KEYFINDER_FUNCTION,
	 _keyfinder_func_taglist},

	{OD_END}};
