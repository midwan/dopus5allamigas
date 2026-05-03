#include "config_lib.h"
#include "config_filetypes.h"

// Filetypes editor window dimensions
const ConfigWindow _config_filetypes_window = {{POS_CENTER, POS_CENTER, 48, 11}, {0, 0, 42, 16}},

				   _filetype_editor_window = {{POS_CENTER, POS_CENTER, 60, 12}, {0, 0, 104, 32}},

				   _fileclass_editor_window = {{POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 54, 10}, {0, 0, 52, 48}};

// Labels
const UWORD

	matchtype_labels[] = {MSG_CLASSED_MATCH_MATCH,
						  MSG_CLASSED_MATCH_MATCH_DESC,
						  MSG_CLASSED_MATCH_MATCHI,
						  MSG_CLASSED_MATCH_MATCHI_DESC,
						  MSG_CLASSED_MATCH_MATCHNAME,
						  MSG_CLASSED_MATCH_MATCHNAME_DESC,
						  MSG_CLASSED_MATCH_MATCHFORM,
						  MSG_CLASSED_MATCH_MATCHFORM_DESC,
						  MSG_CLASSED_MATCH_MATCHBITS,
						  MSG_CLASSED_MATCH_MATCHBITS_DESC,
						  MSG_CLASSED_MATCH_MATCHCOMMENT,
						  MSG_CLASSED_MATCH_MATCHCOMMENT_DESC,
						  MSG_CLASSED_MATCH_MATCHDATE,
						  MSG_CLASSED_MATCH_MATCHDATE_DESC,
						  MSG_CLASSED_MATCH_MATCHSIZE,
						  MSG_CLASSED_MATCH_MATCHSIZE_DESC,
						  MSG_CLASSED_MATCH_DTGROUP,
						  MSG_CLASSED_MATCH_DTGROUP_DESC,
						  MSG_CLASSED_MATCH_DTID,
						  MSG_CLASSED_MATCH_DTID_DESC,
						  MSG_CLASSED_MATCH_MODULE,
						  MSG_CLASSED_MATCH_MODULE_DESC,
						  MSG_CLASSED_MATCH_DIRECTORY,
						  MSG_CLASSED_MATCH_DIRECTORY_DESC,
						  MSG_CLASSED_MATCH_DISK,
						  MSG_CLASSED_MATCH_DISK_DESC,
						  MSG_CLASSED_MATCH_SEARCHFOR,
						  MSG_CLASSED_MATCH_SEARCHFOR_DESC,
						  MSG_CLASSED_MATCH_SEARCHRANGE,
						  MSG_CLASSED_MATCH_SEARCHRANGE_DESC,
						  MSG_CLASSED_MATCH_FINDCHUNK,
						  MSG_CLASSED_MATCH_FINDCHUNK_DESC,
						  MSG_CLASSED_MATCH_MATCHCHUNK,
						  MSG_CLASSED_MATCH_MATCHCHUNK_DESC,
						  MSG_CLASSED_MATCH_MOVE,
						  MSG_CLASSED_MATCH_MOVE_DESC,
						  MSG_CLASSED_MATCH_MOVETO,
						  MSG_CLASSED_MATCH_MOVETO_DESC,
						  MSG_CLASSED_MATCH_AND,
						  MSG_CLASSED_MATCH_AND_DESC,
						  MSG_CLASSED_MATCH_OR,
						  MSG_CLASSED_MATCH_OR_DESC,
						  0,
						  0};

// Tags
const struct TagItem

	_filetype_function_rel[] = {{GTCustom_LayoutRel, GAD_FILETYPES_FUNCTION_AREA}, {TAG_END}},

	_config_filetype_list_tags[] = {{DLV_ShowSelected, 0}, {TAG_END}},

	_filetype_action_list_tags[] = {{DLV_ShowChecks, 2},
									{DLV_ShowSelected, 0},
									{TAG_MORE, (IPTR)_filetype_function_rel}},

	_filetype_icon_menu_tags[] = {{DLV_ShowSelected, 1},
								  {DLV_DragNotify, 1},
								  {TAG_MORE, (IPTR)_filetype_function_rel}},

	_classed_name_tags[] = {{GTST_MaxChars, 31}, {TAG_END}},

	_classed_id_tags[] = {{GTST_MaxChars, 7}, {TAG_END}},

	_classed_pri_tags[] = {{GTIN_MaxChars, 4}, {GTCustom_MinMax, (ULONG)((127 << 16) | 0xff80)}, {TAG_END}},

	_classed_definition_tags[] = {{DLV_ShowSelected, 0}, {TAG_END}},

	_classed_matchdata_tags[] = {{GTST_MaxChars, 79}, {TAG_END}};

// Objects
const ObjectDef
	_config_filetypes_objects[] =
		{

			// Filetype list
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {0, 0, SIZE_MAX_LESS - 12, SIZE_MAX_LESS - 1},
			 {2, 2, -8, -10},
			 0,
			 LISTVIEWFLAG_CURSOR_KEYS,
			 GAD_FILETYPES_LIST,
			 _config_filetype_list_tags},

			// Add
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, 0, 12, 1},
			 {-2, 2, 4, 6},
			 MSG_FILETYPES_ADD,
			 0,
			 GAD_FILETYPES_ADD,
			 0},

			// Duplicate
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, 1, 12, 1},
			 {-2, 9, 4, 6},
			 MSG_FILETYPES_DUPLICATE,
			 0,
			 GAD_FILETYPES_DUPLICATE,
			 0},

			// Edit
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, 2, 12, 1},
			 {-2, 16, 4, 6},
			 MSG_FILETYPES_EDIT,
			 0,
			 GAD_FILETYPES_EDIT,
			 0},

			// Remove
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, 3, 12, 1},
			 {-2, 23, 4, 6},
			 MSG_FILETYPES_REMOVE,
			 0,
			 GAD_FILETYPES_REMOVE,
			 0},

			// Store
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, 4, 12, 1},
			 {-2, 30, 4, 6},
			 MSG_FILETYPES_STORE,
			 0,
			 GAD_FILETYPES_STORE,
			 0},

			// Use
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, POS_RIGHT_JUSTIFY, 12, 1},
			 {2, -2, 4, 6},
			 MSG_FILETYPES_SAVE,
			 BUTTONFLAG_OKAY_BUTTON,
			 GAD_FILETYPES_USE,
			 0},

			// Cancel
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
			 {-2, -2, 4, 6},
			 MSG_CANCEL,
			 BUTTONFLAG_CANCEL_BUTTON,
			 GAD_FILETYPES_CANCEL,
			 0},

			{OD_END}},

	_filetype_editor_objects[] =
		{

			// Function area
			{OD_AREA,
			 0,
			 {0, 0, 60, 11},
			 {2, 2, 100, 20},
			 0,
			 AREAFLAG_RECESSED | AREAFLAG_ERASE,
			 GAD_FILETYPES_FUNCTION_AREA,
			 0},

			// Action list
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {0, 1, 22, 9},
			 {4, 4, 24, 4},
			 MSG_FILETYPES_EVENTS,
			 PLACETEXT_ABOVE,
			 GAD_FILETYPEED_ACTION_LIST,
			 _filetype_action_list_tags},

			// Edit action button
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, 10, 14, 1},
			 {4, 8, 16, 6},
			 MSG_FILETYPES_EDIT_ACTION,
			 0,
			 GAD_FILETYPES_EDIT_ACTION,
			 _filetype_function_rel},

			// Delete action
			{OD_GADGET,
			 BUTTON_KIND,
			 {14, 10, 8, 1},
			 {20, 8, 8, 6},
			 MSG_CLASSED_DEL,
			 0,
			 GAD_FILETYPES_DEL_ACTION,
			 _filetype_function_rel},

			// Icon menu list
			{OD_GADGET,
			 MY_LISTVIEW_KIND,
			 {22, 1, 21, 9},
			 {36, 4, 24, 4},
			 MSG_FILETYPES_ICON_MENU,
			 PLACETEXT_ABOVE,
			 GAD_FILETYPES_ICON_MENU,
			 _filetype_icon_menu_tags},

			// Edit icon menu
			{OD_GADGET,
			 BUTTON_KIND,
			 {22, 10, 7, 1},
			 {36, 8, 8, 6},
			 MSG_FILETYPES_EDIT_ICON_MENU,
			 0,
			 GAD_FILETYPES_EDIT_ICON_MENU,
			 _filetype_function_rel},

			// Add icon menu
			{OD_GADGET,
			 BUTTON_KIND,
			 {29, 10, 7, 1},
			 {44, 8, 8, 6},
			 MSG_FILETYPES_ADD_ICON_MENU,
			 0,
			 GAD_FILETYPES_ADD_ICON_MENU,
			 _filetype_function_rel},

			// Delete icon menu
			{OD_GADGET,
			 BUTTON_KIND,
			 {36, 10, 7, 1},
			 {52, 8, 8, 6},
			 MSG_FILETYPES_DEL_ICON_MENU,
			 0,
			 GAD_FILETYPES_DEL_ICON_MENU,
			 _filetype_function_rel},

			// Icon label
			{OD_TEXT, TEXTPEN, {43, 0, 0, 1}, {68, 3, 0, 0}, MSG_FILETYPES_ICON, 0, 0, _filetype_function_rel},

			// Icon area
			{OD_AREA,
			 0,
			 {43, 1, 17, 9},
			 {68, 4, 24, 4},
			 0,
			 AREAFLAG_RECESSED | AREAFLAG_ERASE | AREAFLAG_FILL_COLOUR,
			 GAD_FILETYPEED_ICON_AREA,
			 _filetype_function_rel},

			// Select icon
			{OD_GADGET,
			 BUTTON_KIND,
			 {43, 10, 17, 1},
			 {68, 8, 24, 6},
			 MSG_FILETYPE_SELECT_ICON,
			 0,
			 GAD_FILETYPEED_SELECT_ICON,
			 _filetype_function_rel},

			// Edit class
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_CENTER, POS_RIGHT_JUSTIFY, 15, 1},
			 {0, -2, 4, 6},
			 MSG_FILETYPE_EDIT_CLASS,
			 0,
			 GAD_FILETYPEED_EDIT_CLASS,
			 0},

			// Use
			{OD_GADGET,
			 BUTTON_KIND,
			 {0, POS_RIGHT_JUSTIFY, 12, 1},
			 {2, -2, 4, 6},
			 MSG_USE,
			 BUTTONFLAG_OKAY_BUTTON,
			 GAD_FILETYPEED_USE,
			 0},

			// Cancel
			{OD_GADGET,
			 BUTTON_KIND,
			 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
			 {-2, -2, 4, 6},
			 MSG_CANCEL,
			 BUTTONFLAG_CANCEL_BUTTON,
			 GAD_FILETYPEED_CANCEL,
			 0},

			{OD_END}},

	_fileclass_editor_objects[] = {

		// Background area
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {2, 2, -2, -10},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 0,
		 0},

		// File class name
		{OD_GADGET,
		 STRING_KIND,
		 {7, 0, 24, 1},
		 {8, 10, 12, 4},
		 MSG_CLASSED_CLASS_NAME,
		 0,
		 GAD_CLASSED_CLASS_NAME,
		 _classed_name_tags},

		// File class ID
		{OD_GADGET,
		 STRING_KIND,
		 {35, 0, 9, 1},
		 {18, 10, 12, 4},
		 MSG_CLASSED_CLASS_ID,
		 0,
		 GAD_CLASSED_CLASS_ID,
		 _classed_id_tags},

		// Class priority
		{OD_GADGET,
		 INTEGER_KIND,
		 {POS_RIGHT_JUSTIFY, 0, 4, 1},
		 {-8, 10, 12, 4},
		 MSG_CLASSED_CLASS_PRI,
		 0,
		 GAD_CLASSED_CLASS_PRI,
		 _classed_pri_tags},

		// Definition lister
		{OD_GADGET,
		 MY_LISTVIEW_KIND,
		 {0, 1, SIZE_MAXIMUM, 6},
		 {8, 16, -8, 4},
		 0,
		 0,
		 GAD_CLASSED_DEFINITION,
		 _classed_definition_tags},

		// Match type
		{OD_GADGET, FILE_BUTTON_KIND, {0, 7, 0, 1}, {8, 21, 28, 6}, 0, 0, GAD_CLASSED_MATCHTYPE, 0},

		// Match type display
		{OD_AREA,
		 TEXTPEN,
		 {0, 7, 16, 1},
		 {36, 21, -4, 6},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE | AREAFLAG_OPTIM | TEXTFLAG_CENTER | AREAFLAG_FILL_COLOUR,
		 GAD_CLASSED_MATCHTYPE_DISPLAY,
		 0},

		// Match data
		{OD_GADGET,
		 STRING_KIND,
		 {16, 7, SIZE_MAXIMUM, 1},
		 {32, 21, -8, 6},
		 0,
		 0,
		 GAD_CLASSED_MATCHDATA,
		 _classed_matchdata_tags},

		// Add entry
		{OD_GADGET, BUTTON_KIND, {0, 8, 12, 1}, {8, 29, 12, 6}, MSG_CLASSED_ADD, 0, GAD_CLASSED_ADD, 0},

		// Insert entry
		{OD_GADGET, BUTTON_KIND, {POS_CENTER, 8, 12, 1}, {0, 29, 12, 6}, MSG_CLASSED_INSERT, 0, GAD_CLASSED_INSERT, 0},

		// Remove entry
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, 8, 12, 1},
		 {-8, 29, 12, 6},
		 MSG_CLASSED_REMOVE,
		 0,
		 GAD_CLASSED_REMOVE,
		 0},

		// Use
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 14, 1},
		 {2, -2, 4, 6},
		 MSG_USE,
		 BUTTONFLAG_OKAY_BUTTON,
		 GAD_FILECLASSED_USE,
		 0},

		// View file
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_CENTER, POS_RIGHT_JUSTIFY, 14, 1},
		 {0, -2, 4, 6},
		 MSG_CLASSED_VIEW_FILE,
		 0,
		 GAD_FILECLASSED_VIEW_FILE,
		 0},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 14, 1},
		 {-2, -2, 4, 6},
		 MSG_CANCEL,
		 BUTTONFLAG_CANCEL_BUTTON,
		 GAD_FILECLASSED_CANCEL,
		 0},

		{OD_END}};

// Filetype action lookup
const short filetype_action_lookup[] = {MSG_FILETYPE_ACTION_DOUBLECLICK,
										FTTYPE_DOUBLE_CLICK,
										MSG_FILETYPE_ACTION_CTRL_DCLICK,
										FTTYPE_CTRL_DOUBLECLICK,
										MSG_FILETYPE_ACTION_ALT_DCLICK,
										FTTYPE_ALT_DOUBLECLICK,
										MSG_FILETYPE_ACTION_DRAGNDROP,
										FTTYPE_DRAG_DROP,
										MSG_FILETYPE_ACTION_CTRL_DDROP,
										FTTYPE_CTRL_DRAGDROP,
										MSG_FILETYPE_ACTION_ALT_DDROP,
										FTTYPE_ALT_DRAGDROP,
										MSG_FILETYPE_ACTION_USER1,
										FTTYPE_USER_1,
										MSG_FILETYPE_ACTION_USER2,
										FTTYPE_USER_2,
										MSG_FILETYPE_ACTION_USER3,
										FTTYPE_USER_3,
										MSG_FILETYPE_ACTION_USER4,
										FTTYPE_USER_4,
										MSG_FILETYPE_ACTION_USER5,
										FTTYPE_USER_5,
										MSG_FILETYPE_ACTION_USER6,
										FTTYPE_USER_6,
										MSG_FILETYPE_ACTION_USER7,
										FTTYPE_USER_7,
										MSG_FILETYPE_ACTION_USER8,
										FTTYPE_USER_8,
										MSG_FILETYPE_ACTION_USER9,
										FTTYPE_USER_9,
										MSG_FILETYPE_ACTION_USER10,
										FTTYPE_USER_10,
										0,
										0};

// Fileclass match lookup
const short fileclass_match_lookup[] = {
	FTOP_MATCH,		   FTOP_MATCHNOCASE,
	FTOP_MATCHNAME,	   FTOP_MATCHFORM,
	FTOP_MATCHBITS,	   FTOP_MATCHCOMMENT,
	FTOP_MATCHDATE,	   FTOP_MATCHSIZE,
	FTOP_MATCHDTGROUP, FTOP_MATCHDTID,
	FTOP_MODULE,	   FTOP_DIRECTORY,
	FTOP_DISK,		   FTOP_SEARCHFOR,
	FTOP_SEARCHRANGE,  FTOP_FINDCHUNK,
	FTOP_MATCHCHUNK,   FTOP_MOVE,
	FTOP_MOVETO,	   FTOP_AND,
	FTOP_OR,		   0,
};
