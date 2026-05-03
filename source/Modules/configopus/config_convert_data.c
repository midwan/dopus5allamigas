#include "config_lib.h"

const ConfigWindow _config_convert_window = {{POS_CENTER, POS_CENTER, 40, 11}, {0, 0, 16, 78}};

const struct TagItem _convert_layout_taglist[] = {{GTCustom_LayoutRel, GAD_CONVERT_LAYOUT}, {TAG_END}},

					 _convert_basename_taglist[] = {{GTST_MaxChars, 20},
													{GTCustom_LayoutRel, GAD_CONVERT_LAYOUT},
													{TAG_END}};

const ObjectDef _config_convert_objects[] = {

	// Layout box
	{OD_AREA, 0, {0, 0, 40, 10}, {4, 4, 8, 60}, 0, AREAFLAG_RECESSED | AREAFLAG_ERASE, GAD_CONVERT_LAYOUT, 0},

	// Title text
	{OD_TEXT,
	 TEXTPEN,
	 {0, 0, SIZE_MAXIMUM, 1},
	 {0, 4, 0, 0},
	 MSG_CONVERT_TITLE_1,
	 TEXTFLAG_CENTER,
	 0,
	 _convert_layout_taglist},

	// Title text
	{OD_TEXT,
	 TEXTPEN,
	 {0, 1, SIZE_MAXIMUM, 1},
	 {0, 4, 0, 0},
	 MSG_CONVERT_TITLE_2,
	 TEXTFLAG_CENTER,
	 0,
	 _convert_layout_taglist},

	// Environment
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {12, 2, 0, 1},
	 {0, 8, 26, 4},
	 MSG_CONVERT_ENVIRONMENT,
	 PLACETEXT_RIGHT,
	 GAD_CONVERT_ENVIRONMENT,
	 _convert_layout_taglist},

	// Buttons
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {12, 4, 0, 1},
	 {0, 20, 26, 4},
	 MSG_CONVERT_BUTTONS,
	 PLACETEXT_RIGHT,
	 GAD_CONVERT_BUTTONS,
	 _convert_layout_taglist},

	// Menus
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {12, 5, 0, 1},
	 {0, 26, 26, 4},
	 MSG_CONVERT_MENUS,
	 PLACETEXT_RIGHT,
	 GAD_CONVERT_MENUS,
	 _convert_layout_taglist},

	// Drives
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {12, 6, 0, 1},
	 {0, 32, 26, 4},
	 MSG_CONVERT_DRIVES,
	 PLACETEXT_RIGHT,
	 GAD_CONVERT_DRIVES,
	 _convert_layout_taglist},

	// Filetypes
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {12, 7, 0, 1},
	 {0, 38, 26, 4},
	 MSG_CONVERT_FILETYPES,
	 PLACETEXT_RIGHT,
	 GAD_CONVERT_FILETYPES,
	 _convert_layout_taglist},

	// Hotkeys
	{OD_GADGET,
	 CHECKBOX_KIND,
	 {12, 8, 0, 1},
	 {0, 44, 26, 4},
	 MSG_CONVERT_HOTKEYS,
	 PLACETEXT_RIGHT,
	 GAD_CONVERT_HOTKEYS,
	 _convert_layout_taglist},

	// Basename
	{OD_GADGET,
	 STRING_KIND,
	 {12, POS_RIGHT_JUSTIFY, SIZE_MAX_LESS - 2, 1},
	 {0, -3, 0, 4},
	 MSG_CONVERT_BASENAME,
	 PLACETEXT_LEFT,
	 GAD_CONVERT_BASENAME,
	 _convert_basename_taglist},

	// Convert
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {4, -4, 8, 6},
	 MSG_CONVERT_CONVERT,
	 BUTTONFLAG_OKAY_BUTTON,
	 GAD_CONVERT_CONVERT,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-4, -4, 8, 6},
	 MSG_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_CONVERT_CANCEL,
	 0},

	{OD_END}};
