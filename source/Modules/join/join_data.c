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

#include "join.h"

#define ARG_1 "FILE=FROM/M/A,AS=TO/A"
#define ARG_2 "FILE=FROM/A,TO/A,CHUNK/N/A,STEM"

char *func_templates[] = {ARG_1, ARG_2};

ModuleInfo_2 module_info = {1,
							"join.module",
							"join.catalog",
							0,
							2,
							{{JOIN, "Join", MSG_JOIN_DESC, FUNCF_WANT_ENTRIES | FUNCF_WANT_DEST, ARG_1},
							 {SPLIT, "Split", MSG_SPLIT_DESC, FUNCF_WANT_ENTRIES | FUNCF_WANT_DEST, ARG_2}}};

ConfigWindow join_window = {{POS_CENTER, POS_CENTER, 45, 12}, {0, 0, 16, 40}},

			 split_window = {{POS_CENTER, POS_CENTER, 48, 5}, {0, 0, 32, 52}};

UWORD
chunk_labels[] = {MSG_CHUNK_CUSTOM,
				  MSG_CHUNK_FLOPPY_FFS,
				  MSG_CHUNK_HD_FLOPPY_FFS,
				  MSG_CHUNK_FLOPPY_OFS,
				  MSG_CHUNK_HD_FLOPPY_OFS,
				  MSG_CHUNK_MSDOS,
				  MSG_CHUNK_MSDOS_HD,
				  0};

struct TagItem

	join_layout_tags[] = {{GTCustom_LayoutRel, GAD_JOIN_LAYOUT}, {GTCustom_CopyTags, TRUE}, {TAG_END}},

	join_lister_tags[] = {{DLV_ShowSelected, 0}, {DLV_ShowFilenames, 1}, {TAG_MORE, (ULONG)join_layout_tags}},

	join_popup_tags[] = {{GTCustom_Control, GAD_JOIN_TO_FIELD},
						 {DFB_DefPath, (ULONG) "ram:"},
						 {TAG_MORE, (ULONG)join_layout_tags}},

	join_to_tags[] = {{GTST_MaxChars, 256}, {TAG_MORE, (ULONG)join_layout_tags}},

	split_from_popup_tags[] = {{GTCustom_Control, GAD_SPLIT_FROM}, {TAG_MORE, (ULONG)join_layout_tags}},

	split_to_popup_tags[] = {{GTCustom_Control, GAD_SPLIT_TO}, {TAG_MORE, (ULONG)join_layout_tags}},

	split_stem_tags[] = {{GTST_MaxChars, 27}, {TAG_MORE, (ULONG)join_layout_tags}},

	split_into_tags[] = {{GTIN_MaxChars, 5}, {TAG_MORE, (ULONG)join_layout_tags}},

	split_sizes_tags[] = {{GTCustom_LocaleLabels, (ULONG)chunk_labels}, {TAG_MORE, (ULONG)join_layout_tags}};

ObjectDef
	join_objects[] =
		{

			// Background area
			{OD_AREA,
			 0,
			 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
			 {2, 2, -2, -10},
			 0,
			 AREAFLAG_RECESSED | AREAFLAG_ERASE,
			 GAD_JOIN_LAYOUT,
			 0},

			// Lister
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {0, 1, SIZE_MAXIMUM, SIZE_MAX_LESS - 2},
			 {4, 4, -4, -18},
			 MSG_JOIN_LISTER,
			 LISTVIEWFLAG_CURSOR_KEYS,
			 GAD_JOIN_LISTER,
			 join_lister_tags},

			// Add
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, POS_RIGHT_JUSTIFY - 1, POS_PROPORTION + 20, 1},
			 {4, -12, -1, 6},
			 MSG_JOIN_ADD,
			 0,
			 GAD_JOIN_ADD,
			 join_layout_tags},

			// Remove
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_REL_RIGHT, POS_RIGHT_JUSTIFY - 1, POS_PROPORTION + 20, 1},
			 {0, -12, -1, 6},
			 MSG_JOIN_REMOVE,
			 0,
			 GAD_JOIN_REMOVE,
			 join_layout_tags},

			// Clear
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_REL_RIGHT, POS_RIGHT_JUSTIFY - 1, POS_PROPORTION + 20, 1},
			 {0, -12, -1, 6},
			 MSG_JOIN_CLEAR,
			 0,
			 GAD_JOIN_CLEAR,
			 join_layout_tags},

			// Move Up
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_REL_RIGHT, POS_RIGHT_JUSTIFY - 1, POS_PROPORTION + 20, 1},
			 {0, -12, -1, 6},
			 MSG_JOIN_MOVE_UP,
			 0,
			 GAD_JOIN_MOVE_UP,
			 join_layout_tags},

			// Move Down
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_REL_RIGHT, POS_RIGHT_JUSTIFY - 1, SIZE_MAXIMUM, 1},
			 {0, -12, -4, 6},
			 MSG_JOIN_MOVE_DOWN,
			 0,
			 GAD_JOIN_MOVE_DOWN,
			 join_layout_tags},

			// To popup
			{OD_GADGET,
			 FILE_BUTTON_KIND,
			 {5, POS_RIGHT_JUSTIFY, 0, 1},
			 {4, -4, 28, 6},
			 MSG_JOIN_TO,
			 PLACETEXT_LEFT | FILEBUTFLAG_SAVE,
			 GAD_JOIN_TO_POPUP,
			 join_popup_tags},

			// To field
			{OD_GADGET,
			 STRING_KIND,
			 {POS_REL_RIGHT, POS_RIGHT_JUSTIFY, SIZE_MAXIMUM, 1},
			 {0, -4, -4, 6},
			 0,
			 0,
			 GAD_JOIN_TO_FIELD,
			 join_to_tags},

			// Join
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, POS_RIGHT_JUSTIFY, 12, 1},
			 {2, -2, 12, 6},
			 MSG_JOIN_JOIN,
			 BUTTONFLAG_OKAY_BUTTON,
			 GAD_JOIN_OK,
			 0},

			// Cancel
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
			 {-2, -2, 12, 6},
			 MSG_JOIN_CANCEL,
			 BUTTONFLAG_CANCEL_BUTTON,
			 GAD_JOIN_CANCEL,
			 0},

			{OD_END}},

	split_objects[] = {

		// Background area
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {2, 2, -2, -10},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_JOIN_LAYOUT,
		 0},

		// Split file popup
		{OD_GADGET,
		 FILE_BUTTON_KIND,
		 {12, 0, 0, 1},
		 {4, 4, 28, 6},
		 MSG_SPLIT_FROM,
		 PLACETEXT_LEFT,
		 GAD_SPLIT_FROM_POPUP,
		 split_from_popup_tags},

		// Split file
		{OD_GADGET,
		 STRING_KIND,
		 {POS_REL_RIGHT, 0, SIZE_MAXIMUM, 1},
		 {0, 4, -4, 6},
		 0,
		 0,
		 GAD_SPLIT_FROM,
		 join_to_tags},

		// Split to popup
		{OD_GADGET,
		 DIR_BUTTON_KIND,
		 {12, 1, 0, 1},
		 {4, 12, 28, 6},
		 MSG_SPLIT_TO,
		 PLACETEXT_LEFT,
		 GAD_SPLIT_TO_POPUP,
		 split_to_popup_tags},

		// Split to
		{OD_GADGET, STRING_KIND, {POS_REL_RIGHT, 1, SIZE_MAXIMUM, 1}, {0, 12, -4, 6}, 0, 0, GAD_SPLIT_TO, join_to_tags},

		// Chunk size
		{OD_GADGET,
		 INTEGER_KIND,
		 {12, 2, 8, 1},
		 {4, 20, 8, 6},
		 MSG_SPLIT_INTO,
		 PLACETEXT_LEFT,
		 GAD_SPLIT_INTO,
		 split_into_tags},

		// Chunk text
		{OD_GADGET,
		 CYCLE_KIND,
		 {POS_REL_RIGHT, 2, SIZE_MAXIMUM, 1},
		 {0, 20, -4, 6},
		 0,
		 0,
		 GAD_SPLIT_SIZES,
		 split_sizes_tags},

		// Stem
		{OD_GADGET,
		 STRING_KIND,
		 {12, 3, SIZE_MAXIMUM, 1},
		 {4, 28, -4, 6},
		 MSG_SPLIT_STEM,
		 PLACETEXT_LEFT,
		 GAD_SPLIT_STEM,
		 split_stem_tags},

		// Split
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 12, 1},
		 {2, -2, 12, 6},
		 MSG_SPLIT_SPLIT,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_SPLIT_OK,
		 0},

		// Skip
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_CENTER, POS_RIGHT_JUSTIFY, 12, 1},
		 {0, -2, 12, 6},
		 MSG_SPLIT_SKIP,
		 0,
		 GAD_SPLIT_SKIP,
		 0},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
		 {-2, -2, 12, 6},
		 MSG_JOIN_CANCEL,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_JOIN_CANCEL,
		 0},

		{OD_END}};
