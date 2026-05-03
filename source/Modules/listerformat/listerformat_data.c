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

#include "listerformat.h"

ModuleInfo module_info = {1, "listerformat.module", "listerformat.catalog", 0, 0, {{0}}};

// Display window dimensions
ConfigWindow listformat_window = {{POS_CENTER, POS_CENTER, 32, 12}, {0, 0, 74, 78}},

			 listformat_window_ftp = {{POS_CENTER, POS_CENTER, 32, 12}, {0, 0, 74, 78 + 4}},

			 listformat_window_ftp_big = {{POS_CENTER, POS_CENTER, 32, 13}, {0, 0, 74, 85 + 7}},

			 listformat_window_big = {{POS_CENTER, POS_CENTER, 32, 13}, {0, 0, 74, 85}};

// Labels
UWORD
lister_separation_labels[] = {MSG_LISTER_SEPARATION_MIX, MSG_LISTER_SEPARATION_DIRS, MSG_LISTER_SEPARATION_FILES, 0};

// Tags for gadgets
struct TagItem

	// Relative to area tags[]={
	lister_relative_taglist[] = {{GTCustom_LayoutRel, GAD_LISTER_LAYOUT}, {TAG_END, 0}},

	// Selected display items lister
	lister_selitems_taglist[] = {{DLV_ShowSelected, 0},
								 {DLV_Check, 1},
								 {DLV_DragNotify, 2},
								 {TAG_MORE, (IPTR)lister_relative_taglist}},

	// Available display items lister
	lister_items_taglist[] = {{DLV_DragNotify, 2}, {TAG_MORE, (IPTR)lister_relative_taglist}},

	// Filter
	lister_filter_taglist[] = {{GTST_MaxChars, 39}, {TAG_MORE, (IPTR)lister_relative_taglist}},

	// Separation method
	lister_separation_taglist[] = {{GTCustom_LocaleLabels, (IPTR)lister_separation_labels},
								   {TAG_MORE, (IPTR)lister_relative_taglist}};

// Lister window gadgets
ObjectDef listformat_objects[] =
	{

		// Edit area
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {2, 2, -2, -10},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_LISTER_LAYOUT,
		 0},

		// Selected display items lister
		{OD_GADGET,
		 MY_LISTVIEW_KIND,
		 {14, 1, SIZE_MAXIMUM, 4},
		 {33, 6, -5, 22},
		 MSG_LISTER_SELECTED_DISPLAY_ITEMS,
		 PLACETEXT_ABOVE,
		 GAD_LISTER_FORMAT_SELITEMS,
		 lister_selitems_taglist},

		// Available display items lister
		{OD_GADGET,
		 MY_LISTVIEW_KIND,
		 {0, 1, 14, 4},
		 {5, 6, 20, 22},
		 MSG_LISTER_AVAILABLE_DISPLAY_ITEMS,
		 PLACETEXT_ABOVE,
		 GAD_LISTER_FORMAT_ITEMS,
		 lister_items_taglist},

		// Entry separation
		{OD_GADGET,
		 CYCLE_KIND,
		 {14, 5, SIZE_MAXIMUM, 1},
		 {33, 30, -5, 6},
		 MSG_LISTER_ENTRY_SEPARATION,
		 PLACETEXT_LEFT,
		 GAD_LISTER_FORMAT_SEPARATION,
		 lister_separation_taglist},

		// Reverse sorting
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {14, 6, 0, 1},
		 {33, 37, 26, 4},
		 MSG_LISTER_REVERSE_SORTING,
		 PLACETEXT_LEFT,
		 GAD_LISTER_FORMAT_REVERSE,
		 lister_relative_taglist},

		// Reject icons
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {14, 7, 0, 1},
		 {33, 42, 26, 4},
		 MSG_LISTER_REJECT_ICONS,
		 PLACETEXT_LEFT,
		 GAD_LISTER_REJECT_ICONS,
		 lister_relative_taglist},

		// Hidden bit
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {POS_RIGHT_JUSTIFY, 7, 0, 1},
		 {-5, 42, 26, 4},
		 MSG_LISTER_HIDDEN_BIT,
		 PLACETEXT_LEFT,
		 GAD_LISTER_HIDDEN_BIT,
		 lister_relative_taglist},

		// Fuel gauge
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {14, 8, 0, 1},
		 {33, 47, 26, 4},
		 MSG_LISTER_FUEL_GAUGE,
		 PLACETEXT_LEFT,
		 GAD_LISTER_FUEL_GAUGE,
		 lister_relative_taglist},

		// Inherit
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {POS_RIGHT_JUSTIFY, 8, 0, 1},
		 {-5, 47, 26, 4},
		 MSG_LISTER_INHERIT,
		 PLACETEXT_LEFT,
		 GAD_LISTER_INHERIT,
		 lister_relative_taglist},

		// Show filter
		{OD_GADGET,
		 STRING_KIND,
		 {14, 9, SIZE_MAXIMUM, 1},
		 {33, 52, -5, 4},
		 MSG_LISTER_SHOW,
		 PLACETEXT_LEFT,
		 GAD_LISTER_SHOW,
		 lister_filter_taglist},

		// Hide filter
		{OD_GADGET,
		 STRING_KIND,
		 {14, 10, SIZE_MAXIMUM, 1},
		 {33, 57, -5, 4},
		 MSG_LISTER_HIDE,
		 PLACETEXT_LEFT,
		 GAD_LISTER_HIDE,
		 lister_filter_taglist},

		{OD_END}},

		  listformat_nosave_objects[] =
			  {

				  // Use
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, POS_RIGHT_JUSTIFY, 12, 1},
				   {2, -2, 12, 6},
				   MSG_LISTER_USE,
				   BUTTONFLAG_OKAY_BUTTON,
				   GAD_LISTER_USE,
				   0},

				  // Cancel
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
				   {-2, -2, 12, 6},
				   MSG_LISTER_CANCEL,
				   BUTTONFLAG_CANCEL_BUTTON,
				   GAD_LISTER_CANCEL,
				   0},

				  {OD_END}},

		  listformat_save_objects[] =
			  {

				  // Save
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, POS_RIGHT_JUSTIFY, 10, 1},
				   {2, -2, 12, 6},
				   MSG_LISTER_SAVE,
				   0,
				   GAD_LISTER_SAVE,
				   0},

				  // Use
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_CENTER, POS_RIGHT_JUSTIFY, 10, 1},
				   {2, -2, 12, 6},
				   MSG_LISTER_USE,
				   BUTTONFLAG_OKAY_BUTTON,
				   GAD_LISTER_USE,
				   0},

				  // Cancel
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 10, 1},
				   {-2, -2, 12, 6},
				   MSG_LISTER_CANCEL,
				   BUTTONFLAG_CANCEL_BUTTON,
				   GAD_LISTER_CANCEL,
				   0},

				  // Default button
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, POS_RIGHT_JUSTIFY, 18, 1},
				   {5, -2, 8, 6},
				   MSG_DEFAULTS,
				   0,
				   GAD_DEFAULTS,
				   lister_relative_taglist},

				  // Set as defaults
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 18, 1},
				   {-5, -2, 8, 6},
				   MSG_SET_AS_DEFAULTS,
				   0,
				   GAD_SET_AS_DEFAULTS,
				   lister_relative_taglist},

				  {OD_END}},

		  listformat_ftp_objects[] =
			  {

				  // Edit area
				  {OD_AREA,
				   0,
				   {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
				   {2, 2, -2, -10},
				   0,
				   AREAFLAG_RECESSED | AREAFLAG_ERASE,
				   GAD_LISTER_LAYOUT,
				   0},

				  // Selected display items lister
				  {OD_GADGET,
				   MY_LISTVIEW_KIND,
				   {14, 1, SIZE_MAXIMUM, 4},
				   {33, 6, -5, 22},
				   MSG_LISTER_SELECTED_DISPLAY_ITEMS,
				   PLACETEXT_ABOVE,
				   GAD_LISTER_FORMAT_SELITEMS,
				   lister_selitems_taglist},

				  // Available display items lister
				  {OD_GADGET,
				   MY_LISTVIEW_KIND,
				   {0, 1, 14, 4},
				   {5, 6, 20, 22},
				   MSG_LISTER_AVAILABLE_DISPLAY_ITEMS,
				   PLACETEXT_ABOVE,
				   GAD_LISTER_FORMAT_ITEMS,
				   lister_items_taglist},

				  // Entry separation
				  {OD_GADGET,
				   CYCLE_KIND,
				   {14, 5, SIZE_MAXIMUM, 1},
				   {33, 30, -5, 6},
				   MSG_LISTER_ENTRY_SEPARATION,
				   PLACETEXT_LEFT,
				   GAD_LISTER_FORMAT_SEPARATION,
				   lister_separation_taglist},

				  // Reverse sorting
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {14, 6, 0, 1},
				   {33, 37, 26, 4},
				   MSG_LISTER_REVERSE_SORTING,
				   PLACETEXT_LEFT,
				   GAD_LISTER_FORMAT_REVERSE,
				   lister_relative_taglist},

				  // Reject icons
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {14, 7, 0, 1},
				   {33, 42, 26, 4},
				   MSG_LISTER_REJECT_ICONS,
				   PLACETEXT_LEFT,
				   GAD_LISTER_REJECT_ICONS,
				   lister_relative_taglist},

				  // Hidden bit
				  {OD_GADGET,
				   CHECKBOX_KIND,
				   {POS_RIGHT_JUSTIFY, 7, 0, 1},
				   {-5, 42, 26, 4},
				   MSG_LISTER_HIDDEN_BIT,
				   PLACETEXT_LEFT,
				   GAD_LISTER_HIDDEN_BIT,
				   lister_relative_taglist},

				  // Show filter
				  {OD_GADGET,
				   STRING_KIND,
				   {14, 8, SIZE_MAXIMUM, 1},
				   {33, 47, -5, 4},
				   MSG_LISTER_SHOW,
				   PLACETEXT_LEFT,
				   GAD_LISTER_SHOW,
				   lister_filter_taglist},

				  // Hide filter
				  {OD_GADGET,
				   STRING_KIND,
				   {14, 9, SIZE_MAXIMUM, 1},
				   {33, 52, -5, 4},
				   MSG_LISTER_HIDE,
				   PLACETEXT_LEFT,
				   GAD_LISTER_HIDE,
				   lister_filter_taglist},

				  {OD_END}},

		  listformat_ftp_use_objects_big[] =
			  {

				  // Use
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, POS_RIGHT_JUSTIFY, 10, 1},
				   {2, -2, 12, 6},
				   MSG_LISTER_USE,
				   BUTTONFLAG_OKAY_BUTTON,
				   GAD_LISTER_USE,
				   0},

				  // Cancel
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 10, 1},
				   {-2, -2, 12, 6},
				   MSG_LISTER_CANCEL,
				   BUTTONFLAG_CANCEL_BUTTON,
				   GAD_LISTER_CANCEL,
				   0},

				  // Default button
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_CENTER, POS_RIGHT_JUSTIFY - 1, 22, 1},
				   {0, -10, 8, 6},
				   MSG_FTP_DEFAULTS,
				   0,
				   GAD_FTP_DEFAULTS,
				   lister_relative_taglist},

				  // Default button
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_CENTER, POS_RIGHT_JUSTIFY, 22, 1},
				   {0, -2, 8, 6},
				   MSG_DEFAULTS,
				   0,
				   GAD_DEFAULTS,
				   lister_relative_taglist},

				  {OD_END}},

		  listformat_ftp_use_objects[] = {

			  // Use
			  {OD_GADGET,
			   BUTTON_KIND,
			   {0, POS_RIGHT_JUSTIFY, 10, 1},
			   {2, -2, 12, 6},
			   MSG_LISTER_USE,
			   BUTTONFLAG_OKAY_BUTTON,
			   GAD_LISTER_USE,
			   0},

			  // Cancel
			  {OD_GADGET,
			   BUTTON_KIND,
			   {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 10, 1},
			   {-2, -2, 12, 6},
			   MSG_LISTER_CANCEL,
			   BUTTONFLAG_CANCEL_BUTTON,
			   GAD_LISTER_CANCEL,
			   0},

			  // Default button
			  {OD_GADGET,
			   BUTTON_KIND,
			   {POS_CENTER, POS_RIGHT_JUSTIFY, 22, 1},
			   {0, -2, 8, 6},
			   MSG_DEFAULTS,
			   0,
			   GAD_DEFAULTS,
			   lister_relative_taglist},

			  {OD_END}};
