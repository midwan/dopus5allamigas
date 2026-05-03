#include "config_lib.h"
#include "config_environment.h"
#include <proto/module.h>

void config_env_fix_picture_gads(config_env_data *data)
{
	BOOL disable_all = 0, disable_pics = 0, disable_wb = 0;

	// Disable everything if 'enable' gadget is off
	if (!(GetGadgetValue(data->option_list, GAD_ENVIRONMENT_OPTIONS_DISPLAY_BACKDROP)))
		disable_all = 1;

	// Disable picture fields if 'use wbpattern' is on, or we're under 37
	else if (GetGadgetValue(data->option_list, GAD_ENVIRONMENT_PICTURE_USE_WBPATTERN) ||
			 ((struct Library *)GfxBase)->lib_Version < 39)
		disable_pics = 1;

	// Disable WBPattenr if 'use wbpattern' is off
	else if (!GetGadgetValue(data->option_list, GAD_ENVIRONMENT_PICTURE_USE_WBPATTERN))
		disable_wb = 1;

	// Disable/enable the gadgets
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_DESK, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_DESK_FIELD, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_DESK_PREFS, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_LISTER, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_LISTER_FIELD, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_LISTER_PREFS, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_REQ, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_REQ_FIELD, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_REQ_PREFS, disable_all | disable_pics);
	DisableObject(data->option_list, GAD_ENVIRONMENT_PICTURE_USE_WBPATTERN, disable_all);
	DisableObject(data->option_list, GAD_ENVIRONMENT_MAIN_WINDOW_BUTTONS, disable_all | disable_wb);
	DisableObject(data->option_list, GAD_ENVIRONMENT_MAIN_WINDOW_FIELD, disable_all | disable_wb);
	DisableObject(data->option_list, GAD_ENVIRONMENT_EDIT_PATTERN, disable_all | disable_wb);
}

void config_env_picture_menu(config_env_data *data, short which)
{
	short a, res;
	PopUpHandle *menu;
	PopUpItem *item;

	// Create menu
	if (!(menu = PopUpNewHandle(0, 0, locale)))
		return;

	// Initialise picture popup
	if ((item = PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_TYPE, PICMENU_TYPE, POPUPF_SUB)))
	{
		if (PopUpItemSub(menu, item))
		{
			PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_CENTERED, PICMENU_CENTERED, POPUPF_CHECKIT);
			PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_TILED, PICMENU_TILED, POPUPF_CHECKIT);
			if (which == 0)
				PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_STRETCHED, PICMENU_STRETCHED, POPUPF_CHECKIT);
			PopUpSeparator(menu);
			if ((item = PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_BORDER, PICMENU_BORDER, POPUPF_SUB)))
			{
				if (PopUpItemSub(menu, item))
				{
					PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_NORMAL, PICMENU_NORMAL, POPUPF_CHECKIT);
					PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_BLACK, PICMENU_BLACK, POPUPF_CHECKIT);
					PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_WHITE, PICMENU_WHITE, POPUPF_CHECKIT);
					PopUpEndSub(menu);
				}
			}
			PopUpEndSub(menu);
		}
	}
	if ((item = PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_REMAP, PICMENU_REMAP, POPUPF_SUB)))
	{
		if (PopUpItemSub(menu, item))
		{
			PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_NONE, PICMENU_NONE, POPUPF_CHECKIT);
			PopUpSeparator(menu);
			PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_GUI, PICMENU_GUI, POPUPF_CHECKIT);
			PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_ICON, PICMENU_ICON, POPUPF_CHECKIT);
			PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_IMAGE, PICMENU_IMAGE, POPUPF_CHECKIT);
			PopUpNewItem(menu, MSG_ENVIRONMENT_PICTURE_EXACT, PICMENU_EXACT, POPUPF_CHECKIT);
			PopUpEndSub(menu);
		}
	}

	// Check appropriate items
	if (data->config->env_BackgroundFlags[which] & ENVBF_CENTER_PIC)
	{
		PopUpSetFlags(&menu->ph_Menu, PICMENU_CENTERED, POPUPF_CHECKED, POPUPF_CHECKED);
		if (data->config->env_BackgroundFlags[which] & ENVBF_USE_COLOUR)
		{
			if (data->config->env_BackgroundBorderColour[which] == 0)
				PopUpSetFlags(&menu->ph_Menu, PICMENU_BLACK, POPUPF_CHECKED, POPUPF_CHECKED);
			else
				PopUpSetFlags(&menu->ph_Menu, PICMENU_WHITE, POPUPF_CHECKED, POPUPF_CHECKED);
		}
		else
			PopUpSetFlags(&menu->ph_Menu, PICMENU_NORMAL, POPUPF_CHECKED, POPUPF_CHECKED);
	}
	else if (which == 0 && data->config->env_BackgroundFlags[which] & ENVBF_STRETCH_PIC)
	{
		PopUpSetFlags(&menu->ph_Menu, PICMENU_STRETCHED, POPUPF_CHECKED, POPUPF_CHECKED);
		PopUpSetFlags(&menu->ph_Menu, PICMENU_BORDER, POPUPF_DISABLED, POPUPF_DISABLED);
	}
	else
	{
		PopUpSetFlags(&menu->ph_Menu, PICMENU_TILED, POPUPF_CHECKED, POPUPF_CHECKED);
		PopUpSetFlags(&menu->ph_Menu, PICMENU_BORDER, POPUPF_DISABLED, POPUPF_DISABLED);
	}
	for (a = 0; a < 5; a++)
		if (data->config->env_BackgroundFlags[which] & (1 << (a + 1)))
		{
			PopUpSetFlags(&menu->ph_Menu, PICMENU_NONE + a, POPUPF_CHECKED, POPUPF_CHECKED);
			break;
		}

	// Do popup menu
	if ((res = DoPopUpMenu(data->window, &menu->ph_Menu, 0, SELECTDOWN)) >= 0)
	{
		// Look at result
		switch (res)
		{
		// Tiled picture
		case PICMENU_TILED:
			data->config->env_BackgroundFlags[which] &= ~(ENVBF_CENTER_PIC | ENVBF_STRETCH_PIC);
			break;

		// Centered picture
		case PICMENU_CENTERED:
			data->config->env_BackgroundFlags[which] |= ENVBF_CENTER_PIC;
			data->config->env_BackgroundFlags[which] &= ~ENVBF_STRETCH_PIC;
			break;

		// Stretched picture
		case PICMENU_STRETCHED:
			data->config->env_BackgroundFlags[which] |= ENVBF_STRETCH_PIC;
			data->config->env_BackgroundFlags[which] &= ~ENVBF_CENTER_PIC;
			break;

		// Precision
		case PICMENU_NONE:
		case PICMENU_GUI:
		case PICMENU_ICON:
		case PICMENU_IMAGE:
		case PICMENU_EXACT:
			data->config->env_BackgroundFlags[which] &=
				~(ENVBF_PRECISION_NONE | ENVBF_PRECISION_GUI | ENVBF_PRECISION_ICON | ENVBF_PRECISION_IMAGE |
				  ENVBF_PRECISION_EXACT);
			data->config->env_BackgroundFlags[which] |= 1 << (res - PICMENU_NONE + 1);
			break;

		// Normal border colour
		case PICMENU_NORMAL:
			data->config->env_BackgroundFlags[which] &= ~ENVBF_USE_COLOUR;
			break;

		// Black
		case PICMENU_BLACK:
			data->config->env_BackgroundFlags[which] |= ENVBF_USE_COLOUR;
			data->config->env_BackgroundBorderColour[which] = 0;
			break;

		// White
		case PICMENU_WHITE:
			data->config->env_BackgroundFlags[which] |= ENVBF_USE_COLOUR;
			data->config->env_BackgroundBorderColour[which] = 0xffffff00;
			break;
		}
	}

	// Free menu
	PopUpFreeHandle(menu);
}
