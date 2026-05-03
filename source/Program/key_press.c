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
#include "key_finder.h"
#include <proto/configopus.h>

// Match a key press against a function
// Really should have some locking in here!
Cfg_Function *match_function_key(UWORD code,
								 UWORD qual,
								 Cfg_ButtonBank *bank,
								 Lister *lister,
								 ULONG window_id,
								 ULONG *match_id)
{
	Cfg_Function *function = 0;
	IPCData *ipc;

	// No match so far
	if (match_id)
		*match_id = KEYMATCH_NONE;

	// Ignore key up and repeat
	if ((code & IECODE_UP_PREFIX) || (qual & IEQUALIFIER_REPEAT))
		return 0;

	// Strip invalid qualifiers
	qual = QualValid(qual);

	// If there's a bank supplied, test that first
	if (bank && (function = match_function_key_list(code, qual, bank, FALSE)))
	{
		if (match_id)
			*match_id = KEYMATCH_BUTTONS;
		return function;
	}

	// If this event is from a lister, check some special things
	if (window_id == WINDOW_LISTER)
	{
		// Check toolbar first
		if (lister && lister->toolbar &&
			(function = match_function_key_list(code, qual, lister->toolbar->buttons, FALSE)))
		{
			if (match_id)
				*match_id = KEYMATCH_TOOLBAR;
			return function;
		}

		// Check lister menu
		if ((function = match_function_key_list(code, qual, GUI->lister_menu, FALSE)))
		{
			if (match_id)
				*match_id = KEYMATCH_LISTERMENU;
			return function;
		}
	}

	// Got user menu?
	if (GUI->user_menu)
	{
		// Lock user menu
		GetSemaphore(&GUI->user_menu_lock, SEMF_SHARED, 0);

		// Match key against user menu
		function = match_function_key_list(code, qual, GUI->user_menu, FALSE);

		// Unlock menu (this should be protected)
		FreeSemaphore(&GUI->user_menu_lock);

		// Return function
		if (function)
		{
			if (match_id)
				*match_id = KEYMATCH_MENU;
			return function;
		}
	}

	// Lock button bank list
	lock_listlock(&GUI->buttons_list, 0);

	// Go through button banks
	for (ipc = (IPCData *)GUI->buttons_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		Buttons *buttons;

		// Get buttons pointer
		buttons = IPCDATA(ipc);

		// Check against this bank
		if ((function = match_function_key_list(code, qual, buttons->bank, FALSE)))
			break;
	}

	// Unlock button bank list
	unlock_listlock(&GUI->buttons_list);

	// Got a function?
	if (function)
	{
		if (match_id)
			*match_id = KEYMATCH_BUTTONS;
		return function;
	}

	// Lock start menu list
	lock_listlock(&GUI->startmenu_list, 0);

	// Go through start menus
	for (ipc = (IPCData *)GUI->startmenu_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		StartMenu *menu;

		// Get menu pointer
		menu = IPCDATA(ipc);

		// Check against this bank
		if ((function = match_function_key_list(code, qual, menu->bank, FALSE)))
			break;
	}

	// Unlock start menu list
	unlock_listlock(&GUI->startmenu_list);

	// Got a function?
	if (function)
	{
		if (match_id)
			*match_id = KEYMATCH_START;
		return function;
	}

	// If not from a lister, check toolbar and lister menu now
	if (window_id != WINDOW_LISTER)
	{
		// Check toolbar first
		if (GUI->toolbar && (function = match_function_key_list(code, qual, GUI->toolbar->buttons, FALSE)))
		{
			if (match_id)
				*match_id = KEYMATCH_TOOLBAR;
			return function;
		}

		// Check lister menu
		if ((function = match_function_key_list(code, qual, GUI->lister_menu, FALSE)))
			if (match_id)
				*match_id = KEYMATCH_LISTERMENU;
	}

	// If no function, check local hotkeys
	if (!function && GUI->hotkeys)
	{
		// Lock hotkey list
		GetSemaphore(&GUI->hotkeys_lock, SEMF_SHARED, 0);

		// Check hotkeys
		if ((function = match_function_key_list(code, qual, GUI->hotkeys, (match_id) ? FALSE : TRUE)))
			if (match_id)
				*match_id = KEYMATCH_HOTKEYS;

		// Unlock hotkey list
		FreeSemaphore(&GUI->hotkeys_lock);
	}

	// If nothing yet, check scripts
	if (!function && GUI->scripts)
	{
		// Lock scripts list
		GetSemaphore(&GUI->scripts_lock, SEMF_SHARED, 0);

		// Check scripts
		if ((function = match_function_key_list(code, qual, GUI->scripts, TRUE)))
			if (match_id)
				*match_id = KEYMATCH_SCRIPTS;

		// Unlock scripts list
		FreeSemaphore(&GUI->scripts_lock);
	}

	// Return what we got (if anything)
	return function;
}

// See if a keypress matches a function
Cfg_Function *match_function_key_list(UWORD code, UWORD qual, Cfg_ButtonBank *bank, BOOL global_check)
{
	Cfg_Button *button;
	Cfg_Function *function;

	// Valid bank?
	if (!bank)
		return 0;

	// Go through button list
	for (button = (Cfg_Button *)bank->buttons.lh_Head; button->node.ln_Succ;
		 button = (Cfg_Button *)button->node.ln_Succ)
	{
		// Must not be global?
		if (global_check && (button->button.flags & BUTNF_GLOBAL))
			continue;

		// Go through functions in button
		for (function = (Cfg_Function *)button->function_list.mlh_Head; function->node.ln_Succ;
			 function = (Cfg_Function *)function->node.ln_Succ)
		{
			// Does key match?
			if (code == function->function.code)
			{
				// Does qualifier match?
				if (check_qualifier(
						qual, function->function.qual, function->function.qual_mask, function->function.qual_same))
				{
					// Yep!
					return function;
				}
			}
		}
	}

	// Nothing
	return 0;
}

// Key Finder
void key_finder(IPCData *ipc)
{
	struct Window *window;
	ObjectList *objlist;
	NewConfigWindow newwin;
	Att_List *func_list;
	BOOL activate = 1;

	// Fill out NewWindow
	newwin.parent = GUI->screen_pointer;
	newwin.dims = &keyfinder_window;
	newwin.title = GetString(&locale, MSG_KEYFINDER_TITLE);
	newwin.locale = &locale;
	newwin.port = 0;
	newwin.flags = WINDOW_SCREEN_PARENT | WINDOW_AUTO_KEYS | WINDOW_VISITOR;
	newwin.font = 0;

	// Allocate function list
	if (!(func_list = Att_NewList(LISTF_POOL)))
		return;

	// Show window
	if (!(window = OpenConfigWindow(&newwin)) || !(objlist = AddObjectList(window, keyfinder_objects)))
	{
		// Failed
		CloseConfigWindow(window);
		Att_RemList(func_list, 0);
		return;
	}

	// Set window ID
	SetWindowID(window, 0, WINDOW_KEYFINDER, 0);

	// Set flag to show key finder is active
	GUI->flags2 |= GUIF2_KEY_FINDER;

	// Event loop
	FOREVER
	{
		struct IntuiMessage *msg;
		IPCMessage *imsg;
		BOOL quit_flag = 0, process = 0;

		// Get window messages
		while ((msg = GetWindowMsg(window->UserPort)))
		{
			struct IntuiMessage msg_copy;

			// Copy message and reply
			msg_copy = *msg;
			ReplyWindowMsg(msg);

			// Look at message
			switch (msg_copy.Class)
			{
			// Close Window?
			case IDCMP_CLOSEWINDOW:
				quit_flag = 1;
				break;

			// Gadget press
			case IDCMP_GADGETUP:

				// Key field?
				if (((struct Gadget *)msg_copy.IAddress)->GadgetID == GAD_KEYFINDER_KEY)
					process = 1;
				break;
			}
		}

		// Get IPC messages
		while ((imsg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Hide or Quit will quit
			if (imsg->command == IPC_HIDE || imsg->command == IPC_QUIT)
				quit_flag = 1;

			// Activate
			else if (imsg->command == IPC_ACTIVATE)
			{
				ActivateWindow(window);
				WindowToFront(window);
			}

			// Key event
			else if (imsg->command == KFIPC_KEYCODE)
			{
				UWORD code, qual, qual_mask, qual_same;
				char buf[128];

				// Get code and qualifier
				code = imsg->flags >> 16;
				qual = imsg->flags & 0xffff;
				qual_mask = ((IPTR)imsg->data) >> 16;
				qual_same = ((IPTR)imsg->data) & 0xffff;

				// Build key string
				BuildKeyString(code, qual, qual_mask, qual_same, buf);

				// Set string value
				SetGadgetValue(objlist, GAD_KEYFINDER_KEY, (IPTR)buf);

				// Set flag to activate key field
				activate = 1;
			}

			// Reply to message
			IPC_Reply(imsg);
		}

		// Quit?
		if (quit_flag)
			break;

		// Process event
		if (process)
		{
			IX ix;
			char *ptr;
			Cfg_Function *function = 0;
			ULONG match_type = KEYMATCH_INVALID;

			// Make window busy
			SetWindowBusy(window);

			// Get key, see if it's invalid, parse it
			if (!(ptr = (char *)GetGadgetValue(objlist, GAD_KEYFINDER_KEY)) || !*ptr || ParseIX(ptr, &ix))
			{
				// Error
				DisplayBeep(window->WScreen);
			}

			// Success, find function
			else
				function = match_function_key(ix.ix_Code, ix.ix_Qualifier, 0, 0, WINDOW_UNKNOWN, &match_type);

			// Get type string
			ptr = GetString(&locale, MSG_KEYFINDER_TYPE_INVALID + match_type);

			// Set type string
			SetGadgetValue(objlist, GAD_KEYFINDER_FOUND, (IPTR)ptr);

			// Remove function list
			SetGadgetChoices(objlist, GAD_KEYFINDER_FUNCTION, (APTR)-1);

			// Clear function list
			Att_RemList(func_list, REMLIST_SAVELIST);

			// Got a function?
			if (function)
			{
#ifndef __amigaos3__
				struct Library *ConfigOpusBase;
	#ifdef __amigaos4__
				struct ConfigOpusIFace *IConfigOpus;
	#endif
#endif
				// Get config library
				if ((ConfigOpusBase = OpenModule(config_name))
#ifdef __amigaos4__
					&& (IConfigOpus = (struct ConfigOpusIFace *)GetInterface(ConfigOpusBase, "main", 1, NULL))
#endif
				)
				{
					// Export function as ASCII to temporary file
					if (FunctionExportASCII("t:keyfinder.tmp", 0, function, 0))
					{
						APTR in;

						// Open exported file
						if ((in = OpenBuf("t:keyfinder.tmp", MODE_OLDFILE, 2048)))
						{
							char line[512];

							// Read lines from file
							while (ReadBufLine(in, line, 512) >= 0)
							{
								// Add node to function list
								Att_NewNode(func_list, line, 0, 0);
							}

							// Close file
							CloseBuf(in);
						}

						// Delete temporary file
						DeleteFile("t:keyfinder.tmp");
					}

// Close library
#ifdef __amigaos4__
					DropInterface((struct Interface *)IConfigOpus);
#endif
					CloseLibrary(ConfigOpusBase);
				}
				else
					CloseLibrary(ConfigOpusBase);  // In case module opens & interface doesn't
			}

			// Add function list to gadget
			SetGadgetChoices(objlist, GAD_KEYFINDER_FUNCTION, func_list);

			// Make window unbusy
			ClearWindowBusy(window);

			// Set flag to activate key field
			activate = 1;
		}

		// Activate key field
		if (activate)
			ActivateStrGad(GADGET(GetObject(objlist, GAD_KEYFINDER_KEY)), window);

		// Wait for event
		Wait(1 << window->UserPort->mp_SigBit | 1 << ipc->command_port->mp_SigBit);
	}

	// Clear flag to show key finder is gone
	GUI->flags2 &= ~GUIF2_KEY_FINDER;

	// Close window, free list
	CloseConfigWindow(window);
	Att_RemList(func_list, 0);
}
