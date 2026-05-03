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

// LOADENVIRONMENT/LOADSETTINGS/EDITFILETYPE internal function
DOPUS_FUNC(function_loadenvironment)
{
	FunctionEntry *entry;
	env_packet *packet;

	// Get first entry
	if ((entry = function_get_entry(handle)))
	{
		BPTR lock;

		// Build full name
		function_build_source(handle, entry, handle->work_buffer);

		// See if file exists
		if ((lock = Lock(handle->work_buffer, ACCESS_READ)))
			UnLock(lock);

		// Load environment?
		if (command->function == FUNC_LOADENVIRONMENT)
		{
			// If file didn't exist, look in default directory
			if (!lock)
				lsprintf(handle->work_buffer, "dopus5:environment/%s", entry->name);

			// Allocate packet
			if ((packet = AllocVec(sizeof(env_packet) + strlen(handle->work_buffer), 0)))
			{
				// Fill out packet
				packet->type = -1;
				strcpy(packet->name, handle->work_buffer);

				// Launch process to open environment
				IPC_Launch(&GUI->process_list,
						   0,
						   "dopus_environment",
						   (IPTR)&environment_proc,
						   STACK_LARGE,
						   (IPTR)packet,
						   (struct Library *)DOSBase);
			}
		}

		// Edit filetype
		else if (command->function == FUNC_EDITFILETYPE)
		{
			// Must not be iconified
			if (GUI->window)
			{
				char *name;

				// Allocate name copy
				if ((name = AllocVec(strlen(handle->work_buffer) + 1, 0)))
					strcpy(name, handle->work_buffer);

				// Launch process to configure filetypes
				if (!(misc_startup("dopus_config_filetypes", MENU_FILETYPES, GUI->window, name, 1)))
					FreeVec(name);
			}
		}

		// End this entry
		function_end_entry(handle, entry, 1);
	}

	return 0;
}
