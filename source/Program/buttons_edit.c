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

#include <proto/configopus.h>

// Handles key press in edit mode
void buttons_edit_key(Buttons *buttons, UWORD code, UWORD qual)
{
	// Look at key
	switch (code)
	{
	// Cursor to move
	case CURSORLEFT:
	case CURSORRIGHT:
	case CURSORUP:
	case CURSORDOWN:

		// Is the editor running?
		if (buttons->editor)
		{
			// Send key to the editor
			IPC_Command(buttons->editor, BUTTONEDIT_PROCESS_KEY, code, (APTR)qual, 0, 0);
		}
		break;

	// Enter edits
	case 0x44:

		// Is the editor running?
		if (buttons->editor)
		{
			// Send edit command
			IPC_Command(
				buttons->editor, BUTTONEDIT_EDIT_BUTTON, buttons->editor_sel_col, (APTR)buttons->editor_sel_row, 0, 0);
		}
		break;
	}
}

// Make sure a button selector is visible
BOOL buttons_visible_select(Buttons *buttons)
{
	int old_left, old_top;

	// Anything selected?
	if (!buttons->button_sel_button)
		return 0;

	// Store old positions
	old_left = buttons->button_left;
	old_top = buttons->button_top;

	// Check selection column
	if (buttons->editor_sel_col < 0)
	{
		// Move to previous row
		buttons->editor_sel_col = buttons->bank->window.columns - 1;
		--buttons->editor_sel_row;
	}

	else if (buttons->editor_sel_col >= buttons->bank->window.columns)
	{
		// Move to next row
		buttons->editor_sel_col = 0;
		++buttons->editor_sel_row;
	}

	// Check selection row
	if (buttons->editor_sel_row < 0)
		buttons->editor_sel_row = 0;
	else if (buttons->editor_sel_row >= buttons->bank->window.rows)
		buttons->editor_sel_row = buttons->bank->window.rows - 1;

	// Do we need to scroll left?
	if (buttons->editor_sel_col < buttons->button_left)
		buttons->button_left = buttons->editor_sel_col;

	// Or right?
	else if (buttons->editor_sel_col >= buttons->button_left + buttons->button_cols)
		buttons->button_left = buttons->editor_sel_col - buttons->button_cols + 1;

	// Check left/right
	if (buttons->button_left > buttons->bank->window.columns - buttons->button_cols)
		buttons->button_left = buttons->bank->window.columns - buttons->button_cols;
	if (buttons->button_left < 0)
		buttons->button_left = 0;

	// Do we need to scroll up?
	if (buttons->editor_sel_row < buttons->button_top)
		buttons->button_top = buttons->editor_sel_row;

	// Or down?
	else if (buttons->editor_sel_row >= buttons->button_top + buttons->button_rows)
		buttons->button_top = buttons->editor_sel_row - buttons->button_rows + 1;

	// Check up/down
	if (buttons->button_top > buttons->bank->window.rows - buttons->button_rows)
		buttons->button_top = buttons->bank->window.rows - buttons->button_rows;
	if (buttons->button_top < 0)
		buttons->button_top = 0;

	// Fix slider positions
	if (buttons->horiz_scroll)
		SetGadgetAttrs(buttons->horiz_scroll, buttons->window, 0, PGA_Top, buttons->button_left, TAG_DONE);
	if (buttons->vert_scroll)
		SetGadgetAttrs(buttons->vert_scroll, buttons->window, 0, PGA_Top, buttons->button_top, TAG_DONE);

	// Did we have to scroll?
	return (BOOL)(buttons->button_left != old_left || buttons->button_top != old_top);
}

// Start dragging a button
void buttons_start_drag(Buttons *buttons, short x, short y, short col, short row)
{
	struct Rectangle rect;

	// Store column and row
	buttons->drag_col = buttons->button_sel_col;
	buttons->drag_row = buttons->button_sel_row;

	// Save button pointer
	buttons->button_drag_button = buttons->button_sel_button;

	// Get draginfo
	if (!(buttons->drag_info = GetDragInfo(
			  buttons->window, 0, buttons->button_width, buttons->button_height, DRAGF_NEED_GELS | DRAGF_CUSTOM)))
		return;

	// Get button coordinates
	buttons_get_rect(buttons, col, row, &rect);

	// Set opaque flag
	buttons->drag_info->flags |= DRAGF_OPAQUE;

	// Redraw button
	buttons_show_button(buttons,
						buttons->button_drag_button,
						buttons->drag_col,
						buttons->drag_row,
						buttons->button_drag_button->current,
						0);

	// Draw button image for drag
	SetDrMd(&buttons->drag_info->drag_rp, JAM1);
	SetFont(&buttons->drag_info->drag_rp, buttons->window->RPort->Font);
	buttons_show_button(buttons, buttons->button_drag_button, 0, 0, FTYPE_LEFT_BUTTON, &buttons->drag_info->drag_rp);

	// Calculate mask
	GetDragMask(buttons->drag_info);

	// Calculate offset
	buttons->drag_x_offset -= rect.MinX;
	buttons->drag_y_offset -= rect.MinY;

	// Initialise tick count
	buttons->tick_count = 1;
	buttons->last_tick = 0;

	// Show drag image
	buttons_show_drag(buttons, x, y);
}

// Show button drag image
void buttons_show_drag(Buttons *buttons, short x, short y)
{
	ShowDragImage(buttons->drag_info,
				  buttons->window->LeftEdge + (x - buttons->drag_x_offset),
				  buttons->window->TopEdge + (y - buttons->drag_y_offset));
}

// Stop dragging a button
void buttons_stop_drag(Buttons *buttons, short x, short y)
{
	struct Layer *layer;
	struct Window *window;
	ULONG id = 0;
	BOOL swap_local = 0;
	IPCData *ipc = 0;
	BOOL ok = 0;

	// Free drag info
	FreeDragInfo(buttons->drag_info);
	buttons->drag_info = 0;

	// Nothing selected now
	buttons->editor_sel_col = -1;
	buttons->editor_sel_row = -1;
	buttons->button_sel_button = 0;
	IPC_Command(buttons->editor, BUTTONEDIT_SELECT_BUTTON, -1, (APTR)-1, 0, 0);

	// Invalid coordinates?
	if (x == -1 || y == -1)
		return;

	// Lock layer
	LockScreenLayer(buttons->window->WScreen);

	// Find which layer we dropped it on
	if ((layer = WhichLayer(
			 &buttons->window->WScreen->LayerInfo, x + buttons->window->LeftEdge, y + buttons->window->TopEdge)))
	{
		// Does layer have a window?
		if ((window = layer->Window))
		{
			// Is it our own window?
			if (window == buttons->window)
			{
				// Set flag to swap with local button
				swap_local = 1;
			}
			else
			{
				// Get window ID
				id = GetWindowID(window);

				// Forbid to get IPC
				Forbid();
				if (!(ipc = (IPCData *)GetWindowAppPort(window)))
					Permit();
			}
		}
	}

	// Unlock layer
	UnlockScreenLayer(buttons->window->WScreen);

	// Got an IPC port?
	if (ipc)
	{
		// Button editor or another button bank?
		if (id == WINDOW_BUTTON_CONFIG || id == WINDOW_BUTTONS || id == WINDOW_FUNCTION_EDITOR)
		{
			Cfg_Button *button;
			Point *pos;

			// Store position (screen relative)
			if ((pos = AllocVec(sizeof(Point), 0)))
			{
				pos->x = x + buttons->window->LeftEdge;
				pos->y = y + buttons->window->TopEdge;
			}

			// Copy current button
			if ((button = CopyButton(buttons->button_drag_button, 0, -1)))
			{
				// Send button
				IPC_Command(ipc, BUTTONEDIT_CLIP_BUTTON, 0, button, pos, 0);
				ok = 1;
			}
		}

		// Permit now we've sent the message
		Permit();
	}

	// Swapping local buttons?
	else if (swap_local)
	{
		Cfg_Button *swap;

		// Lock bank
		GetSemaphore(&buttons->bank->lock, SEMF_SHARED, 0);

		// Get swap button
		if ((swap = button_from_point(buttons, &x, &y)))
		{
			// Different button?
			if (swap != buttons->button_drag_button)
			{
				// Swap buttons
				SwapListNodes(&buttons->bank->buttons, (struct Node *)buttons->button_drag_button, (struct Node *)swap);

				// Redraw buttons
				buttons_show_button(buttons, swap, buttons->drag_col, buttons->drag_row, swap->current, 0);
				buttons_show_button(
					buttons, buttons->button_drag_button, x, y, buttons->button_drag_button->current, 0);
				buttons->flags |= BUTTONF_CHANGED;
			}
		}

		// Unlock bank
		FreeSemaphore(&buttons->bank->lock);
		ok = 1;
	}

	// Failed?
	if (!ok)
		DisplayBeep(buttons->window->WScreen);
}

// Open the button editor (called from a sub-process)
void buttons_edit(IPCData *my_ipc, buttons_edit_packet *packet)
{
	ButtonsStartup startup;
#ifndef __amigaos3__
	struct Library *ConfigOpusBase;
	#ifdef __amigaos4__
	struct ConfigOpusIFace *IConfigOpus;
	#endif
#endif
	IPCData *ipc;
	long ret, command;
	Buttons *buttons = 0;
	Cfg_Button *send_button = 0;
	struct AppMessage *send_msg = 0;
	short button_col = -1, button_row = -1;
	short can_start = 1;

	// Get packet data
	if (packet)
	{
		buttons = packet->buttons;
		send_button = packet->edit;
		button_col = packet->col;
		button_row = packet->row;
		can_start = packet->can_start;
		send_msg = packet->appmsg;
	}

	// Lock process list
	lock_listlock(&GUI->process_list, FALSE);

	// Get edit command
	command = (button_col == -2) ? BUTTONEDIT_RE_EDIT_ME : BUTTONEDIT_EDIT_ME;

	// See if button editor is running
	if ((ipc = IPC_FindProc(&GUI->process_list, NAME_BUTTON_EDITOR_RUN, FALSE, 0)))
	{
		BOOL front = 1;

		// Button bank supplied?
		if (buttons)
		{
			// Does the bank not already have the editor?
			if (!buttons->editor)
			{
				// Tell editor to edit the toolbar bank
				IPC_Command(ipc, command, (ULONG)buttons->bank, buttons->ipc, 0, 0);

				// Set flag for pending edit request
				buttons->flags |= BUTTONF_EDIT_REQUEST;
				front = 0;
			}

			// Button to send?
			if (send_button)
			{
				Point *pos;

				// Allocate position
				if ((pos = AllocVec(sizeof(Point), MEMF_CLEAR)))
				{
					// Convert coordinates to window relative
					pos->x = button_col - buttons->window->LeftEdge;
					pos->y = button_row - buttons->window->TopEdge;

					// Send command
					IPC_Command(ipc, BUTTONEDIT_BUTTON_TO_BANK, 0, send_button, pos, 0);
					send_button = 0;
				}
				front = 0;
			}

			// Message to send?
			if (send_msg)
			{
				// Send it on
				IPC_Command(ipc, CFG_APPMESSAGE_PASS, (ULONG)buttons->bank, send_msg, 0, 0);
				send_msg = 0;
			}

			// Button to edit?
			if (button_col > -1)
			{
				// Send edit command
				IPC_Command(ipc, BUTTONEDIT_EDIT_BUTTON, button_col, (APTR)button_row, 0, 0);
				front = 0;
			}
		}

		// Bring editor to front
		if (front)
			IPC_Command(ipc, IPC_ACTIVATE, 0, 0, 0, 0);
	}

	// Unlock the process list
	unlock_listlock(&GUI->process_list);

	// Free button if we have one
	if (send_button)
		FreeButton(send_button);

	// Free message if we have one
	if (send_msg)
		ReplyAppMessage((DOpusAppMessage *)send_msg);

	// If editor was already running, or we can't start it if not, return
	if (ipc || !can_start)
		return;

	// Change our name
	my_ipc->proc->pr_Task.tc_Node.ln_Name = NAME_BUTTON_EDITOR_RUN;

	// Set flag
	GUI->flags |= GUIF_BUTTON_EDITOR;

	// Open configuration library
	if (!(ConfigOpusBase = OpenModule(config_name))
#ifdef __amigaos4__
		|| !(IConfigOpus = (struct ConfigOpusIFace *)GetInterface(ConfigOpusBase, "main", 1, NULL))
#endif
	)
	{
		if (ConfigOpusBase)
			CloseLibrary(ConfigOpusBase);
		return;
	}

	// Bank supplied?
	if (buttons)
	{
		// Fill out startup packet
		startup.bank = buttons->bank;
		startup.ipc = buttons->ipc;

		// Button supplied?
		if (button_col > -1)
		{
			// Set flag
			startup.flag = 1;

			// Pass button
			startup.button = (button_col << 16) | button_row;
		}
		else
			startup.flag = 0;

		// Set pending edit request
		buttons->flags |= BUTTONF_EDIT_REQUEST;
	}
	else
		startup.bank = 0;

	// Configure buttons
	ret = Config_Buttons(&startup, my_ipc, &main_ipc, GUI->screen_pointer, (ULONG)&GUI->command_list.list);

	// Change our name back
	my_ipc->proc->pr_Task.tc_Node.ln_Name = NAME_BUTTON_EDITOR;

	// Clear flag
	GUI->flags &= ~GUIF_BUTTON_EDITOR;

	// Permit now we've cleaned up
	Permit();

// Close library
#ifdef __amigaos4__
	DropInterface((struct Interface *)IConfigOpus);
#endif
	CloseLibrary(ConfigOpusBase);

	// Lock buttons list
	lock_listlock(&GUI->buttons_list, FALSE);

	// Go through button banks
	for (ipc = (IPCData *)GUI->buttons_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Get buttons pointer
		Buttons *buttons = IPCDATA(ipc);

		// Toolbar buttons?
		if (buttons->flags & BUTTONF_TOOLBAR)
		{
			// Hide bank
			IPC_Command(ipc, IPC_HIDE, 0, 0, 0, REPLY_NO_PORT);

			// Need to save?
			if (ret & CONFIG_SAVE)
				IPC_Command(ipc, BUTTONS_SAVE, 0, 0, 0, REPLY_NO_PORT);

			// Use?
			if (ret)
			{
				// Send change
				send_main_reset_cmd(CONFIG_CHANGE_LIST_TOOLBAR, 0, buttons->bank);

				// Steal bank pointer
				IPC_Command(ipc, BUTTONEDIT_NEW_BANK, 1, 0, 0, REPLY_NO_PORT);
			}

			// Close bank
			IPC_Command(ipc, IPC_QUIT, 0, 0, 0, 0);
		}

		// Ok?
		else if (ret)
		{
			// Save?
			if (ret & CONFIG_SAVE)
			{
				// Buttons been changed?
				if (buttons->flags & BUTTONF_CHANGED)
				{
					// Tell bank to save itself
					IPC_Command(ipc, BUTTONS_SAVE, 0, 0, 0, REPLY_NO_PORT);
				}
			}

			// Clear 'new' flag
			buttons->flags &= ~BUTTONF_NEW_BANK;
		}
	}

	// Unlock buttons list
	unlock_listlock(&GUI->buttons_list);
}

// Edit a bank
BOOL buttons_edit_bank(Buttons *buttons,
					   short col,
					   short row,
					   Cfg_Button *edit,
					   struct AppMessage *appmsg,
					   short can_start)
{
	buttons_edit_packet *packet;

	// Allocate packet
	if ((packet = AllocVec(sizeof(buttons_edit_packet), MEMF_CLEAR)))
	{
		// Fill out packet
		packet->buttons = buttons;
		packet->col = col;
		packet->row = row;
		packet->edit = edit;
		packet->can_start = can_start;
		packet->appmsg = appmsg;

		// Edit it
		if (misc_startup(NAME_BUTTON_EDITOR, MENU_EDIT_BUTTONS, buttons->window, packet, FALSE))
		{
			// Set flag for pending edit request
			buttons->flags |= BUTTONF_EDIT_REQUEST;
			return 1;
		}

		// Failed
		FreeVec(packet);
	}

	return 0;
}

// Save from button editor
void buttons_edit_save(ULONG id)
{
	IPCData *ipc;

	// Lock buttons list
	lock_listlock(&GUI->buttons_list, FALSE);

	// Go through button banks
	for (ipc = (IPCData *)GUI->buttons_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Get buttons pointer
		Buttons *buttons = IPCDATA(ipc);

		// Is this the bank being edited?
		if (buttons->editor)
		{
			// Send command to button bank
			IPC_Command(buttons->ipc, (id == BUTTONEDIT_MENU_SAVE) ? BUTTONS_SAVE : BUTTONS_SAVEAS, 0, 0, 0, 0);
			break;
		}
	}

	// Unlock buttons list
	unlock_listlock(&GUI->buttons_list);
}

// Save from button editor
void buttons_edit_defaults(ULONG id)
{
	IPCData *ipc;

	// Lock buttons list
	lock_listlock(&GUI->buttons_list, FALSE);

	// Go through button banks
	for (ipc = (IPCData *)GUI->buttons_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Get buttons pointer
		Buttons *buttons = IPCDATA(ipc);

		// Is this the bank being edited, and is it a toolbar?
		if (buttons->editor && buttons->flags & BUTTONF_TOOLBAR)
		{
			// Send command to button bank
			IPC_Command(buttons->ipc, id, 0, 0, 0, 0);
			break;
		}
	}

	// Unlock buttons list
	unlock_listlock(&GUI->buttons_list);
}
