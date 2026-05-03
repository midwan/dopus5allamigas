#include "config_lib.h"

// Settings window dimensions
ConfigWindow _config_settings_window = {{POS_CENTER, POS_CENTER, 58, 12}, {0, 0, 38, 41}};

// Labels
UWORD

_settings_dateformat_labels[] = {MSG_SETTINGS_DATEFORMAT_DDMMMYY,
								 MSG_SETTINGS_DATEFORMAT_YYMMDD,
								 MSG_SETTINGS_DATEFORMAT_MMDDYY,
								 MSG_SETTINGS_DATEFORMAT_DDMMYY,
								 0},

	_settings_hidemethod_labels[] = {MSG_SETTINGS_SUB_HIDE_CLOCK,
									 MSG_SETTINGS_SUB_HIDE_NONE,
									 MSG_SETTINGS_SUB_HIDE_ICON,
									 MSG_SETTINGS_SUB_HIDE_MENU,
									 0};

// Tags for gadgets
struct TagItem

	// Option lister
	_settings_lister_taglist[] = {{DLV_ShowSelected, 0}, {DLV_NoScroller, 1}, {DLV_TopJustify, 1}, {TAG_END, 0}},

	_settings_relative_taglist[] = {{GTCustom_LayoutRel, GAD_SETTINGS_EDIT_AREA}, {TAG_END}},

	// Maximum number of cached directories
	_settings_caching_buffers_taglist[] = {{GTIN_MaxChars, 5}, {TAG_MORE, (IPTR)&_settings_relative_taglist}},

	// Disable caching
	_settings_disable_caching_taglist[] = {{GTCustom_Control, GAD_SETTINGS_CACHING_MAX_BUFFERS},
										   {TAG_MORE, (IPTR)&_settings_relative_taglist}},

	// Date format
	_settings_dateformat_taglist[] = {{GTCustom_LocaleLabels, (IPTR)_settings_dateformat_labels},
									  {TAG_MORE, (IPTR)&_settings_relative_taglist}},

	// Hide method
	_settings_hide_taglist[] = {{GTCustom_LocaleLabels, (IPTR)_settings_hidemethod_labels},
								{TAG_MORE, (IPTR)&_settings_relative_taglist}},

	// Popkey
	_settings_popkey_taglist[] = {{GTST_MaxChars, 80}, {TAG_MORE, (IPTR)&_settings_relative_taglist}},

	// Priority
	_settings_priority_taglist[] = {{GTIN_MaxChars, 4},
									{GTCustom_MinMax, (127 << 16) | ((unsigned short)-128)},
									{TAG_MORE, (IPTR)&_settings_relative_taglist}};

// Settings objects
ObjectDef
	_config_settings_objects[] =
		{

			// Config item lister
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {0, 0, 14, SIZE_MAX_LESS - 1},
			 {2, 2, 4, -10},
			 0,
			 LISTVIEWFLAG_CURSOR_KEYS,
			 GAD_SETTINGS_ITEM_LISTER,
			 _settings_lister_taglist},

			// Edit area
			{OD_AREA,
			 0,
			 {14, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
			 {8, 2, -2, -10},
			 0,
			 AREAFLAG_RECESSED | AREAFLAG_ERASE,
			 GAD_SETTINGS_EDIT_AREA,
			 0},

			// Save
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, POS_RIGHT_JUSTIFY, 14, 1},
			 {2, -2, 4, 6},
			 MSG_SAVE,
			 BUTTONFLAG_OKAY_BUTTON,
			 GAD_SETTINGS_SAVE,
			 0},

			// Use
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_CENTER, POS_RIGHT_JUSTIFY, 14, 1},
			 {2, -2, 4, 6},
			 MSG_USE,
			 0,
			 GAD_SETTINGS_USE,
			 0},

			// Cancel
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 14, 1},
			 {-2, -2, 4, 6},
			 MSG_CANCEL,
			 BUTTONFLAG_CANCEL_BUTTON,
			 GAD_SETTINGS_CANCEL,
			 0},

			{OD_END}},

	_settings_copy_gadgets[] =
		{

			// When copying files and directories...
			{OD_TEXT,
			 TEXTPEN,
			 {1, 1, 0, 0},
			 {5, 0, 0, 0},
			 MSG_SETTINGS_WHEN_COPYING_FILES,
			 0,
			 0,
			 _settings_relative_taglist},

			// Update destination space
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 2, 0, 1},
			 {5, 4, 26, 4},
			 MSG_SETTINGS_UPDATE_FREE_SPACE,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_COPY_UPDATE,
			 _settings_relative_taglist},

			// Set archive bit
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 3, 0, 1},
			 {5, 9, 26, 4},
			 MSG_SETTINGS_SET_ARCHIVE_BIT,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_COPY_SETARCHIVE,
			 _settings_relative_taglist},

			// Also copy source's...
			{OD_TEXT,
			 TEXTPEN,
			 {1, 5, 0, 0},
			 {5, 13, 0, 0},
			 MSG_SETTINGS_ALSO_COPY_SOURCE,
			 0,
			 0,
			 _settings_relative_taglist},

			// Copy datestamp
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 6, 0, 1},
			 {5, 17, 26, 4},
			 MSG_SETTINGS_COPY_DATESTAMP,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_COPY_DATESTAMP,
			 _settings_relative_taglist},

			// Copy protection bits
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 7, 0, 1},
			 {5, 22, 26, 4},
			 MSG_SETTINGS_COPY_PROTECTION,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_COPY_PROTECTION,
			 _settings_relative_taglist},

			// Copy protection bits
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 8, 0, 1},
			 {5, 27, 26, 4},
			 MSG_SETTINGS_COPY_COMMENT,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_COPY_COMMENT,
			 _settings_relative_taglist},

			{OD_END}},

	_settings_delete_gadgets[] =
		{

			// Ask before...
			{OD_TEXT, TEXTPEN, {1, 3, 0, 0}, {5, 5, 0, 0}, MSG_SETTINGS_ASK_BEFORE, 0, 0, _settings_relative_taglist},

			// Commencing delete
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 4, 0, 1},
			 {5, 9, 26, 4},
			 MSG_SETTINGS_COMMENCING_DELETE,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_ASKDELETE_BEFORE,
			 _settings_relative_taglist},

			// Deleting files
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 5, 0, 1},
			 {5, 14, 26, 4},
			 MSG_SETTINGS_DELETING_FILES,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_ASKDELETE_FILES,
			 _settings_relative_taglist},

			// Deleting non-empty directories
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 6, 0, 1},
			 {5, 19, 26, 4},
			 MSG_SETTINGS_DELETING_DIRS,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_ASKDELETE_DIRS,
			 _settings_relative_taglist},

			{OD_END}},

	_settings_icons_gadgets[] =
		{

			// Icon handling...
			{OD_TEXT, TEXTPEN, {1, 3, 0, 0}, {5, 4, 0, 0}, MSG_SETTINGS_ICONS, 0, 0, _settings_relative_taglist},

			// Perform all actions on icons as well
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 4, 0, 1},
			 {5, 8, 26, 4},
			 MSG_SETTINGS_PERFORM_ALL_ACTIONS,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_ICONS_DOUNTO,
			 _settings_relative_taglist},

			// Select icons automatically
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 5, 0, 1},
			 {5, 13, 26, 4},
			 MSG_SETTINGS_SELECT_ICONS_AUTOMATICALLY,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_ICONS_SELECT,
			 _settings_relative_taglist},

			// Trap "more",
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {4, 6, 0, 1},
			 {5, 18, 26, 4},
			 MSG_SETTINGS_TRAP_MORE,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_TRAP_MORE,
			 _settings_relative_taglist},

			{OD_END}},

	_settings_caching_gadgets[] =
		{

			// Maximum buffers
			{OD_GADGET,
			 INTEGER_KIND,
			 {POS_RIGHT_JUSTIFY - 4, 3, 4, 1},
			 {-8, 10, 26, 4},
			 MSG_SETTINGS_CACHING_MAX_BUFFERS,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_CACHING_MAX_BUFFERS,
			 _settings_caching_buffers_taglist},

			// Disable caching
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {POS_RIGHT_JUSTIFY - 8, 4, 0, 1},
			 {-8, 15, 26, 4},
			 MSG_SETTINGS_DISABLE_CACHING,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_DISABLE_CACHING,
			 _settings_disable_caching_taglist},

			// Re-read changed buffers
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {POS_RIGHT_JUSTIFY - 8, 5, 0, 1},
			 {-8, 20, 26, 4},
			 MSG_SETTINGS_REREAD_CHANGED_BUFFERS,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_REREAD_CHANGED_BUFFERS,
			 _settings_relative_taglist},

			{OD_END}},

	_settings_date_gadgets[] =
		{

			// Date format...
			{OD_TEXT, TEXTPEN, {3, 0, 0, 0}, {5, 6, 0, 0}, MSG_SETTINGS_DATE_FORMAT, 0, 0, _settings_relative_taglist},

			// Date format gadget
			{OD_GADGET,
			 MX_KIND,
			 {6, 1, 1, 1},
			 {5, 10, 7, 1},
			 0,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_DATE_FORMAT,
			 _settings_dateformat_taglist},

			// Misc. flags...
			{OD_TEXT, TEXTPEN, {3, 6, 0, 0}, {5, 14, 0, 0}, MSG_SETTINGS_MISC, 0, 0, _settings_relative_taglist},

			// Name substitution
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {6, 7, 0, 1},
			 {5, 18, 26, 4},
			 MSG_SETTINGS_NAME_SUBSTITUTION,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_DATE_NAMESUB,
			 _settings_relative_taglist},

			// 12 hour clock
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {6, 8, 0, 1},
			 {5, 23, 26, 4},
			 MSG_SETTINGS_12_HOUR_CLOCK,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_DATE_12HOUR,
			 _settings_relative_taglist},

			// Thousands separator
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {6, 9, 0, 1},
			 {5, 28, 26, 4},
			 MSG_SETTINGS_THOUSANDS_SEPS,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_THOUSANDS_SEPS,
			 _settings_relative_taglist},

			{OD_END}},

	_settings_hide_gadgets[] =
		{

			// Hide method title
			{OD_TEXT,
			 TEXTPEN,
			 {3, 2, 0, 0},
			 {5, 4, 0, 0},
			 MSG_SETTINGS_SUB_HIDE_TITLE,
			 0,
			 0,
			 _settings_relative_taglist},

			// Date format gadget
			{OD_GADGET,
			 MX_KIND,
			 {6, 3, 1, 1},
			 {5, 8, 7, 1},
			 0,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_HIDE_METHOD,
			 _settings_hide_taglist},

			// Popkey
			{OD_GADGET,
			 HOTKEY_KIND,
			 {10, 8, SIZE_MAX_LESS - 1, 1},
			 {4, 16, -4, 4},
			 MSG_SETTINGS_POPKEY,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_POPKEY,
			 _settings_popkey_taglist},

			{OD_END}},

	_settings_misc_gadgets[] =
		{

			// Misc title
			{OD_TEXT,
			 TEXTPEN,
			 {3, 2, 0, 0},
			 {5, 11, 0, 0},
			 MSG_SETTINGS_SUB_MISC_TITLE,
			 0,
			 0,
			 _settings_relative_taglist},

			// Right button hook
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {6, 3, 0, 1},
			 {5, 14, 26, 4},
			 MSG_SETTINGS_MOUSE_HOOK,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_MOUSE_HOOK,
			 _settings_relative_taglist},

			// Quick quit
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {6, 4, 0, 1},
			 {5, 19, 26, 4},
			 MSG_SETTINGS_QUICK_QUIT,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_QUICK_QUIT,
			 _settings_relative_taglist},

			// Extended lister key selection
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {6, 5, 0, 1},
			 {5, 24, 26, 4},
			 MSG_SETTINGS_EXTENDED_KEY,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_EXTENDED_KEY,
			 _settings_relative_taglist},

			// Filetype sniffer
			{OD_GADGET,
			 CHECKBOX_KIND,
			 {6, 6, 0, 1},
			 {5, 29, 26, 4},
			 MSG_SETTINGS_FILETYPE_SNIFFER,
			 PLACETEXT_RIGHT,
			 GAD_SETTINGS_FILETYPE_SNIFFER,
			 _settings_relative_taglist},

			{OD_END}},

	_settings_pri_gadgets[] =
		{

			// Priority title
			{OD_TEXT,
			 TEXTPEN,
			 {3, 3, 0, 0},
			 {5, 11, 0, 0},
			 MSG_SETTINGS_SUB_PRI_TITLE,
			 0,
			 0,
			 _settings_relative_taglist},

			// Main priority
			{OD_GADGET,
			 INTEGER_KIND,
			 {18, 4, 6, 1},
			 {5, 14, 8, 6},
			 MSG_SETTINGS_PRI_MAIN,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_PRI_MAIN,
			 _settings_priority_taglist},

			// Main priority when busy
			{OD_GADGET,
			 INTEGER_KIND,
			 {POS_RIGHT_JUSTIFY, 4, 6, 1},
			 {-8, 14, 8, 6},
			 MSG_SETTINGS_PRI_WHEN_BUSY,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_PRI_MAIN_BUSY,
			 _settings_priority_taglist},

			// Lister priority
			{OD_GADGET,
			 INTEGER_KIND,
			 {18, 5, 6, 1},
			 {5, 22, 8, 6},
			 MSG_SETTINGS_PRI_LISTER,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_PRI_LISTER,
			 _settings_priority_taglist},

			// Lister priority when busy
			{OD_GADGET,
			 INTEGER_KIND,
			 {POS_RIGHT_JUSTIFY, 5, 6, 1},
			 {-8, 22, 8, 6},
			 MSG_SETTINGS_PRI_WHEN_BUSY,
			 PLACETEXT_LEFT,
			 GAD_SETTINGS_PRI_LISTER_BUSY,
			 _settings_priority_taglist},

			{OD_END}},

	_settings_replace_gadgets[] = {

		// Replace title
		{OD_TEXT,
		 TEXTPEN,
		 {3, 3, 0, 0},
		 {5, 11, 0, 0},
		 MSG_SETTINGS_SUB_REPLACE_TITLE,
		 0,
		 0,
		 _settings_relative_taglist},

		// Verbose replace requester
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {6, 4, 0, 1},
		 {5, 14, 26, 4},
		 MSG_SETTINGS_VERBOSE_REPLACE,
		 PLACETEXT_RIGHT,
		 GAD_SETTINGS_VERBOSE_REPLACE,
		 _settings_relative_taglist},

		// Check version
		{OD_GADGET,
		 CHECKBOX_KIND,
		 {6, 5, 0, 1},
		 {5, 19, 26, 4},
		 MSG_SETTINGS_CHECK_VERSION,
		 PLACETEXT_RIGHT,
		 GAD_SETTINGS_CHECK_VERSION,
		 _settings_relative_taglist},

		{OD_END}};

// Sub options
SubOptionHandle _settings_options[] = {{SETTINGS_COPY, MSG_SETTINGS_SUB_COPY, _settings_copy_gadgets},
									   {SETTINGS_DELETE, MSG_SETTINGS_SUB_DELETE, _settings_delete_gadgets},
									   {SETTINGS_ICONS, MSG_SETTINGS_SUB_ICONS, _settings_icons_gadgets},
									   {SETTINGS_CACHING, MSG_SETTINGS_SUB_CACHING, _settings_caching_gadgets},
									   {SETTINGS_DATE, MSG_SETTINGS_SUB_DATE_FORMAT, _settings_date_gadgets},
									   {SETTINGS_HIDE, MSG_SETTINGS_SUB_HIDE, _settings_hide_gadgets},
									   {SETTINGS_MISC, MSG_SETTINGS_SUB_MISC, _settings_misc_gadgets},
									   {SETTINGS_PRI, MSG_SETTINGS_SUB_PRI, _settings_pri_gadgets},
									   {SETTINGS_REPLACE, MSG_SETTINGS_SUB_REPLACE, _settings_replace_gadgets},
									   {-1}};
