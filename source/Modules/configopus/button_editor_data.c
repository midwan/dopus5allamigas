#include "config_lib.h"

// Button editor window dimensions
const ConfigWindow _button_editor_window = {{POS_MOUSE_CENTER, 0, 50, 9}, {0, 0, 52, 93}};

// Call-back hook to get task pointer
static void ASM button_editor_task_callback(REG(a1, struct TagItem *tag), REG(a2, struct Window *window))
{
	// Get task pointer
	tag->ti_Tag = GTCustom_ChangeSigTask;
	tag->ti_Data = (IPTR)FindTask(0);
}

// Call-back hook to get signal bit
static void ASM button_editor_bit_callback(REG(a1, struct TagItem *tag), REG(a2, struct Window *window))
{
	ButtonEdData *data;

	// Get data pointer
	data = (ButtonEdData *)CFGDATA(window);

	// Get signal bit
	tag->ti_Tag = GTCustom_ChangeSigBit;
	tag->ti_Data = (ULONG)data->change_bit;
}

// Taglists
const struct TagItem

	_button_editor_relative[] = {{GTCustom_LayoutRel, GAD_BUTTONED_LAYOUT_AREA}, {TAG_END}},

	_button_editor_function_layout[] = {{GTCustom_LayoutRel, GAD_BUTTONED_LAYOUT}, {TAG_END}},

	_button_editor_palette_layout[] = {{GTCustom_LayoutRel, GAD_BUTTONED_PALETTE}, {TAG_END}},

	_button_editor_functions[] = {{DLV_ShowSelected, TRUE},
								  {DLV_DragNotify, 1},
								  {DLV_ShowChecks, 2},
								  {TAG_MORE, (IPTR)_button_editor_function_layout}},

	_button_editor_name[] = {{GTST_MaxChars, 255},
							 {GTCustom_ThinBorders, TRUE},
							 {TAG_MORE, (IPTR)_button_editor_function_layout}},

	_button_editor_label[] = {{GTST_MaxChars, 255},
							  {GTCustom_CallBack, (IPTR)button_editor_task_callback},
							  {GTCustom_CallBack, (IPTR)button_editor_bit_callback},
							  {GTCustom_ThinBorders, TRUE},
							  {TAG_MORE, (IPTR)_button_editor_function_layout}},

	_button_editor_image[] = {{GTCustom_Control, GAD_BUTTONED_LABEL},
							  {DFB_DefPath, (IPTR) "DOpus5:Images/"},
							  {TAG_MORE, (IPTR)_button_editor_function_layout}};

// Button editor objects
const ObjectDef _button_editor_objects[] =
	{

		// Layout box
		{OD_AREA,
		 0,
		 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
		 {3, 3, -3, -12},
		 0,
		 AREAFLAG_RECESSED | AREAFLAG_ERASE | AREAFLAG_THIN,
		 GAD_BUTTONED_LAYOUT_AREA,
		 0},

		// Function area
		{OD_AREA,
		 TEXTPEN,
		 {0, 1, SIZE_MAXIMUM, 4},
		 {6, -2, -6, 20},
		 MSG_BUTTONED_FUNCTIONS,
		 AREAFLAG_TITLE,
		 GAD_BUTTONED_LAYOUT,
		 _button_editor_relative},

		// Function list
		{OD_GADGET,
		 MY_LISTVIEW_KIND,
		 {0, 0, 16, SIZE_MAX_LESS - 1},
		 {4, 0, 24, -8},
		 0,
		 LISTVIEWFLAG_CURSOR_KEYS,
		 GAD_BUTTONED_FUNCTIONS,
		 _button_editor_functions},

		// Add
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 8, 1},
		 {4, -4, 12, 4},
		 MSG_BUTTONED_ADD,
		 BUTTONFLAG_THIN_BORDERS,
		 GAD_BUTTONED_ADD,
		 _button_editor_function_layout},

		// Del
		{OD_GADGET,
		 BUTTON_KIND,
		 {8, POS_RIGHT_JUSTIFY, 8, 1},
		 {16, -4, 12, 4},
		 MSG_BUTTONED_DEL,
		 BUTTONFLAG_THIN_BORDERS,
		 GAD_BUTTONED_DEL,
		 _button_editor_function_layout},

		// Name
		{OD_GADGET,
		 STRING_KIND,
		 {24, 0, SIZE_MAXIMUM, 1},
		 {32, 0, -4, 6},
		 MSG_BUTTONED_NAME,
		 PLACETEXT_LEFT,
		 GAD_BUTTONED_NAME,
		 _button_editor_name},

		// Edit function
		{OD_GADGET,
		 BUTTON_KIND,
		 {24, 2, SIZE_MAXIMUM, 1},
		 {32, 16, -4, 6},
		 MSG_BUTTONED_EDIT_FUNCTION,
		 BUTTONFLAG_THIN_BORDERS,
		 GAD_BUTTONED_EDIT_FUNCTION,
		 _button_editor_function_layout},

		// Palette area
		{OD_AREA,
		 TEXTPEN,
		 {0, 6, 20, SIZE_MAXIMUM},
		 {6, 22, 54, -6},
		 MSG_BUTTONED_PALETTE,
		 AREAFLAG_TITLE,
		 GAD_BUTTONED_PALETTE,
		 _button_editor_relative},

		// Foreground
		{OD_GADGET,
		 PALETTE_KIND,
		 {0, 1, 10, SIZE_MAXIMUM},
		 {6, 2, 16, -4},
		 MSG_PALETTE_FOREGROUND,
		 PLACETEXT_ABOVE | BUTTONFLAG_THIN_BORDERS,
		 GAD_PALETTE_FOREGROUND,
		 _button_editor_palette_layout},

		// Background
		{OD_GADGET,
		 PALETTE_KIND,
		 {10, 1, 10, SIZE_MAXIMUM},
		 {28, 2, 16, -4},
		 MSG_PALETTE_BACKGROUND,
		 PLACETEXT_ABOVE | BUTTONFLAG_THIN_BORDERS,
		 GAD_PALETTE_BACKGROUND,
		 _button_editor_palette_layout},

		// Button area
		{OD_AREA,
		 TEXTPEN,
		 {20, 6, SIZE_MAXIMUM, SIZE_MAXIMUM},
		 {64, 22, -6, -6},
		 MSG_BUTTONED_BUTTON,
		 AREAFLAG_TITLE | AREAFLAG_ERASE,
		 GAD_BUTTONED_BUTTON,
		 _button_editor_relative},

		// Use
		{OD_GADGET,
		 BUTTON_KIND,
		 {0, POS_RIGHT_JUSTIFY, 14, 1},
		 {3, -3, 8, 6},
		 MSG_USE,
		 BUTTONFLAG_OKAY_BUTTON | BUTTONFLAG_THIN_BORDERS,
		 GAD_BUTTONED_USE,
		 0},

		// Cancel
		{OD_GADGET,
		 BUTTON_KIND,
		 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 14, 1},
		 {-3, -3, 8, 6},
		 MSG_CANCEL,
		 BUTTONFLAG_CANCEL_BUTTON | BUTTONFLAG_THIN_BORDERS,
		 GAD_BUTTONED_CANCEL,
		 0},

		{OD_END}},

				_button_editor_image_objects[] =
					{

						// Image requester
						{OD_GADGET,
						 FILE_BUTTON_KIND,
						 {24, 1, 0, 1},
						 {32, 8, 28, 6},
						 MSG_BUTTONED_IMAGE,
						 BUTTONFLAG_THIN_BORDERS,
						 GAD_BUTTONED_IMAGE_REQ,
						 _button_editor_image},

						// Image path
						{OD_GADGET,
						 STRING_KIND,
						 {24, 1, SIZE_MAXIMUM, 1},
						 {60, 8, -4, 6},
						 0,
						 0,
						 GAD_BUTTONED_LABEL,
						 _button_editor_name},

						{OD_END}},

				_button_editor_label_objects[] = {

					// Label
					{OD_GADGET,
					 STRING_KIND,
					 {24, 1, SIZE_MAXIMUM, 1},
					 {32, 8, -4, 6},
					 MSG_BUTTONED_LABEL,
					 PLACETEXT_LEFT,
					 GAD_BUTTONED_LABEL,
					 _button_editor_label},

					{OD_END}};
