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
	ARG_MODULES,
	ARG_SYSTEM,
};

// Reset internal function
DOPUS_FUNC(function_reset)
{
	// Arguments?
	if (!instruction->funcargs)
		return 0;

	// Reset modules?
	if (instruction->funcargs->FA_Arguments[ARG_MODULES])
	{
		// Update commands
		update_commands(SNIFF_BOTH);
	}

	// Reset system
	else if (instruction->funcargs->FA_Arguments[ARG_SYSTEM])
	{
		// Reboot
		ColdReboot();
	}

	return 1;
}
