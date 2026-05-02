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

#include "dopus.h"

// Do popup
void buttons_do_popup(Buttons *buttons, UWORD code)
{
	if (buttons_button_popup(buttons, code))
	{
		// Get new setting
		buttons->button_sel_which = buttons->button_sel_button->current;

		// Show button
		buttons_highlight_button(buttons, 0, 1);

		// Clear button pointer
		buttons->button_sel_button = 0;
		buttons->button_sel_code = 0;
	}
}

BOOL buttons_button_popup(Buttons *buttons, UWORD code)
{
	Cfg_Button *button;
	Cfg_ButtonFunction *func;
	PopUpHandle *menu;
	PopUpItem *item;
	short count, num, ret = 0, spec = 0;
	long checkit;

	// Get button pointer
	if (!(button = buttons->button_sel_button))
		return 0;

	// Get menu handle
	if (!(menu = PopUpNewHandle((IPTR)buttons, GET_CALLBACK(buttons_refresh_callback), &locale)))
		return 0;

	// Backfill hook?
	if (buttons->pattern_data.valid)
	{
		menu->ph_Menu.flags |= POPUPMF_BACKFILL;
		menu->ph_Menu.backfill = (struct Hook *)&buttons->pattern;
	}

	// Only use checkmarks if active popups not set
	checkit = (!(buttons->bank->window.flags & BTNWF_ACTIVE_POPUP)) ? POPUPF_CHECKIT : 0;

	// Go through button functions
	for (func = (Cfg_ButtonFunction *)button->function_list.mlh_Head, count = 0, num = 0; func->node.ln_Succ;
		 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
	{
		// Valid function?
		if (!(IsListEmpty((struct List *)&func->instructions)))
		{
			// Get menu item
			if ((item = PopUpNewItem(menu,
									 (IPTR)((func->node.ln_Name) ? func->node.ln_Name : func->label),
									 0,
									 checkit | POPUPF_STRING)))
			{
				// Fill out item
				item->data = func;
				item->flags = checkit;

				// Special function?
				if (func->function.func_type == FTYPE_LIST)
				{
					// Set ID appropriately
					item->id = FTYPE_SPECIAL_FLAG | num;
					spec = 1;
				}

				// Normal function
				else
					item->id = func->function.func_type;

				// Is this the current selection?
				if (checkit && button->current == item->id)
					item->flags |= POPUPF_CHECKED;
				++count;
			}
		}

		// Increment count for special functions
		if (func->function.func_type == FTYPE_LIST)
			num++;
	}

	// If there's only one function in the list, don't bother
	if (count > 1 || spec)
	{
		unsigned short res;

		// Do popup menu
		if ((res = DoPopUpMenu(buttons->window, &menu->ph_Menu, 0, code)) != (UWORD)-1)
		{
			// Help?
			if (res & POPUP_HELPFLAG)
			{
				// Show help for function
				help_button_help(button, 0, 0xffff, res & ~POPUP_HELPFLAG, GENERIC_BUTTON);
			}

			// Store new default function if not active popup
			else if (checkit)
				button->current = res;

			// For an active popup, launch the function
			else
				buttons_run_button(buttons, button, res);
		}

		ret = 1;
	}

	// Free data and return
	PopUpFreeHandle(menu);
	return ret;
}

// Popup over border
UWORD button_border_popup(Buttons *buttons)
{
	PopUpHandle *menu;
	UWORD res;

	// Create menu
	if (!(menu = PopUpNewHandle((IPTR)buttons, GET_CALLBACK(buttons_refresh_callback), &locale)))
		return 0;

	// Backfill hook?
	if (buttons->pattern_data.valid)
	{
		menu->ph_Menu.flags |= POPUPMF_BACKFILL;
		menu->ph_Menu.backfill = (struct Hook *)&buttons->pattern;
	}

	// Build button popup
	PopUpNewItem(menu, MSG_LISTER_ICONIFY, MENU_ICONIFY_BUTTONS, 0);
	PopUpSeparator(menu);
	PopUpNewItem(menu, MSG_SAVE_MENU, MENU_SAVE_BUTTONS, 0);
	PopUpNewItem(menu, MSG_SAVEAS_MENU, MENU_SAVEAS_BUTTONS, 0);
	PopUpSeparator(menu);
	PopUpNewItem(menu, MSG_CLOSE_BUTTONS_MENU, MENU_CLOSE_BUTTONS, 0);
	PopUpSeparator(menu);
	PopUpNewItem(menu, MSG_EDIT_MENU, MENU_EDIT_BUTTONS, 0);

	// Do pop-up menu
	res = DoPopUpMenu(buttons->window, &menu->ph_Menu, 0, MENUDOWN);

	// Free menu and return result
	PopUpFreeHandle(menu);
	return res;
}
