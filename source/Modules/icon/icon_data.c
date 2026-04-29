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

#include "icon.h"

ModuleInfo module_info =
	{1, "icon.module", "icon.catalog", 0, 1, {{0, "IconInfo", MSG_ICONINFO_DESC, FUNCF_NO_ARGS, 0}}};

ConfigWindow icon_disk_window = {{POS_CENTER, POS_CENTER, 62, 9}, {0, 0, 12, 38}},

			 icon_tool_window = {{POS_CENTER, POS_CENTER, 62, 17}, {0, 0, 12, 54}},

			 icon_project_window = {{POS_CENTER, POS_CENTER, 62, 18}, {0, 0, 12, 60}},

			 *icon_windows[] = {
				 &icon_disk_window,
				 &icon_tool_window,
				 &icon_tool_window,
				 &icon_project_window,
				 &icon_tool_window,
};

struct TagItem icon_layout[] = {{GTCustom_CopyTags, 1}, {GTCustom_LayoutRel, GAD_ICON_LAYOUT_AREA}, {TAG_END}},

			   icon_stack_tags[] = {{GTIN_MaxChars, 10}, {TAG_MORE, (IPTR)icon_layout}},

			   icon_protection_tags[] = {{DLV_MultiSelect, 1}, {DLV_NoScroller, 1}, {TAG_MORE, (IPTR)icon_layout}},

			   icon_comment_tags[] = {{GTST_MaxChars, 79}, {TAG_MORE, (IPTR)icon_layout}},

			   icon_tooltypes_tags[] = {{DLV_ShowSelected, 0},
										{DLV_Flags, PLACETEXT_LEFT},
										{DLV_DragNotify, 2},
										{GTST_MaxChars, 255},
										{GTCustom_NoSelectNext, TRUE},
										{TAG_MORE, (IPTR)icon_layout}},

			   icon_default_tool_tags[] = {{GTST_MaxChars, 255}, {TAG_MORE, (IPTR)icon_layout}},

			   icon_location_tags[] = {{GTST_MaxChars, 256}, {TAG_MORE, (IPTR)icon_layout}};

ObjectDef

	// Common to all icons
	icon_info_objects[] =
		{

			// Background area
			{OD_AREA,
			 0,
			 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
			 {2, 2, -2, -10},
			 0,
			 AREAFLAG_RECESSED | AREAFLAG_ERASE,
			 GAD_ICON_LAYOUT_AREA,
			 0},

			// Icon display area
			{OD_AREA,
			 0,
			 {20, 0, 22, 6},
			 {4, 4, 8, 4},
			 0,
			 /*AREAFLAG_RAISED|AREAFLAG_ERASE|AREAFLAG_FILL_COLOUR,*/
			 AREAFLAG_RAISED | AREAFLAG_FILL_COLOUR | AREAFLAG_OPTIM /*|AREAFLAG_ICON*/,
			 GAD_ICON_IMAGE,
			 icon_layout},

			// Save
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, POS_RIGHT_JUSTIFY, 12, 1},
			 {2, -2, 12, 6},
			 MSG_ICON_SAVE,
			 BUTTONFLAG_OKAY_BUTTON,
			 GAD_ICON_SAVE,
			 0},

			// Cancel
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
			 {-2, -2, 12, 6},
			 MSG_ICON_CANCEL,
			 BUTTONFLAG_CANCEL_BUTTON,
			 GAD_ICON_CANCEL,
			 0},

			{OD_END}},

	// Next gadget
	icon_info_next_object[] =
		{

			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_CENTER, POS_RIGHT_JUSTIFY, 12, 1},
			 {0, -2, 12, 6},
			 MSG_ICON_NEXT,
			 0,
			 GAD_ICON_NEXT,
			 0},

			{OD_END}},

	// Blocks/Bytes/Stack objects
	icon_file_info_objects[] =
		{

			// Blocks
			{OD_TEXT, TEXTPEN, {8, 1, 0, 1}, {4, 4, 0, 0}, MSG_BLOCKS, TEXTFLAG_RIGHT_JUSTIFY, 0, icon_layout},

			// Blocks field
			{OD_TEXT, TEXTPEN, {9, 1, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_ICON_BLOCKS, icon_layout},

			// Bytes
			{OD_TEXT, TEXTPEN, {8, 2, 0, 1}, {4, 6, 0, 0}, MSG_BYTES, TEXTFLAG_RIGHT_JUSTIFY, 0, icon_layout},

			// Bytes field
			{OD_TEXT, TEXTPEN, {9, 2, 0, 1}, {4, 6, 0, 0}, 0, 0, GAD_ICON_BYTES, icon_layout},

			// Stack
			{OD_GADGET, INTEGER_KIND, {9, 4, 6, 1}, {4, 8, 12, 4}, MSG_STACK, 0, GAD_ICON_STACK, icon_stack_tags},

			{OD_END}},

	// Tool/Project common objects
	icon_toolproj_objects[] =
		{

			// Protection bits lister
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {42, 0, SIZE_MAXIMUM, 6},
			 {20, 4, -4, 4},
			 0,
			 0,
			 GAD_ICON_PROTECTION,
			 icon_protection_tags},

			// Last changed
			{OD_TEXT,
			 TEXTPEN,
			 {19, 6, 0, 1},
			 /*{0,POS_RIGHT_JUSTIFY,0,1},*/
			 {4, 12, 0, 0},
			 /*{0,0,0,0},*/
			 MSG_LAST_CHANGED,
			 TEXTFLAG_RIGHT_JUSTIFY,
			 0,
			 icon_layout},

			// Last changed field
			{OD_TEXT, TEXTPEN, {20, 6, 0, 1}, {4, 12, 0, 0}, 0, 0, GAD_ICON_LAST_CHANGED, icon_layout},

			// Location
			{OD_GADGET,
			 FIELD_KIND,
			 {20, 7, SIZE_MAXIMUM, 1},
			 {4, 16, -4, 4},
			 MSG_LOCATION,
			 0,
			 GAD_ICON_LOCATION,
			 icon_location_tags},

			// Comment
			{OD_GADGET,
			 STRING_KIND,
			 {20, 8, SIZE_MAXIMUM, 1},
			 {4, 22, -4, 4},
			 MSG_COMMENT,
			 0,
			 GAD_ICON_COMMENT,
			 icon_comment_tags},

			// Tooltypes edit field
			{OD_GADGET,
			 STRING_KIND,
			 {20, POS_RIGHT_JUSTIFY, SIZE_MAXIMUM, 1},
			 {4, -4, -4, 4},
			 0,
			 0,
			 GAD_ICON_TOOLTYPES_EDIT,
			 icon_tooltypes_tags},

			{OD_END}},

	// Tool only objects
	icon_tool_objects[] =
		{

			// Tooltypes
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {20, 9, SIZE_MAXIMUM, SIZE_MAX_LESS - 2},
			 {4, 28, -4, -2},
			 MSG_TOOLTYPES,
			 0,
			 GAD_ICON_TOOLTYPES,
			 icon_tooltypes_tags},

			// New button
			{OD_GADGET,
			 BUTTON_KIND,
			 /*{9,POS_RIGHT_JUSTIFY-3,10,1},*/
			 {9, 10, 10, 1},
			 /*{4,-15,0,6},*/
			 {4, 34, 0, 6},
			 MSG_NEW,
			 0,
			 GAD_ICON_TOOLTYPES_NEW,
			 icon_layout},

			// Delete button
			{OD_GADGET,
			 BUTTON_KIND,
			 /*{9,POS_RIGHT_JUSTIFY-2,10,1},*/
			 {9, 11, 10, 1},
			 /*{4,-8,0,6},*/
			 {4, 40, 0, 6},
			 MSG_DELETE,
			 0,
			 GAD_ICON_TOOLTYPES_DELETE,
			 icon_layout},

			// Sort button
			{OD_GADGET,
			 BUTTON_KIND,
			 /*{9,POS_RIGHT_JUSTIFY-1,10,1},*/
			 {9, 12, 10, 1},
			 /*{4,-1,0,6},*/
			 {4, 46, 0, 6},
			 MSG_SORT,
			 0,
			 GAD_ICON_TOOLTYPES_SORT,
			 icon_layout},

			{OD_END}},

	// Project gadgets
	icon_project_objects[] =
		{

			// Default tool glass
			{OD_GADGET,
			 FILE_BUTTON_KIND,
			 /*{20,POS_RIGHT_JUSTIFY-7,0,1},*/
			 {20, 9, 0, 1},
			 /*{4,-14,28,4},*/
			 {4, 28, 28, 4},
			 MSG_DEFAULT_TOOL,
			 0,
			 GAD_ICON_DEFAULT_TOOL_GLASS,
			 icon_layout},

			// Default tool
			{OD_GADGET,
			 STRING_KIND,
			 /*{20,POS_RIGHT_JUSTIFY-7,SIZE_MAXIMUM,1},*/
			 {20, 9, SIZE_MAXIMUM, 1},
			 /*{32,-14,-4,4},*/
			 {32, 28, -4, 4},
			 0,
			 0,
			 GAD_ICON_DEFAULT_TOOL,
			 icon_default_tool_tags},

			// Tooltypes
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {20, 10, SIZE_MAXIMUM, SIZE_MAX_LESS - 2},
			 {4, 34, -4, -2},
			 MSG_TOOLTYPES,
			 0,
			 GAD_ICON_TOOLTYPES,
			 icon_tooltypes_tags},

			// New button
			{OD_GADGET, BUTTON_KIND, {9, 11, 10, 1}, {4, 40, 0, 6}, MSG_NEW, 0, GAD_ICON_TOOLTYPES_NEW, icon_layout},

			// Delete button
			{OD_GADGET,
			 BUTTON_KIND,
			 {9, 12, 10, 1},
			 {4, 46, 0, 6},
			 MSG_DELETE,
			 0,
			 GAD_ICON_TOOLTYPES_DELETE,
			 icon_layout},

			// Sort button
			{OD_GADGET, BUTTON_KIND, {9, 13, 10, 1}, {4, 52, 0, 6}, MSG_SORT, 0, GAD_ICON_TOOLTYPES_SORT, icon_layout},

			{OD_END}},

	// Disk objects
	icon_disk_objects[] = {

		// Size
		{OD_TEXT, TEXTPEN, {7, 1, 0, 1}, {4, 4, 0, 0}, MSG_SIZE, TEXTFLAG_RIGHT_JUSTIFY, 0, icon_layout},

		// Size field
		{OD_TEXT, TEXTPEN, {8, 1, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_ICON_SIZE, icon_layout},

		// Used
		{OD_TEXT, TEXTPEN, {7, 2, 0, 1}, {4, 6, 0, 0}, MSG_USED, TEXTFLAG_RIGHT_JUSTIFY, 0, icon_layout},

		// Used field
		{OD_TEXT, TEXTPEN, {8, 2, 0, 1}, {4, 6, 0, 0}, 0, 0, GAD_ICON_USED, icon_layout},

		// Free
		{OD_TEXT, TEXTPEN, {7, 3, 0, 1}, {4, 8, 0, 0}, MSG_FREE, TEXTFLAG_RIGHT_JUSTIFY, 0, icon_layout},

		// Free field
		{OD_TEXT, TEXTPEN, {8, 3, 0, 1}, {4, 8, 0, 0}, 0, 0, GAD_ICON_FREE, icon_layout},

		// File system
		{OD_TEXT, TEXTPEN, {7, 4, 0, 1}, {4, 10, 0, 0}, MSG_FILE_SYSTEM, TEXTFLAG_RIGHT_JUSTIFY, 0, icon_layout},

		// File system field
		{OD_TEXT, TEXTPEN, {8, 4, 0, 1}, {4, 10, 0, 0}, 0, 0, GAD_ICON_FILE_SYSTEM, icon_layout},

		// Status text
		{OD_AREA, TEXTPEN, {42, 2, SIZE_MAXIMUM, 1}, {20, 4, -4, 0}, MSG_VOLUME_IS, TEXTFLAG_CENTER, 0, icon_layout},

		// Status text field
		{OD_AREA, TEXTPEN, {42, 3, SIZE_MAXIMUM, 1}, {20, 6, -4, 0}, 0, TEXTFLAG_CENTER, GAD_ICON_STATUS, icon_layout},

		// Created
		{OD_TEXT, TEXTPEN, {19, 6, 0, 1}, {4, 12, 0, 0}, MSG_CREATED, TEXTFLAG_RIGHT_JUSTIFY, 0, icon_layout},

		// Created field
		{OD_TEXT, TEXTPEN, {20, 6, 0, 1}, {4, 12, 0, 0}, 0, 0, GAD_ICON_LAST_CHANGED, icon_layout},

		// Default tool glass
		{OD_GADGET,
		 FILE_BUTTON_KIND,
		 {20, 7, 0, 1},
		 {4, 16, 28, 4},
		 MSG_DEFAULT_TOOL,
		 0,
		 GAD_ICON_DEFAULT_TOOL_GLASS,
		 icon_layout},

		// Default tool
		{OD_GADGET,
		 STRING_KIND,
		 {20, 7, SIZE_MAXIMUM, 1},
		 {32, 16, -4, 4},
		 0,
		 0,
		 GAD_ICON_DEFAULT_TOOL,
		 icon_default_tool_tags},

		{OD_END}};

short protect_lookup[] = {FIBF_SCRIPT, FIBF_ARCHIVE, FIBF_READ, FIBF_WRITE, FIBF_EXECUTE, FIBF_DELETE};

unsigned long filesystem_table[] = {ID_UNREADABLE_DISK,
									MSG_UNREADABLE,
									ID_NOT_REALLY_DOS,
									MSG_NDOS,
									ID_MSDOS_DISK,
									MSG_MSDOS,
									ID_AFS_PRO,
									MSG_AFS,
									ID_AFS_USER,
									MSG_AFS,
									ID_AFS_MULTI,
									MSG_AFS,
									ID_PFS_FLOPPY,
									MSG_PFS1,
									ID_PFS_HARD,
									MSG_PFS1,
									ID_PFS2_DISK,
									MSG_PFS2,
									ID_PFS3_DISK,
									MSG_PFS3,
									ID_PFS2_SC_DISK,
									MSG_PFS2,
									ID_PFS3_SC_DISK,
									MSG_PFS3,
									ID_PFS3_MULTI,
									MSG_PFS3_MULTI,
									ID_DOS_DISK,
									MSG_OFS,
									ID_FFS_DISK,
									MSG_FFS,
									ID_INTER_DOS_DISK,
									MSG_IOFS,
									ID_INTER_FFS_DISK,
									MSG_IFFS,
									ID_FASTDIR_DOS_DISK,
									MSG_COFS,
									ID_FASTDIR_FFS_DISK,
									MSG_CFFS,
									ID_FFS7_DISK,
									MSG_FFS7,
									ID_VP255_DISK,
									MSG_VP255,
									ID_SFS0_DISK,
									MSG_SFS0,
									ID_SFS1_DISK,
									MSG_SFS1,
									ID_SFS2_DISK,
									MSG_SFS2,
									ID_SFS3_DISK,
									MSG_SFS3,
									ID_JXFS_DISK,
									MSG_JXFS,
									ID_BOX0_DISK,
									MSG_BOX,
									ID_FAT32_DISK,
									MSG_FAT32,
									ID_EXFAT_DISK,
									MSG_EXFAT,
									ID_EXT2_DISK,
									MSG_EXT2FS,
									ID_HFS_DISK,
									MSG_HFS,
									ID_NTFS_DISK,
									MSG_NTFS,
									ID_SWAP_DISK,
									MSG_SWAP,
									0,
									MSG_UNKNOWN};

short icon_type_labels[] = {WBDISK,
							MSG_ICON_DISK,
							WBDRAWER,
							MSG_ICON_DRAWER,
							WBTOOL,
							MSG_ICON_TOOL,
							WBPROJECT,
							MSG_ICON_PROJECT,
							WBGARBAGE,
							MSG_ICON_GARBAGE,
							WBDEVICE,
							MSG_ICON_DEVICE,
							WBKICK,
							MSG_ICON_KICK,
							0};

UWORD oldicon_imagedata[1] = {0xffff};

struct Image oldicon_image = {0, 0, 1, 1, 1, oldicon_imagedata, 1, 0, 0};
