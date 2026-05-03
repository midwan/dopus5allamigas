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

#include "play.h"

ModuleInfo module_info = {1, "play.module", "play.catalog", 0, 1, {{0, "Play", MSG_PLAY_DESC, FUNCF_NO_ARGS, 0}}};

ConfigWindow play_window = {{POS_CENTER, POS_CENTER, 40, 5}, {0, 0, 8, 18}};

struct TagItem play_layout[] = {{GTCustom_LayoutRel, GAD_PLAY_LAYOUT}, {TAG_END}};

ObjectDef play_objects[] = {

	// Information area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {2, 2, -2, -10},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_PLAY_LAYOUT,
	 0},

	// Filename title
	{OD_TEXT, TEXTPEN, {8, 0, 0, 1}, {2, 2, 0, 0}, MSG_FILENAME, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Filename
	{OD_AREA, TEXTPEN, {9, 0, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_FILENAME, play_layout},

	// Length title
	{OD_TEXT, TEXTPEN, {8, 1, 0, 1}, {2, 2, 0, 0}, MSG_LENGTH, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Length
	{OD_AREA, TEXTPEN, {9, 1, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_LENGTH, play_layout},

	// Type title
	{OD_TEXT, TEXTPEN, {8, 2, 0, 1}, {2, 2, 0, 0}, MSG_TYPE, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Type
	{OD_AREA, TEXTPEN, {9, 2, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_TYPE, play_layout},

	// Status title
	{OD_TEXT, TEXTPEN, {8, 3, 0, 0}, {2, 2, 0, 0}, MSG_STATUS, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Status
	{OD_AREA, TEXTPEN, {9, 3, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_STATUS, play_layout},

	// Next button
	{OD_GADGET, BUTTON_KIND, {0, POS_RIGHT_JUSTIFY, 12, 1}, {2, -2, 12, 6}, MSG_NEXT, 0, GAD_PLAY_NEXT, 0},

	// Abort
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-2, -2, 12, 6},
	 MSG_ABORT,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_PLAY_ABORT,
	 0},

	{OD_END}};

ObjectDef play_objects_single[] = {

	// Information area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {2, 2, -2, -10},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_PLAY_LAYOUT,
	 0},

	// Filename title
	{OD_TEXT, TEXTPEN, {8, 0, 0, 1}, {2, 2, 0, 0}, MSG_FILENAME, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Filename
	{OD_AREA, TEXTPEN, {9, 0, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_FILENAME, play_layout},

	// Length title
	{OD_TEXT, TEXTPEN, {8, 1, 0, 1}, {2, 2, 0, 0}, MSG_LENGTH, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Length
	{OD_AREA, TEXTPEN, {9, 1, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_LENGTH, play_layout},

	// Type title
	{OD_TEXT, TEXTPEN, {8, 2, 0, 1}, {2, 2, 0, 0}, MSG_TYPE, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Type
	{OD_AREA, TEXTPEN, {9, 2, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_TYPE, play_layout},

	// Status title
	{OD_TEXT, TEXTPEN, {8, 3, 0, 0}, {2, 2, 0, 0}, MSG_STATUS, TEXTFLAG_RIGHT_JUSTIFY, 0, play_layout},

	// Status
	{OD_AREA, TEXTPEN, {9, 3, SIZE_MAXIMUM, 1}, {2, 2, -4, 0}, 0, AREAFLAG_ERASE, GAD_PLAY_STATUS, play_layout},

	// Abort
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_CENTER, POS_RIGHT_JUSTIFY, 12, 1},
	 {-2, -2, 12, 6},
	 MSG_ABORT,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_PLAY_ABORT,
	 0},

	{OD_END}};
