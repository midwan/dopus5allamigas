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

#include "print.h"

ModuleInfo module_info = {1, "print.module", "print.catalog", 0, 1, {{0, "Print", MSG_PRINT_DESC, FUNCF_NO_ARGS, 0}}};

ConfigWindow print_window = {{POS_CENTER, POS_CENTER, 40, 11}, {0, 0, 74, 95}};

UWORD
print_quality_labels[] = {MSG_PRINT_QUALITY_DRAFT, MSG_PRINT_QUALITY_LETTER, 0},

	print_spacing_labels[] = {MSG_PRINT_SPACING_6, MSG_PRINT_SPACING_8, 0},

	print_pitch_labels[] = {MSG_PRINT_PITCH_PICA, MSG_PRINT_PITCH_ELITE, MSG_PRINT_PITCH_FINE, 0},

	print_header_footer_labels[] = {MSG_PRINT_HEADER, MSG_PRINT_FOOTER, 0},

	print_style_labels[] = {MSG_PRINT_STYLE_NORMAL,
							MSG_PRINT_STYLE_BOLD,
							MSG_PRINT_STYLE_ITALICS,
							MSG_PRINT_STYLE_UNDERLINE,
							0},

	print_output_labels[] = {MSG_PRINT_OUTPUT_PRINTER, MSG_PRINT_OUTPUT_FILE, 0};

static struct TagItem
	print_layout[] = {{GTCustom_LayoutRel, GAD_PRINT_LAYOUT}, {TAG_END}},

	text_layout[] = {{GTCustom_LayoutRel, GAD_PRINT_TEXT_LAYOUT}, {TAG_END}},

	header_layout[] = {{GTCustom_LayoutRel, GAD_PRINT_HEADER_LAYOUT}, {TAG_END}},

	print_quality[] = {{GTCustom_LocaleLabels, (IPTR)print_quality_labels}, {TAG_MORE, (IPTR)text_layout}},

	print_spacing[] = {{GTCustom_LocaleLabels, (IPTR)print_spacing_labels}, {TAG_MORE, (IPTR)text_layout}},

	print_pitch[] = {{GTCustom_LocaleLabels, (IPTR)print_pitch_labels}, {TAG_MORE, (IPTR)text_layout}},

	print_margin[] = {{GTIN_MaxChars, 3}, {TAG_MORE, (IPTR)text_layout}},

	print_tab_size[] = {{GTIN_MaxChars, 1}, {TAG_MORE, (IPTR)text_layout}},

	print_output[] = {{GTCustom_LocaleLabels, (IPTR)print_output_labels}, {TAG_MORE, (IPTR)text_layout}},

	print_header_footer_tags[] = {{GTCustom_LocaleLabels, (IPTR)print_header_footer_labels},
								  {TAG_MORE, (IPTR)header_layout}},

	print_title[] = {{GTST_MaxChars, 40}, {TAG_MORE, (IPTR)header_layout}},

	print_style[] = {{GTCustom_LocaleLabels, (IPTR)print_style_labels}, {TAG_MORE, (IPTR)header_layout}};

ObjectDef print_objects[] = {

	// Print layout area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {3, 3, -3, -12},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_PRINT_LAYOUT,
	 0},

	// Text area
	{OD_AREA,
	 TEXTPEN,
	 {0, 1, SIZE_MAXIMUM, 4},
	 {6, -2, -6, 34},
	 MSG_TEXT,
	 AREAFLAG_TITLE,
	 GAD_PRINT_TEXT_LAYOUT,
	 print_layout},

	// Quality
	{OD_GADGET,
	 CYCLE_KIND,
	 {10, 0, 10, 1},
	 {8, 0, 24, 6},
	 MSG_PRINT_QUALITY,
	 PLACETEXT_LEFT,
	 GAD_PRINT_TEXT_QUALITY,
	 print_quality},

	// Spacing
	{OD_GADGET,
	 CYCLE_KIND,
	 {10, 1, 10, 1},
	 {8, 7, 24, 6},
	 MSG_PRINT_SPACING,
	 PLACETEXT_LEFT,
	 GAD_PRINT_TEXT_SPACING,
	 print_spacing},

	// Pitch
	{OD_GADGET,
	 CYCLE_KIND,
	 {10, 2, 10, 1},
	 {8, 14, 24, 6},
	 MSG_PRINT_PITCH,
	 PLACETEXT_LEFT,
	 GAD_PRINT_TEXT_PITCH,
	 print_pitch},

	// Output
	{OD_GADGET,
	 CYCLE_KIND,
	 {10, 3, 10, 1},
	 {8, 21, 24, 6},
	 MSG_PRINT_OUTPUT,
	 PLACETEXT_LEFT,
	 GAD_PRINT_OUTPUT,
	 print_output},

	// Left margin
	{OD_GADGET,
	 INTEGER_KIND,
	 {36, 0, 4, 1},
	 {32, 0, 12, 6},
	 MSG_PRINT_LEFT_MARGIN,
	 PLACETEXT_LEFT,
	 GAD_PRINT_LEFT_MARGIN,
	 print_margin},

	// Right margin
	{OD_GADGET,
	 INTEGER_KIND,
	 {36, 1, 4, 1},
	 {32, 7, 12, 6},
	 MSG_PRINT_RIGHT_MARGIN,
	 PLACETEXT_LEFT,
	 GAD_PRINT_RIGHT_MARGIN,
	 print_margin},

	// Page length
	{OD_GADGET,
	 INTEGER_KIND,
	 {36, 2, 4, 1},
	 {32, 14, 12, 6},
	 MSG_PRINT_PAGE_LENGTH,
	 PLACETEXT_LEFT,
	 GAD_PRINT_PAGE_LENGTH,
	 print_margin},

	// Tab size
	{OD_GADGET,
	 INTEGER_KIND,
	 {36, 3, 4, 1},
	 {32, 21, 12, 6},
	 MSG_PRINT_TAB_SIZE,
	 PLACETEXT_LEFT,
	 GAD_PRINT_TAB_SIZE,
	 print_tab_size},

	// Header/footer area
	{OD_AREA,
	 TEXTPEN,
	 {0, POS_RIGHT_JUSTIFY, SIZE_MAXIMUM, 4},
	 {6, -4, -6, 31},
	 MSG_HEADER_FOOTER,
	 AREAFLAG_TITLE,
	 GAD_PRINT_HEADER_LAYOUT,
	 print_layout},

	// Header/footer cycle
	{OD_GADGET,
	 CYCLE_KIND,
	 {0, 0, 10, 1},
	 {8, 2, 24, 6},
	 MSG_PRINT_CONFIGURATION,
	 PLACETEXT_RIGHT,
	 GAD_PRINT_HEADER_FOOTER,
	 print_header_footer_tags},

	// Title
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {0, 1, 0, 1},
	 {36, 10, 26, 4},
	 MSG_PRINT_TITLE,
	 PLACETEXT_RIGHT,
	 GAD_PRINT_TITLE,
	 header_layout},

	// Title string
	{OD_GADGET, STRING_KIND, {8, 1, SIZE_MAXIMUM, 1}, {62, 10, -4, 4}, 0, 0, GAD_PRINT_TITLE_STRING, print_title},

	// Date
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {0, 2, 0, 1},
	 {36, 15, 26, 4},
	 MSG_PRINT_DATE,
	 PLACETEXT_RIGHT,
	 GAD_PRINT_DATE,
	 header_layout},

	// Page no.
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {0, 3, 0, 1},
	 {36, 20, 26, 4},
	 MSG_PRINT_PAGE,
	 PLACETEXT_RIGHT,
	 GAD_PRINT_PAGE,
	 header_layout},

	// Style
	{OD_GADGET,
	 CYCLE_KIND,
	 {POS_RIGHT_JUSTIFY, 3, 10, 1},
	 {-4, 18, 24, 6},
	 MSG_PRINT_STYLE,
	 PLACETEXT_LEFT,
	 GAD_PRINT_STYLE,
	 print_style},

	// Okay
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {3, -3, 4, 6},
	 MSG_PRINT,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_PRINT_OKAY,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-3, -3, 4, 6},
	 MSG_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_PRINT_CANCEL,
	 0},

	{OD_END}};

// Print styles
char *print_styles[] = {"\x1b[1m",	 // bold
						"\x1b[3m",	 // italics
						"\x1b[4m"};	 // underline
