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

// Do a bank-specific function
int buttons_do_function(Buttons *buttons, IPTR func)
{
	int result = 1;

	// Look at function
	switch (func)
	{
	// New/Open/Defaults/Last Saved
	case MENU_TOOLBAR_BUTTONS_NEW:
	case MENU_OPEN_BUTTONS_LOCAL:
	case MENU_TOOLBAR_RESET_DEFAULTS:
	case MENU_TOOLBAR_LAST_SAVED:
		buttons_new_bank(buttons, func, 0);
		break;

	// Restore
	case MENU_TOOLBAR_RESTORE:
		buttons_new_bank(buttons, func, buttons->backup);
		break;

	// Save
	case MENU_SAVE_BUTTONS: {
		BOOL ok = 0;

		// Lock bank
		GetSemaphore(&buttons->bank->lock, SEMF_SHARED, 0);

		// Valid path?
		if (buttons->bank->path[0])
		{
			buttons_save(buttons, buttons->bank->path);
			ok = 1;
		}

		// Unlock bank
		FreeSemaphore(&buttons->bank->lock);
		if (ok)
			break;
	}

	// Save as
	case MENU_SAVEAS_BUTTONS:
		GetSemaphore(&buttons->bank->lock, SEMF_SHARED, 0);
		buttons_saveas(buttons);
		FreeSemaphore(&buttons->bank->lock);
		break;

	// Close
	case MENU_CLOSE_BUTTONS:
		if (buttons_check_change(buttons, 1))
			result = 0;
		break;

	// Iconify
	case MENU_ICONIFY_BUTTONS:
		if (buttons_iconify(buttons))
			buttons_close(buttons, 0);
		break;

	// Don't understand it
	default:
		result = -1;
		break;
	}

	return result;
}

// Highlight button
void buttons_highlight_button(Buttons *buttons, short state, short do_corners)
{
	short ret = 0;

	// Redraw imagery
	if (do_corners != -1 && (state == 1 || !(buttons->flags & BUTTONF_NO_ALTIMAGE)))
	{
		if (!(ret = buttons_draw_button(buttons,
										buttons->button_sel_button,
										buttons->button_sel_col,
										buttons->button_sel_row,
										buttons->button_sel_which,
										state,
										0)) &&
			state && 0)
			buttons->flags |= BUTTONF_NO_ALTIMAGE;
	}
	else
		buttons->flags &= ~BUTTONF_NO_ALTIMAGE;

	// Redraw border
	if (!(buttons_draw_border(buttons,
							  buttons->button_sel_button,
							  buttons->button_sel_col,
							  buttons->button_sel_row,
							  state,
							  do_corners,
							  0)) &&
		!ret && state)
	{
		short x, y;

		// We must be borderless with no alternate imagery
		// Calculate column/row to highlight
		x = buttons->button_sel_col - buttons->button_left;
		y = buttons->button_sel_row - buttons->button_top;

		// See if button is visible
		if (x >= 0 && x < buttons->button_cols && y >= 0 && y < buttons->button_rows)
		{
			// Get position
			x = buttons->internal.Left + x * buttons->button_width;
			y = buttons->internal.Top + y * buttons->button_height;

			// Set Complement mode
			SetDrMd(buttons->window->RPort, COMPLEMENT);

			// Highlight button
			RectFill(buttons->window->RPort, x, y, x + buttons->button_width - 1, y + buttons->button_height - 1);

			// Set back to JAM1
			SetDrMd(buttons->window->RPort, JAM1);
		}
	}
}

// Load a new bank into open buttons window
void buttons_new_bank(Buttons *buttons, short func, Cfg_ButtonBank *use_bank)
{
	Cfg_ButtonBank *bank;
	IPCData *ipc;

	// Check for change
	if (!buttons_check_change(buttons, 1))
		return;

	// Save old bank pointer
	bank = buttons->bank;

	// New?
	if (func == MENU_TOOLBAR_BUTTONS_NEW)
	{
		// Create new button bank
		if (!(buttons->bank = NewButtonBank(1, 1)))
		{
			// Restore old pointer
			buttons->bank = bank;
			return;
		}

		// Free old bank
		CloseButtonBank(bank);
	}

	// Load?
	else if (func == MENU_OPEN_BUTTONS_LOCAL)
	{
		// Open button bank
		if (!(buttons_load(buttons, GUI->screen_pointer, 0)))
			return;
	}

	// Defaults
	else if (func == MENU_TOOLBAR_RESET_DEFAULTS)
	{
		char *paths[] = {
			environment->toolbar_path,
			"PROGDIR:Buttons/Toolbar",
			"PROGDIR:Buttons/Toolbar.default",
			"PROGDIR:Buttons/toolbar_default",
			"DOpus5:Buttons/Toolbar",
			"DOpus5:Buttons/Toolbar.default",
			"DOpus5:Buttons/toolbar_default",
			0,
		};
		short a;

		// Open button bank
		for (a = 0; paths[a]; a++)
		{
			if (paths[a][0] && buttons_load(buttons, GUI->screen_pointer, paths[a]))
				break;
		}
		if (!paths[a])
			return;
	}

	// Restore
	else if (func == MENU_TOOLBAR_RESTORE)
	{
		// Copy backup bank
		if (!(buttons->bank = CopyButtonBank(buttons->backup)))
		{
			// Restore old pointer
			buttons->bank = bank;
			return;
		}

		// Free old bank
		CloseButtonBank(bank);
	}

	// Last saved
	else if (buttons->last_saved[0])
	{
		// Open button bank
		if (!(buttons_load(buttons, GUI->screen_pointer, buttons->last_saved)))
			return;
	}

	// Lock process list
	lock_listlock(&GUI->process_list, FALSE);

	// Find editor
	if ((ipc = IPC_FindProc(&GUI->process_list, NAME_BUTTON_EDITOR_RUN, 0, 0)))
	{
		// Set flag in bank
		buttons->bank->window.flags |= BTNWF_TOOLBAR;

		// Tell editor to change bank pointer
		IPC_Command(buttons->editor, BUTTONEDIT_CHANGE_BANK, (IPTR)bank, buttons->bank, 0, REPLY_NO_PORT);
	}

	// Unlock the process list
	unlock_listlock(&GUI->process_list);

	// Signal for re-open
	IPC_Command(buttons->ipc, BUTTONEDIT_REOPEN, 0, 0, 0, 0);

	// Clear change flag
	buttons->flags &= ~BUTTONF_CHANGED;
}
