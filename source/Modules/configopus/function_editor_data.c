#include "config_lib.h"

// Function editor window dimensions
ConfigWindow _function_editor_label_window = {{POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 50, 12}, {0, 0, 44, 68}},

			 _function_editor_window = {{POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 50, 11}, {0, 0, 44, 60}},

			 _function_editor_window_center = {{POS_CENTER, POS_CENTER, 50, 11}, {0, 0, 44, 60}};

// Labels
UWORD
_function_type_labels[] = {MSG_FUNCED_FUNCTYPE_COMMAND,
						   MSG_FUNCED_FUNCTYPE_AMIGADOS,
						   MSG_FUNCED_FUNCTYPE_WORKBENCH,
						   MSG_FUNCED_FUNCTYPE_BATCH,
						   MSG_FUNCED_FUNCTYPE_AREXX,
						   0};

// Taglists
struct TagItem

	_function_edit_layout[] = {{GTCustom_LayoutRel, GAD_FUNCED_LAYOUT}, {TAG_END}},

	_function_label_taglist[] = {{GTST_MaxChars, 80}, {GTCustom_LayoutRel, GAD_FUNCED_LAYOUT_1}, {TAG_END}},

	_function_key_taglist[] = {{GTST_MaxChars, 80},
							   {GTCustom_NoSelectNext, TRUE},
							   {TAG_MORE, (IPTR)_function_edit_layout}},

	_function_type_taglist[] = {{GTCustom_LocaleLabels, (IPTR)_function_type_labels},
								{TAG_MORE, (IPTR)_function_edit_layout}},

	_function_lister_taglist[] = {{DLV_ShowSelected, 0}, {DLV_DragNotify, 2}, {TAG_MORE, (IPTR)_function_edit_layout}},

	_function_flags_lister_taglist[] = {{DLV_MultiSelect, 1},
										{DLV_Flags, PLACETEXT_LEFT},
										{TAG_MORE, (IPTR)_function_edit_layout}},

	_function_edit_taglist[] = {{GTST_MaxChars, 255},
								{GTCustom_NoSelectNext, TRUE},
								{TAG_MORE, (IPTR)_function_edit_layout}},

	_function_editor_image[] = {{GTCustom_Control, GAD_FUNCED_LABEL},
								{DFB_DefPath, (IPTR) "DOpus5:Images/"},
								{GTCustom_LayoutRel, GAD_FUNCED_LAYOUT_1},
								{TAG_END}};

// Function editor objects
ObjectDef _function_editor_label_objects[] =
	{

		// Layout box
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {2, 2, -2, -10},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE,
		 GAD_FUNCED_LAYOUT_1,
		 0},

		// Function label
		{OD_GADGET,
		 STRING_KIND,
		 {8, 0, SIZE_MAXIMUM, 1},
		 {4, 4, -4, 4},
		 MSG_FUNCED_LABEL,
		 PLACETEXT_LEFT,
		 GAD_FUNCED_LABEL,
		 _function_label_taglist},

		// Real layout box
		{OD_AREA,
		 0,
		 {0, 1, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {2, 10, -2, -10},
		 0,
		 AREAFLAG_NOFILL,
		 GAD_FUNCED_LAYOUT,
		 0},

		{OD_END}},

		  _function_editor_image_objects[] =
			  {

				  // Layout box
				  {OD_AREA,
				   0,
				   {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
				   {2, 2, -2, -10},
				   0,
				   AREAFLAG_RECESSED | AREAFLAG_ERASE,
				   GAD_FUNCED_LAYOUT_1,
				   0},

				  // Image requester
				  {OD_GADGET,
				   FILE_BUTTON_KIND,
				   {8, 0, 0, 1},
				   {4, 4, 28, 4},
				   MSG_FUNCED_IMAGE,
				   PLACETEXT_LEFT,
				   GAD_FUNCED_IMAGE_REQ,
				   _function_editor_image},

				  // Function image
				  {OD_GADGET,
				   STRING_KIND,
				   {8, 0, SIZE_MAXIMUM, 1},
				   {32, 4, -4, 4},
				   0,
				   0,
				   GAD_FUNCED_LABEL,
				   _function_label_taglist},

				  // Real layout box
				  {OD_AREA,
				   0,
				   {0, 1, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
				   {2, 10, -2, -10},
				   0,
				   AREAFLAG_NOFILL,
				   GAD_FUNCED_LAYOUT,
				   0},

				  {OD_END}},

		  _function_editor_normal_objects[] =
			  {

				  // Layout box
				  {OD_AREA,
				   0,
				   {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
				   {2, 2, -2, -10},
				   0,
				   AREAFLAG_RECESSED | AREAFLAG_ERASE,
				   GAD_FUNCED_LAYOUT,
				   0},

				  {OD_END}},

		  _function_editor_objects[] =
			  {

				  // Function lister
				  {OD_GADGET,
				   MY_LISTVIEW_KIND,
				   {0, 0, SIZE_MAXIMUM, 5},
				   {4, 4, -4, 4},
				   0,
				   PLACETEXT_LEFT,
				   GAD_FUNCED_LISTER,
				   _function_lister_taglist},

				  // Function type
				  {OD_GADGET,
				   CYCLE_KIND,
				   {0, 5, 10, 1},
				   {4, 9, 24, 6},
				   0,
				   0,
				   GAD_FUNCED_FUNCTION_TYPE,
				   _function_type_taglist},

				  // Function edit glass
				  {OD_GADGET,
				   FILE_BUTTON_KIND,
				   {10, 5, 0, 1},
				   {28, 9, 28, 6},
				   0,
				   0,
				   GAD_FUNCED_EDIT_GLASS,
				   _function_edit_layout},

				  // Function edit
				  {OD_GADGET,
				   STRING_KIND,
				   {10, 5, SIZE_MAXIMUM, 1},
				   {56, 9, -32, 6},
				   0,
				   0,
				   GAD_FUNCED_EDIT,
				   _function_edit_taglist},

				  // Function argument button
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_RIGHT_JUSTIFY, 5, 0, 1},
				   {-4, 9, 28, 6},
				   (IPTR) "{ }",
				   TEXTFLAG_TEXT_STRING,
				   GAD_FUNCED_EDIT_ARGUMENT,
				   _function_edit_layout},

				  // New entry
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, 6, 10, 1},
				   {4, 16, 24, 6},
				   MSG_FUNCED_NEW_ENTRY,
				   0,
				   GAD_FUNCED_NEW_ENTRY,
				   _function_edit_layout},

				  // Insert entry
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, 7, 10, 1},
				   {4, 23, 24, 6},
				   MSG_FUNCED_INSERT,
				   0,
				   GAD_FUNCED_INSERT_ENTRY,
				   _function_edit_layout},

				  // Delete entry
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, 8, 10, 1},
				   {4, 30, 24, 6},
				   MSG_FUNCED_DELETE,
				   0,
				   GAD_FUNCED_DELETE_ENTRY,
				   _function_edit_layout},

				  // Flags
				  {OD_GADGET,
				   MY_LISTVIEW_KIND,
				   {POS_RIGHT_JUSTIFY, 6, 24, 3},
				   {-4, 16, 20, 20},
				   MSG_FUNCED_FLAGS,
				   PLACETEXT_ABOVE,
				   GAD_FUNCED_FLAGS,
				   _function_flags_lister_taglist},

				  // Key
				  {OD_GADGET,
				   HOTKEY_KIND,
				   {POS_RIGHT_JUSTIFY, 9, 24, 1},
				   {-4, 38, 20, 4},
				   MSG_FUNCED_KEY,
				   PLACETEXT_LEFT,
				   GAD_FUNCED_KEY,
				   _function_key_taglist},

				  // Cancel button
				  {OD_GADGET,
				   BUTTON_KIND,
				   {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 14, 1},
				   {-2, -2, 4, 6},
				   MSG_CANCEL,
				   BUTTONFLAG_CANCEL_BUTTON,
				   GAD_FUNCED_CANCEL,
				   0},

				  {OD_END}},

		  _function_editor_objects_use[] =
			  {

				  // Use button
				  {OD_GADGET,
				   BUTTON_KIND,
				   {0, POS_RIGHT_JUSTIFY, 14, 1},
				   {2, -2, 4, 6},
				   MSG_USE,
				   BUTTONFLAG_OKAY_BUTTON,
				   GAD_FUNCED_USE,
				   0},

				  {OD_END}},

		  _function_editor_objects_save[] = {

			  // Save button
			  {OD_GADGET,
			   BUTTON_KIND,
			   {0, POS_RIGHT_JUSTIFY, 14, 1},
			   {2, -2, 4, 6},
			   MSG_SAVE,
			   BUTTONFLAG_OKAY_BUTTON,
			   GAD_FUNCED_USE,
			   0},

			  {OD_END}};

long
	// Flag table
	_funced_flaglist[] = {FUNCF_OUTPUT_WINDOW,
						  MSG_FUNCED_FLAG_OUTPUT_WINDOW,
						  FUNCF_WORKBENCH_OUTPUT,
						  MSG_FUNCED_FLAG_WORKBENCH_OUTPUT,
						  FUNCF_OUTPUT_FILE,
						  MSG_FUNCED_FLAG_OUTPUT_FILE,
						  FUNCF_RUN_ASYNC,
						  MSG_FUNCED_FLAG_RUN_ASYNC,
						  FUNCF_CD_SOURCE,
						  MSG_FUNCED_FLAG_CD_SOURCE,
						  FUNCF_CD_DESTINATION,
						  MSG_FUNCED_FLAG_CD_DEST,
						  FUNCF_DO_ALL_FILES,
						  MSG_FUNCED_FLAG_DO_ALL_FILES,
						  FUNCF_RECURSE_DIRS,
						  MSG_FUNCED_FLAG_RECURSE_DIRS,
						  FUNCF_RELOAD_FILES,
						  MSG_FUNCED_FLAG_RELOAD_EACH_FILE,
						  FUNCF_NO_QUOTES,
						  MSG_FUNCED_FLAG_NO_FILENAME_QUOTE,
						  FUNCF_RESCAN_SOURCE,
						  MSG_FUNCED_FLAG_RESCAN_SOURCE,
						  FUNCF_RESCAN_DEST,
						  MSG_FUNCED_FLAG_RESCAN_DEST,
						  FUNCF_WAIT_CLOSE,
						  MSG_FUNCED_FLAG_WAIT_CLOSE,
						  0};

MenuData _funced_menus[] = {
	{NM_TITLE, 0, MSG_EDIT, 0},
	{NM_ITEM, MENU_FUNCED_COPY, MSG_COPY, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('C')},
	{NM_ITEM, MENU_FUNCED_CUT, MSG_CUT, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('X')},
	{NM_ITEM, MENU_FUNCED_PASTE, MSG_PASTE, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('V')},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_FUNCED_EXPORT_ASCII, MSG_FUNCED_EXPORT_ASCII_MENU, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('E')},
	{NM_ITEM, MENU_FUNCED_EXPORT_CMD, MSG_FUNCED_EXPORT_CMD_MENU, MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('F')},
	{NM_END}};
