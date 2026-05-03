#include "config_lib.h"

// Buttons editor window dimensions
const ConfigWindow _config_buttons_window = {{POS_CENTER, POS_CENTER, 66, 11}, {0, 0, 90, 85}},

				   _config_buttons_clip_window = {{POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 14, 8}, {0, 0, 32, 16}};

// Labels
const UWORD

	_buttons_dragbar_labels[] = {MSG_BUTTONS_DRAG_AUTO,
								 MSG_BUTTONS_DRAG_TOP,
								 MSG_BUTTONS_DRAG_LEFT,
								 MSG_BUTTONS_DRAG_BOTTOM,
								 MSG_BUTTONS_DRAG_RIGHT,
								 MSG_BUTTONS_DRAG_NONE,
								 0};

// Tags
const struct TagItem _buttons_thin_tags[] = {{GTCustom_ThinBorders, TRUE}, {TAG_END}},

					 _buttons_relative_tags[] = {{GTCustom_LayoutRel, GAD_BUTTONS_LAYOUT_AREA}, {TAG_END}},

					 _buttons_bank_layout[] = {{GTCustom_LayoutRel, GAD_BUTTONS_BANK_EDITING},
											   {TAG_MORE, (IPTR)_buttons_thin_tags}},

					 _buttons_button_layout[] = {{GTCustom_LayoutRel, GAD_BUTTONS_BUTTON_EDITING},
												 {TAG_MORE, (IPTR)_buttons_thin_tags}},

					 _buttons_visual_layout[] = {{GTCustom_LayoutRel, GAD_BUTTONS_VISUAL_DISPLAY},
												 {TAG_MORE, (IPTR)_buttons_thin_tags}},

					 _buttons_name_tags[] = {{GTST_MaxChars, 31}, {TAG_MORE, (IPTR)_buttons_bank_layout}},

					 _buttons_fontname_tags[] = {{GTST_MaxChars, 50}, {TAG_MORE, (IPTR)_buttons_bank_layout}},

					 _buttons_fontsize_tags[] = {{GTIN_MaxChars, 2},
												 {STRINGA_Justification, GACT_STRINGCENTER},
												 {TAG_MORE, (IPTR)_buttons_bank_layout}},

					 _buttons_backpic_popup_tags[] = {{GTCustom_Control, GAD_BUTTONS_BACKPIC},
													  {TAG_MORE, (IPTR)_buttons_bank_layout}},

					 _buttons_backpic_tags[] = {{GTST_MaxChars, 256}, {TAG_MORE, (IPTR)_buttons_bank_layout}},

					 _clipboard_scroller_tags[] = {{GTSC_Total, 1},
												   {GA_Immediate, TRUE},
												   {GA_RelVerify, TRUE},
												   {GTSC_Arrows, 12},
												   {PGA_Freedom, LORIENT_VERT},
												   {TAG_END}},

					 _buttons_dragbar_orientation[] = {{GTCustom_LocaleLabels, (IPTR)_buttons_dragbar_labels},
													   {TAG_MORE, (IPTR)_buttons_visual_layout}};

// Objects
const ObjectDef _config_buttons_objects[] =
	{

		// Layout area
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {3, 3, -3, -12},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE | AREAFLAG_THIN,
		 GAD_BUTTONS_LAYOUT_AREA,
		 0},

		// Bank editing
		{OD_AREA,
		 TEXTPEN,
		 {0, 1, 45, 5},
		 {6, -2, 34, 45},
		 MSG_BUTTONS_BANK_EDITING,
		 AREAFLAG_TITLE,
		 GAD_BUTTONS_BANK_EDITING,
		 _buttons_relative_tags},

		// Name
		{OD_GADGET,
		 STRING_KIND,
		 {10, 0, SIZE_MAXIMUM, 1},
		 {4, 0, -6, 6},
		 MSG_BUTTONS_NAME,
		 PLACETEXT_LEFT,
		 GAD_BUTTONS_NAME,
		 _buttons_name_tags},

		// Columns display
		{OD_GADGET,
		 NUMBER_KIND,
		 {10, 1, 7, 1},
		 {4, 8, 4, 6},
		 MSG_BUTTONS_COLUMNS,
		 0,
		 GAD_BUTTONS_COLUMNS,
		 _buttons_bank_layout},

		// Columns add
		{OD_GADGET,
		 BUTTON_KIND,
		 {17, 1, 7, 1},
		 {8, 8, 4, 6},
		 MSG_BUTTONS_ADD,
		 0,
		 GAD_BUTTONS_COLUMNS_ADD,
		 _buttons_bank_layout},

		// Columns insert
		{OD_GADGET,
		 BUTTON_KIND,
		 {24, 1, 7, 1},
		 {12, 8, 4, 6},
		 MSG_BUTTONS_INSERT,
		 0,
		 GAD_BUTTONS_COLUMNS_INSERT,
		 _buttons_bank_layout},

		// Columns delete
		{OD_GADGET,
		 BUTTON_KIND,
		 {31, 1, 7, 1},
		 {16, 8, 4, 6},
		 MSG_BUTTONS_DELETE,
		 0,
		 GAD_BUTTONS_COLUMNS_DELETE,
		 _buttons_bank_layout},

		// Columns remove
		{OD_GADGET,
		 BUTTON_KIND,
		 {38, 1, 7, 1},
		 {20, 8, 4, 6},
		 MSG_BUTTONS_REMOVE,
		 0,
		 GAD_BUTTONS_COLUMNS_REMOVE,
		 _buttons_bank_layout},

		// Rows display
		{OD_GADGET,
		 NUMBER_KIND,
		 {10, 2, 7, 1},
		 {4, 16, 4, 6},
		 MSG_BUTTONS_ROWS,
		 0,
		 GAD_BUTTONS_ROWS,
		 _buttons_bank_layout},

		// Rows add
		{OD_GADGET,
		 BUTTON_KIND,
		 {17, 2, 7, 1},
		 {8, 16, 4, 6},
		 MSG_BUTTONS_ADD,
		 0,
		 GAD_BUTTONS_ROWS_ADD,
		 _buttons_bank_layout},

		// Rows insert
		{OD_GADGET,
		 BUTTON_KIND,
		 {24, 2, 7, 1},
		 {12, 16, 4, 6},
		 MSG_BUTTONS_INSERT,
		 0,
		 GAD_BUTTONS_ROWS_INSERT,
		 _buttons_bank_layout},

		// Rows delete
		{OD_GADGET,
		 BUTTON_KIND,
		 {31, 2, 7, 1},
		 {16, 16, 4, 6},
		 MSG_BUTTONS_DELETE,
		 0,
		 GAD_BUTTONS_ROWS_DELETE,
		 _buttons_bank_layout},

		// Rows remove
		{OD_GADGET,
		 BUTTON_KIND,
		 {38, 2, 7, 1},
		 {20, 16, 4, 6},
		 MSG_BUTTONS_REMOVE,
		 0,
		 GAD_BUTTONS_ROWS_REMOVE,
		 _buttons_bank_layout},

		// Bank font glass
		{OD_GADGET,
		 FILE_BUTTON_KIND,
		 {10, 3, 0, 1},
		 {4, 24, 28, 6},
		 MSG_BUTTONS_FONT,
		 0,
		 GAD_BUTTONS_FONT_GLASS,
		 _buttons_bank_layout},

		// Font field
		{OD_GADGET,
		 STRING_KIND,
		 {10, 3, SIZE_MAX_LESS - 4, 1},
		 {32, 24, -18, 6},
		 0,
		 0,
		 GAD_BUTTONS_FONT_NAME,
		 _buttons_fontname_tags},

		// Font size
		{OD_GADGET,
		 INTEGER_KIND,
		 {POS_RIGHT_JUSTIFY, 3, 4, 1},
		 {-6, 24, 12, 6},
		 0,
		 0,
		 GAD_BUTTONS_FONT_SIZE,
		 _buttons_fontsize_tags},

		// Background picture popup
		{OD_GADGET,
		 FILE_BUTTON_KIND,
		 {10, 4, 0, 1},
		 {4, 32, 28, 6},
		 MSG_BUTTONS_BACKPIC,
		 0,
		 GAD_BUTTONS_BACKPIC_POPUP,
		 _buttons_backpic_popup_tags},

		// Background picture
		{OD_GADGET,
		 STRING_KIND,
		 {10, 4, SIZE_MAX_LESS, 1},
		 {32, 32, -6, 6},
		 0,
		 0,
		 GAD_BUTTONS_BACKPIC,
		 _buttons_backpic_tags},

		/****************************************************************/

		// Button editing
		{OD_AREA,
		 TEXTPEN,
		 {0, 7, 45, SIZE_MAXIMUM},
		 {6, 46, 34, -6},
		 MSG_BUTTONS_BUTTON_EDITING,
		 AREAFLAG_TITLE,
		 GAD_BUTTONS_BUTTON_EDITING,
		 _buttons_relative_tags},

		// Button edit
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, 0, 11, 1},
		 {6, 2, 4, 6},
		 MSG_BUTTONS_EDIT,
		 0,
		 GAD_BUTTONS_EDIT,
		 _buttons_button_layout},

		// Button copy
		{OD_GADGET,
		 BUTTON_KIND,
		 {11, 0, 11, 1},
		 {13, 2, 4, 6},
		 MSG_BUTTONS_COPY,
		 0,
		 GAD_BUTTONS_COPY,
		 _buttons_button_layout},

		// Button cut
		{OD_GADGET,
		 BUTTON_KIND,
		 {22, 0, 11, 1},
		 {20, 2, 4, 6},
		 MSG_BUTTONS_CUT,
		 0,
		 GAD_BUTTONS_CUT,
		 _buttons_button_layout},

		// Button erase
		{OD_GADGET,
		 BUTTON_KIND,
		 {33, 0, 11, 1},
		 {27, 2, 4, 6},
		 MSG_BUTTONS_ERASE,
		 0,
		 GAD_BUTTONS_ERASE,
		 _buttons_button_layout},

		// Paint mode
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 1, 0, 1},
		 {6, 11, 28, 6},
		 MSG_BUTTONS_PAINT_MODE,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_PAINT_MODE,
		 _buttons_button_layout},

		// Clipboard
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {22, 1, 0, 1},
		 {20, 11, 28, 6},
		 MSG_BUTTONS_SHOW_CLIPBOARD,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_SHOW_CLIPBOARD,
		 _buttons_button_layout},

		/****************************************************************/

		// Visual appearance
		{OD_AREA,
		 TEXTPEN,
		 {46, 1, 20, SIZE_MAXIMUM},
		 {40, -2, 34, -6},
		 MSG_BUTTONS_SETTINGS,
		 AREAFLAG_TITLE,
		 GAD_BUTTONS_VISUAL_DISPLAY,
		 _buttons_relative_tags},

		// Full window border
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 0, 0, 1},
		 {6, 2, 26, 4},
		 MSG_BUTTONS_BANK_BORDER,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_BANK_BORDER,
		 _buttons_visual_layout},

		// Dragbar orientation
		{OD_GADGET,
		 CYCLE_KIND,
		 {8, 1, 10, 1},
		 {6, 8, 24, 6},
		 MSG_BUTTONS_DRAG,
		 PLACETEXT_LEFT,
		 GAD_BUTTONS_DRAGBAR_ORIENTATION,
		 _buttons_dragbar_orientation},

		// Borderless buttons
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 2, 0, 1},
		 {6, 16, 26, 4},
		 MSG_BUTTONS_BORDERLESS,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_BORDERLESS,
		 _buttons_visual_layout},

		// Simple-refresh window
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 3, 0, 1},
		 {6, 22, 26, 4},
		 MSG_BUTTONS_SIMPLE_REFRESH,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_SIMPLE_REFRESH,
		 _buttons_visual_layout},

		// No dog-ears
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 4, 0, 1},
		 {6, 28, 26, 4},
		 MSG_BUTTONS_NO_DOGEARS,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_NO_DOGEARS,
		 _buttons_visual_layout},

		// Auto close
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 5, 0, 1},
		 {6, 34, 26, 4},
		 MSG_BUTTONS_AUTO_CLOSE,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_AUTO_CLOSE,
		 _buttons_visual_layout},

		// Auto iconify
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 6, 0, 1},
		 {6, 40, 26, 4},
		 MSG_BUTTONS_AUTO_ICONIFY,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_AUTO_ICONIFY,
		 _buttons_visual_layout},

		// Active popups
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {0, 7, 0, 1},
		 {6, 46, 26, 4},
		 MSG_BUTTONS_ACTIVE_POPUPS,
		 PLACETEXT_RIGHT,
		 GAD_BUTTONS_ACTIVE_POPUP,
		 _buttons_visual_layout},

		/****************************************************************/

		// Save
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 14, 1},
		 {3, -3, 4, 6},
		 MSG_SAVE,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_BUTTONS_SAVE,
		 _buttons_thin_tags},

		// Use
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_CENTER, POS_RIGHT_JUSTIFY, 14, 1},
		 {0, -3, 4, 6},
		 MSG_USE,
		 0,
		 GAD_BUTTONS_OK,
		 _buttons_thin_tags},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 14, 1},
		 {-3, -3, 4, 6},
		 MSG_CANCEL,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_BUTTONS_CANCEL,
		 _buttons_thin_tags},

		{OD_END}},

				_config_buttons_clip_objects[] = {

					// Clipboard area
					{OD_AREA,
					 0,
					 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
					 {3, 3, -3, -12},
					 0,
					 AREAFLAG_ERASE,
					 GAD_BUTTONS_CLIPBOARD,
					 0},

					// Clipboard
					{OD_AREA,
					 0,
					 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
					 {3, 3, -19, -12},
					 0,
					 AREAFLAG_RAISED | AREAFLAG_ERASE | AREAFLAG_THIN,
					 GAD_BUTTONS_CLIPBOARD_AREA,
					 0},

					// Clipboard scroller
					{OD_GADGET,
					 SCROLLER_KIND,
					 {POS_RIGHT_JUSTIFY, 0, 0, SIZE_MAX_LESS - 1},
					 {-3, 3, 16, -12},
					 0,
					 0,
					 GAD_BUTTONS_CLIPBOARD_SCROLLER,
					 _clipboard_scroller_tags},

					// Clipboard clear
					{OD_GADGET,
					 BUTTON_KIND,
					 {0, POS_RIGHT_JUSTIFY, SIZE_MAXIMUM, 1},
					 {3, -3, -3, 6},
					 MSG_BUTTONS_CLIPBOARD_CLEAR,
					 0,
					 GAD_BUTTONS_CLIPBOARD_CLEAR,
					 _buttons_thin_tags},

					{OD_END}};

// Menu for button banks
const MenuData
	button_toolbar_menu[] =
		{{NM_TITLE, 0, MSG_PROJECT, 0},
		 {NM_ITEM, BUTTONEDIT_MENU_NEW, MSG_LISTER_BUTTONS_MENU_NEW, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('N')},
		 {NM_ITEM, BUTTONEDIT_MENU_OPEN, MSG_LISTER_BUTTONS_MENU_OPEN, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('O')},
		 {NM_ITEM, 0, NM_BAR_LABEL, 0},
		 {NM_ITEM, BUTTONEDIT_MENU_SAVE, MSG_LISTER_BUTTONS_MENU_SAVE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('S')},
		 {NM_ITEM, BUTTONEDIT_MENU_SAVEAS, MSG_LISTER_BUTTONS_MENU_SAVEAS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('A')},
		 {NM_ITEM, 0, NM_BAR_LABEL, 0},
		 {NM_ITEM, BUTTONEDIT_MENU_CLOSE, MSG_LISTER_BUTTONS_MENU_QUIT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('Q')},
		 {NM_TITLE, 0, MSG_EDIT, 0},
		 {NM_ITEM, BUTTONEDIT_MENU_DEFAULTS, MSG_RESET_DEFAULTS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('D')},
		 {NM_ITEM, BUTTONEDIT_MENU_LASTSAVED, MSG_LAST_SAVED, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('L')},
		 {NM_ITEM, BUTTONEDIT_MENU_RESTORE, MSG_RESTORE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('R')},
		 {NM_END}},

	button_menu[] = {
		{NM_TITLE, 0, MSG_PROJECT, 0},
		{NM_ITEM, BUTTONEDIT_MENU_NEW, MSG_LISTER_BUTTONS_MENU_NEW, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('N')},
		{NM_ITEM, BUTTONEDIT_MENU_OPEN, MSG_LISTER_BUTTONS_MENU_OPEN, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('O')},
		{NM_ITEM, 0, NM_BAR_LABEL, 0},
		{NM_ITEM, BUTTONEDIT_MENU_SAVE, MSG_LISTER_BUTTONS_MENU_SAVE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('S')},
		{NM_ITEM, BUTTONEDIT_MENU_SAVEAS, MSG_LISTER_BUTTONS_MENU_SAVEAS, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('A')},
		{NM_ITEM, 0, NM_BAR_LABEL, 0},
		{NM_ITEM, BUTTONEDIT_MENU_CLOSE, MSG_LISTER_BUTTONS_MENU_QUIT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('Q')},
		{NM_END}};
