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

#include "show.h"

ModuleInfo module_info = {1, "show.module", "show.catalog", 0, 1, {{0, "Show", MSG_SHOW_DESC, FUNCF_NO_ARGS, 0}}};

ConfigWindow picture_info_window = {{POS_CENTER, POS_CENTER, 48, 10}, {0, 0, 20, 60}},

			 anim_info_window = {{POS_CENTER, POS_CENTER, 48, 11}, {0, 0, 20, 60}};

UWORD
print_aspect_labels[] = {MSG_PRINT_ASPECT_PORTRAIT, MSG_PRINT_ASPECT_LANDSCAPE, 0},

	print_image_labels[] = {MSG_PRINT_IMAGE_POSITIVE, MSG_PRINT_IMAGE_NEGATIVE, 0},

	print_shade_labels[] = {MSG_PRINT_SHADE_BW, MSG_PRINT_SHADE_GREY, MSG_PRINT_SHADE_COLOUR, 0},

	print_placement_labels[] = {MSG_PRINT_PLACEMENT_CENTER, MSG_PRINT_PLACEMENT_LEFT, 0};

struct TagItem info_layout[] = {{GTCustom_LayoutRel, GAD_INFO_AREA}, {TAG_END}},

			   print_layout[] = {{GTCustom_LayoutRel, GAD_PRINT_AREA}, {TAG_END}},

			   print_aspect_tags[] = {{GTCustom_LocaleLabels, (IPTR)print_aspect_labels},
									  {TAG_MORE, (IPTR)print_layout}},

			   print_image_tags[] = {{GTCustom_LocaleLabels, (IPTR)print_image_labels},
									 {TAG_MORE, (IPTR)print_layout}},

			   print_shade_tags[] = {{GTCustom_LocaleLabels, (IPTR)print_shade_labels},
									 {TAG_MORE, (IPTR)print_layout}},

			   print_placement_tags[] = {{GTCustom_LocaleLabels, (IPTR)print_placement_labels},
										 {TAG_MORE, (IPTR)print_layout}};

ObjectDef
	picture_info_objects[] =
		{

			// Information area
			{OD_AREA,
			 0,
			 {0, 0, SIZE_MAXIMUM, 5},
			 {2, 2, -2, 10},
			 0,
			 AREAFLAG_RECESSED | AREAFLAG_ERASE,
			 GAD_INFO_AREA,
			 0},

			// Type title
			{OD_TEXT, TEXTPEN, {16, 4, 0, 1}, {4, 4, 0, 0}, MSG_INFO_TYPE, TEXTFLAG_RIGHT_JUSTIFY, 0, info_layout},

			// Type field
			{OD_TEXT, TEXTPEN, {17, 4, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_INFO_TYPE, info_layout},

			{OD_END}},

	anim_info_objects[] =
		{

			// Information area
			{OD_AREA,
			 0,
			 {0, 0, SIZE_MAXIMUM, 6},
			 {2, 2, -2, 10},
			 0,
			 AREAFLAG_RECESSED | AREAFLAG_ERASE,
			 GAD_INFO_AREA,
			 0},

			// Frame info title
			{OD_TEXT, TEXTPEN, {16, 4, 0, 1}, {4, 4, 0, 0}, MSG_INFO_FRAME, TEXTFLAG_RIGHT_JUSTIFY, 0, info_layout},

			// Frame info field
			{OD_TEXT, TEXTPEN, {17, 4, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_INFO_FRAME, info_layout},

			// Anim info title
			{OD_TEXT, TEXTPEN, {16, 5, 0, 1}, {4, 4, 0, 0}, MSG_INFO_ANIM, TEXTFLAG_RIGHT_JUSTIFY, 0, info_layout},

			// Anim info field
			{OD_TEXT, TEXTPEN, {17, 5, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_INFO_ANIM, info_layout},

			{OD_END}},

	show_info_objects[] =
		{

			// File info title
			{OD_TEXT, TEXTPEN, {16, 0, 0, 1}, {4, 4, 0, 0}, MSG_INFO_FILE, TEXTFLAG_RIGHT_JUSTIFY, 0, info_layout},

			// File info field
			{OD_TEXT, TEXTPEN, {17, 0, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_INFO_FILE, info_layout},

			// Image size info title
			{OD_TEXT,
			 TEXTPEN,
			 {16, 1, 0, 1},
			 {4, 4, 0, 0},
			 MSG_INFO_IMAGE_SIZE,
			 TEXTFLAG_RIGHT_JUSTIFY,
			 0,
			 info_layout},

			// Image size info field
			{OD_TEXT, TEXTPEN, {17, 1, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_INFO_IMAGE_SIZE, info_layout},

			// Colours info title
			{OD_TEXT, TEXTPEN, {16, 2, 0, 1}, {4, 4, 0, 0}, MSG_INFO_COLOURS, TEXTFLAG_RIGHT_JUSTIFY, 0, info_layout},

			// Colours info field
			{OD_TEXT, TEXTPEN, {17, 2, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_INFO_COLOURS, info_layout},

			// Display mode info title
			{OD_TEXT, TEXTPEN, {16, 3, 0, 1}, {4, 4, 0, 0}, MSG_INFO_MODE, TEXTFLAG_RIGHT_JUSTIFY, 0, info_layout},

			// Display mode info field
			{OD_TEXT, TEXTPEN, {17, 3, 0, 1}, {4, 4, 0, 0}, 0, 0, GAD_INFO_MODE, info_layout},

			// Ok
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, POS_RIGHT_JUSTIFY, 14, 1},
			 {2, -2, 4, 6},
			 MSG_OK,
			 BUTTONFLAG_OKAY_BUTTON | BUTTONFLAG_CANCEL_BUTTON,
			 GAD_OK,
			 0},

			// Print
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 14, 1},
			 {-2, -2, 4, 6},
			 MSG_PRINT,
			 0,
			 GAD_PRINT,
			 0},

			{OD_END}},

	print_objects[] = {

		// Print area
		{OD_AREA,
		 0,
		 {0, POS_RIGHT_JUSTIFY - 1, SIZE_MAXIMUM, 4},
		 {2, -10, -2, 36},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_PRINT_AREA,
		 0},

		// Print aspect
		{OD_GADGET,
		 CYCLE_KIND,
		 {12, 0, 14, 1},
		 {4, 4, 24, 6},
		 MSG_PRINT_ASPECT,
		 0,
		 GAD_PRINT_ASPECT,
		 print_aspect_tags},

		// Print image
		{OD_GADGET, CYCLE_KIND, {12, 1, 14, 1}, {4, 11, 24, 6}, MSG_PRINT_IMAGE, 0, GAD_PRINT_IMAGE, print_image_tags},

		// Print shade
		{OD_GADGET, CYCLE_KIND, {12, 2, 14, 1}, {4, 18, 24, 6}, MSG_PRINT_SHADE, 0, GAD_PRINT_SHADE, print_shade_tags},

		// Print placement
		{OD_GADGET,
		 CYCLE_KIND,
		 {12, 3, 14, 1},
		 {4, 25, 24, 6},
		 MSG_PRINT_PLACEMENT,
		 0,
		 GAD_PRINT_PLACEMENT,
		 print_placement_tags},

		// Form feed
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {26, 1, 0, 1},
		 {32, 11, 26, 6},
		 MSG_PRINT_FORM_FEED,
		 PLACETEXT_RIGHT,
		 GAD_PRINT_FORM_FEED,
		 print_layout},

		// Print title
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {26, 2, 0, 1},
		 {32, 18, 26, 6},
		 MSG_PRINT_PRINT_TITLE,
		 PLACETEXT_RIGHT,
		 GAD_PRINT_PRINT_TITLE,
		 print_layout},

		{OD_END}};
