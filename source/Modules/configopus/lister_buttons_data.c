#include "config_lib.h"

ConfigWindow _lister_buttons_window = {{POS_CENTER, POS_CENTER, 42, 2}, {0, 0, 80, 80}};

struct TagItem

	_lister_buttons_layout[] = {{GTCustom_LayoutRel, GAD_LISTER_BUTTONS_LAYOUT}, {TAG_DONE}},

	_lister_buttons_scroller[] = {{GA_RelVerify, TRUE},
								  {GA_Immediate, TRUE},
								  {GTSC_Arrows, 14},
								  {TAG_MORE, (IPTR)_lister_buttons_layout}};

ObjectDef _lister_buttons_objects[] = {

	// Layout area
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, SIZE_MAX_LESS - 1},
	 {2, 2, -2, -10},
	 0,
	 AREAFLAG_RECESSED | AREAFLAG_ERASE,
	 GAD_LISTER_BUTTONS_LAYOUT,
	 0},

	// Lister button display
	{OD_AREA,
	 0,
	 {0, 0, SIZE_MAXIMUM, 0},
	 {4, 4, -4, 38},
	 0,
	 AREAFLAG_RAISED | AREAFLAG_ERASE,
	 GAD_LISTER_BUTTONS_DISPLAY,
	 _lister_buttons_layout},

	// Lister button scroller
	{OD_GADGET,
	 SCROLLER_KIND,
	 {0, 0, SIZE_MAXIMUM, 0},
	 {4, 42, -4, 14},
	 0,
	 0,
	 GAD_LISTER_BUTTONS_SCROLLER,
	 _lister_buttons_scroller},

	// Add
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, 0, 7, 1},
	 {4, 58, 4, 4},
	 MSG_LISTER_BUTTONS_ADD,
	 0,
	 GAD_LISTER_BUTTONS_ADD,
	 _lister_buttons_layout},

	// Insert
	{OD_GADGET,
	 BUTTON_KIND,
	 {7, 0, 7, 1},
	 {16, 58, 4, 4},
	 MSG_LISTER_BUTTONS_INSERT,
	 0,
	 GAD_LISTER_BUTTONS_INSERT,
	 _lister_buttons_layout},

	// Remove
	{OD_GADGET,
	 BUTTON_KIND,
	 {14, 0, 7, 1},
	 {28, 58, 4, 4},
	 MSG_LISTER_BUTTONS_REMOVE,
	 0,
	 GAD_LISTER_BUTTONS_REMOVE,
	 _lister_buttons_layout},

	// Left
	{OD_GADGET,
	 BUTTON_KIND,
	 {21, 0, 7, 1},
	 {40, 58, 4, 4},
	 MSG_LISTER_BUTTONS_LEFT,
	 0,
	 GAD_LISTER_BUTTONS_LEFT,
	 _lister_buttons_layout},

	// Right
	{OD_GADGET,
	 BUTTON_KIND,
	 {28, 0, 7, 1},
	 {52, 58, 4, 4},
	 MSG_LISTER_BUTTONS_RIGHT,
	 0,
	 GAD_LISTER_BUTTONS_RIGHT,
	 _lister_buttons_layout},

	// Edit
	{OD_GADGET,
	 BUTTON_KIND,
	 {35, 0, 7, 1},
	 {64, 58, 4, 4},
	 MSG_LISTER_BUTTONS_EDIT,
	 0,
	 GAD_LISTER_BUTTONS_EDIT,
	 _lister_buttons_layout},

	// Save
	{OD_GADGET,
	 BUTTON_KIND,
	 {0, POS_RIGHT_JUSTIFY, 12, 1},
	 {2, -2, 12, 6},
	 MSG_LISTER_BUTTONS_SAVE,
	 0,
	 GAD_LISTER_BUTTONS_SAVE,
	 0},

	// Use
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_CENTER, POS_RIGHT_JUSTIFY, 12, 1},
	 {2, -2, 12, 6},
	 MSG_LISTER_BUTTONS_USE,
	 0,
	 GAD_LISTER_BUTTONS_USE,
	 0},

	// Cancel
	{OD_GADGET,
	 BUTTON_KIND,
	 {POS_RIGHT_JUSTIFY, POS_RIGHT_JUSTIFY, 12, 1},
	 {-2, -2, 12, 6},
	 MSG_LISTER_BUTTONS_CANCEL,
	 BUTTONFLAG_CANCEL_BUTTON,
	 GAD_LISTER_BUTTONS_CANCEL,
	 0},

	{OD_END}};

MenuData _lister_buttons_menus[] = {
	{NM_TITLE, 0, MSG_PROJECT, 0},
	{NM_ITEM, MENU_LISTER_BUTTONS_NEW, MSG_LISTER_BUTTONS_MENU_NEW, MENUFLAG_COMM_SEQ},
	{NM_ITEM, MENU_LISTER_BUTTONS_OPEN, MSG_LISTER_BUTTONS_MENU_OPEN, MENUFLAG_COMM_SEQ},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, MENU_LISTER_BUTTONS_SAVE, MSG_LISTER_BUTTONS_MENU_SAVE, MENUFLAG_COMM_SEQ},
	{NM_ITEM, MENU_LISTER_BUTTONS_SAVEAS, MSG_LISTER_BUTTONS_MENU_SAVEAS, MENUFLAG_COMM_SEQ},
	{NM_ITEM, 0, NM_BAR_LABEL, 0},
	{NM_ITEM, GAD_LISTER_BUTTONS_CANCEL, MSG_LISTER_BUTTONS_MENU_QUIT, MENUFLAG_COMM_SEQ},
	{NM_TITLE, 0, MSG_BUTTONS_EDIT, 0},
	{NM_ITEM,
	 MENU_LISTER_RESET_DEFAULTS,
	 MSG_RESET_DEFAULTS,
	 MENUFLAG_COMM_SEQ | MENUFLAG_USE_SEQ | MENUFLAG_MAKE_SEQ('D')},
	{NM_ITEM, MENU_LISTER_LAST_SAVED, MSG_LAST_SAVED, MENUFLAG_COMM_SEQ},
	{NM_ITEM, MENU_LISTER_RESTORE, MSG_RESTORE, MENUFLAG_COMM_SEQ},
	{NM_END}};
