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

enum {
	ARG_NAME,
	ARG_ALL,
	ARG_ICONIFY,
	ARG_START,
	ARG_HIDE,
};

// CLOSEBUTTONS function
DOPUS_FUNC(function_closebuttons)
{
	char *close_bank = 0;
	BOOL close_all = 0, iconify = 0, start = 0, hide = 0;
	Buttons *bank = 0;
	IPCData *ipc;

	// Arguments?
	if (instruction->funcargs)
	{
		// Close all?
		if (instruction->funcargs->FA_Arguments[ARG_ALL])
			close_all = 1;

		// Specific bank
		else if (instruction->funcargs->FA_Arguments[ARG_NAME])
			close_bank = (char *)instruction->funcargs->FA_Arguments[ARG_NAME];

		// Iconify?
		if (instruction->funcargs->FA_Arguments[ARG_ICONIFY])
			iconify = 1;

		// Hide
		else if (instruction->funcargs->FA_Arguments[ARG_HIDE])
			hide = 1;

		// Start menu?
		if (instruction->funcargs->FA_Arguments[ARG_START])
			start = 1;
	}

	// Start Menu?
	if (start)
	{
		// Lock startmenu list
		lock_listlock(&GUI->startmenu_list, FALSE);

		// Go through start menus
		for (ipc = (IPCData *)GUI->startmenu_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
		{
			BOOL match = 0;

			// Closing all?
			if (close_all)
				match = 1;

			// Or does it match the name?
			else if (close_bank)
			{
				Cfg_Button *button;
				StartMenu *menu = (StartMenu *)IPCDATA(ipc);

				// Go through buttons in menu
				for (button = (Cfg_Button *)menu->bank->buttons.lh_Head; button->node.ln_Succ;
					 button = (Cfg_Button *)button->node.ln_Succ)
				{
					// Button title?
					if (button->button.flags & BUTNF_TITLE)
					{
						Cfg_ButtonFunction *func;

						// Get function
						if (!(IsListEmpty((struct List *)&button->function_list)))
						{
							// Get first function
							func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

							// Compare name
							if (func->label && stricmp(close_bank, func->label) == 0)
								match = 1;
						}

						break;
					}
				}

				// Or the filename?
				if (!match && stricmp(close_bank, FilePart(menu->bank->path)) == 0)
					match = 1;
			}

			// Match menu to close?
			if (match)
			{
				// Send quit message
				IPC_Quit(ipc, 0, FALSE);
			}
		}

		// Unlock menu list
		unlock_listlock(&GUI->startmenu_list);
		return 1;
	}

	// Close current bank?
	if (!close_all && !close_bank)
	{
		// If we don't have one, return
		if (!(bank = handle->buttons))
			return 1;
	}

	// Lock buttons list
	lock_listlock(&GUI->buttons_list, FALSE);

	// Look for bank in list
	for (ipc = (IPCData *)GUI->buttons_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		BOOL match = 0;
		Buttons *buttons = (Buttons *)IPCDATA(ipc);

		// Closing all?
		if (close_all)
			match = 1;

		// Does this match our bank pointer?
		else if (bank && bank == buttons)
			match = 1;

		// Or does it match the name?
		else if (close_bank && stricmp(close_bank, buttons->bank->window.name) == 0)
			match = 1;

		// Or the filename?
		else if (close_bank && stricmp(close_bank, FilePart(buttons->buttons_file)) == 0)
			match = 1;

		// Match bank to close?
		if (match)
		{
			// Iconify?
			if (iconify)
				IPC_Command(ipc, BUTTONS_ICONIFY, 0, 0, 0, 0);

			// Hide?
			else if (hide)
				IPC_Command(ipc, IPC_HIDE, 0, 0, 0, 0);

			// Send quit message
			else
				IPC_Quit(ipc, 0, FALSE);

			// If we were given a specific bank, break out
			if (bank)
				break;
		}
	}

	// Unlock buttons list
	unlock_listlock(&GUI->buttons_list);

	return 1;
}
