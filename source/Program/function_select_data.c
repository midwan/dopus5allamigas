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

UWORD
_select_cycle_labels[] = {MSG_SELECT_IGNORE, MSG_SELECT_MATCH, MSG_SELECT_NOMATCH, 0},

	_select_compare_labels[] = {MSG_SELECT_NEWER, MSG_SELECT_OLDER, MSG_SELECT_DIFFERENT, 0},

	_select_include_labels[] = {MSG_SELECT_INCLUDE, MSG_SELECT_EXCLUDE, 0},

	_select_entry_type_labels[] = {MSG_SELECT_ENTRY_BOTH, MSG_SELECT_ENTRY_FILES, MSG_SELECT_ENTRY_DIRS, 0};

struct TagItem _select_name_tags[] = {{GTST_MaxChars, 59}, {GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA}, {TAG_END}},

			   _select_cycle_tags[] = {{GTCustom_LocaleLabels, (IPTR)_select_cycle_labels},
									   {GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA},
									   {TAG_END}},

			   _select_date_tags[] = {{GTST_MaxChars, 9}, {GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA}, {TAG_END}},

			   _select_compare_tags[] = {{GTCustom_LocaleLabels, (IPTR)_select_compare_labels},
										 {GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA},
										 {TAG_END}},

			   _select_path_tags[] = {{GTST_MaxChars, 256}, {GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA}, {TAG_END}},

			   _select_layout_tags[] = {{GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA}, {TAG_END}},

			   _select_include_tags[] = {{GTCustom_LocaleLabels, (IPTR)_select_include_labels},
										 {GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA},
										 {TAG_END, 0}},

			   _select_entry_type_tags[] = {{GTCustom_LocaleLabels, (IPTR)_select_entry_type_labels},
											{GTCustom_LayoutRel, GAD_SELECT_LAYOUT_AREA},
											{TAG_END}};

ObjectDef _function_select_objects[] = {

	// Layout area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, 6},
	 {4, 4, -4, 46},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_SELECT_LAYOUT_AREA,
	 0},

	// Name
	{OD_GADGET,
	 STRING_KIND,
	 {8, 0, 20, 1},
	 {6, 4, 136, 6},
	 MSG_SELECT_NAME,
	 PLACETEXT_LEFT,
	 GAD_SELECT_NAME_STRING,
	 _select_name_tags},

	// Name cycle
	{OD_GADGET, CYCLE_KIND, {28, 0, 10, 1}, {150, 4, 24, 6}, 0, 0, GAD_SELECT_NAME_CYCLE, _select_cycle_tags},

	// Date from
	{OD_GADGET,
	 STRING_KIND,
	 {8, 1, 8, 1},
	 {6, 12, 64, 6},
	 MSG_SELECT_DATE_FROM,
	 PLACETEXT_LEFT,
	 GAD_SELECT_DATE_FROM,
	 _select_date_tags},

	// Date to
	{OD_GADGET,
	 STRING_KIND,
	 {20, 1, 8, 1},
	 {78, 12, 64, 6},
	 MSG_SELECT_DATE_TO,
	 PLACETEXT_LEFT,
	 GAD_SELECT_DATE_TO,
	 _select_date_tags},

	// Date cycle
	{OD_GADGET, CYCLE_KIND, {28, 1, 10, 1}, {150, 12, 24, 6}, 0, 0, GAD_SELECT_DATE_CYCLE, _select_cycle_tags},

	// Protection text
	{OD_TEXT, TEXTPEN, {7, 2, 0, 0}, {6, 23, 0, 0}, MSG_SELECT_PROT, TEXTFLAG_RIGHT_JUSTIFY, 0, _select_layout_tags},

	// Protection off text
	{OD_TEXT,
	 TEXTPEN,
	 {19, 2, 0, 0},
	 {78, 23, 0, 0},
	 MSG_SELECT_PROT_OFF,
	 TEXTFLAG_RIGHT_JUSTIFY,
	 0,
	 _select_layout_tags},

	// Protection cycle
	{OD_GADGET, CYCLE_KIND, {28, 2, 10, 1}, {150, 20, 24, 6}, 0, 0, GAD_SELECT_PROT_CYCLE, _select_cycle_tags},

	// Compare type cycle
	{OD_GADGET,
	 CYCLE_KIND,
	 {20, 3, 8, 1},
	 {78, 28, 64, 6},
	 MSG_SELECT_COMPARE,
	 0,
	 GAD_SELECT_COMPARE_TYPE_CYCLE,
	 _select_compare_tags},

	// Compare cycle
	{OD_GADGET, CYCLE_KIND, {28, 3, 10, 1}, {150, 28, 24, 6}, 0, 0, GAD_SELECT_COMPARE_CYCLE, _select_cycle_tags},

	// Include/exclude radio
	{OD_GADGET,
	 MX_KIND,
	 {4, 4, 1, 1},
	 {6, 38, 7, 1},
	 0,
	 PLACETEXT_RIGHT,
	 GAD_SELECT_INCLUDE_EXCLUDE,
	 _select_include_tags},

	// Files/Dirs/Both cycle
	{OD_GADGET,
	 CYCLE_KIND,
	 {20, 4, 8, 1},
	 {78, 36, 64, 6},
	 MSG_SELECT_ENTRY_TYPE,
	 0,
	 GAD_SELECT_ENTRY_TYPE,
	 _select_entry_type_tags},

	// Ok
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 10, 1},
	 {4, -4, 24, 6},
	 MSG_OKAY,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_SELECT_OK,
	 0},

	// Simple
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_CENTER, POS_RIGHT_JUSTIFY, 10, 1},
	 {0, -4, 24, 6},
	 MSG_SELECT_SIMPLE,
	 0,
	 GAD_SELECT_SIMPLE,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 10, 1},
	 {-4, -4, 24, 6},
	 MSG_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_SELECT_CANCEL,
	 0},

	{OD_END}};
