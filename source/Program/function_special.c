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

// Quit, Hide, Reveal
DOPUS_FUNC(function_special)
{
	// Look at command
	switch (command->function)
	{
	// Quit
	case FUNC_QUIT:

		// Force quit?
		if (instruction->funcargs && instruction->funcargs->FA_Arguments[0])
		{
			// Send quit message directory
			IPC_Quit(&main_ipc, 0, FALSE);
		}

		// Otherwise, start quit process
		else
		{
			misc_startup("dopus_quit", MENU_QUIT, GUI->window, 0, 1);
		}
		return 0;

	// Hide
	case FUNC_HIDE:

		// Send hide command
		IPC_Command(&main_ipc, IPC_HIDE, 0, 0, 0, 0);
		break;

	// Reveal
	case FUNC_REVEAL:

		// Send show command
		IPC_Command(&main_ipc, IPC_SHOW, 0, 0, 0, 0);
		break;
	}

	return 1;
}
