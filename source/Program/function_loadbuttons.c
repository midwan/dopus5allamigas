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
	ARG_START,
	ARG_LABEL,
	ARG_IMAGE,
	ARG_UNDERMOUSE,
	ARG_TOGGLE,
	ARG_HIDDEN,
	ARG_SHOW,
};

// LOADBUTTONS internal function
DOPUS_FUNC(function_loadbuttons)
{
	FunctionEntry *entry;
	BOOL start = 0, hidden = 0;
	ULONG buttonflags = BUTTONF_FAIL;
	char label[40], image[256];

	// Clear buffers
	label[0] = 0;
	image[0] = 0;

	// Args?
	if (instruction->funcargs)
	{
		// Start?
		if (instruction->funcargs->FA_Arguments[ARG_START])
		{
			// Set flag
			start = 1;

			// Get label
			if (instruction->funcargs->FA_Arguments[ARG_LABEL])
				stccpy(label, (char *)instruction->funcargs->FA_Arguments[ARG_LABEL], 39);

			// Get image
			if (instruction->funcargs->FA_Arguments[ARG_IMAGE])
				stccpy(image, (char *)instruction->funcargs->FA_Arguments[ARG_IMAGE], 255);
		}

		// Open under mouse?
		if (instruction->funcargs->FA_Arguments[ARG_UNDERMOUSE])
			buttonflags |= BUTTONF_UNDERMOUSE;

		// Open hidden
		if (instruction->funcargs->FA_Arguments[ARG_HIDDEN])
			hidden = 1;

		// Toggle/Show?
		if (!start &&
			(instruction->funcargs->FA_Arguments[ARG_TOGGLE] || instruction->funcargs->FA_Arguments[ARG_SHOW]))
		{
			IPCData *ipc;
			BOOL matched = 0;

			// Get first entry
			if ((entry = function_get_entry(handle)))
			{
				// Lock buttons list
				lock_listlock(&GUI->buttons_list, FALSE);

				// Look for bank in list
				for (ipc = (IPCData *)GUI->buttons_list.list.lh_Head; ipc->node.mln_Succ;
					 ipc = (IPCData *)ipc->node.mln_Succ)
				{
					Buttons *buttons = (Buttons *)IPCDATA(ipc);
					BOOL match = 0;

					// Or does it match the name?
					if (stricmp(entry->name, buttons->bank->window.name) == 0)
						match = 1;

					// Or the filename?
					else if (stricmp(entry->name, FilePart(buttons->buttons_file)) == 0)
						match = 1;

					// Match bank to close?
					if (match)
					{
						// Send message
						if (instruction->funcargs->FA_Arguments[ARG_SHOW])
							IPC_Command(ipc, IPC_SHOW, 0, GUI->screen_pointer, 0, 0);
						else
							IPC_Quit(ipc, 0, FALSE);
						matched = 1;
					}
				}

				// Unlock buttons list
				unlock_listlock(&GUI->buttons_list);
			}

			// If we closed a bank, return
			if (matched)
				return 1;
		}
	}

	// Go through entries
	while ((entry = function_get_entry(handle)))
	{
		Buttons *buttons;

		// Build full name
		function_build_source(handle, entry, handle->work_buffer);

		// Start menu?
		if (start)
		{
			IPCData *ipc;

			// Open as a start menu
			if ((ipc = start_new(handle->work_buffer, label, image, -1, -1)))
			{
				// Not iconified?
				if (GUI->window && !hidden)
					IPC_Command(ipc, IPC_SHOW, 0, GUI->screen_pointer, 0, 0);
			}
		}

		// Open button bank
		else if ((buttons = buttons_new(handle->work_buffer, 0, 0, 0, buttonflags)))
		{
			// Not iconified?
			if (GUI->window && !hidden)
				IPC_Command(buttons->ipc, BUTTONS_OPEN, 0, GUI->screen_pointer, 0, 0);
		}

		// Get next entry
		function_end_entry(handle, entry, 1);
	}

	return 1;
}
