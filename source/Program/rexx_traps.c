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

// Add a function trap
short AddFunctionTrap(char *command, char *handler, char *port)
{
	FunctionTrap *trap;

	// Valid handler?
	if (!handler || !*handler)
		return RXERR_INVALID_NAME;

	// Trap all?
	if (*command == '*')
	{
		short num;

		// Go through internal commands
		for (num = 0; commandlist_internal[num].name; num++)
		{
			// Trap this function (a bit recursive)
			if (commandlist_internal[num].function != 0 &&
				commandlist_internal[num].flags & (FUNCF_NEED_ENTRIES | FUNCF_NEED_SOURCE | FUNCF_WANT_SOURCE))
				AddFunctionTrap(commandlist_internal[num].name, handler, port);
		}
		return RXERR_OK;
	}

	// See if trap is already installed
	if (FindFunctionTrap(command, handler, 0))
		return RXERR_OK;

	// Allocate new trap entry
	if (!(trap = AllocVec(sizeof(FunctionTrap) + ((*port) ? sizeof(TrapSignal) : 0) + strlen(command) + 1, MEMF_CLEAR)))
		return RXERR_NO_MEMORY;

	// Fill out trap
	if (*port)
		trap->command = (char *)(trap + 1) + sizeof(TrapSignal);
	else
		trap->command = (char *)(trap + 1);
	strcpy(trap->command, command);
	stccpy(trap->handler, handler, 31);

	// Individual port?
	if (*port)
	{
		TrapSignal *signal = (TrapSignal *)(trap + 1);

		// Copy port name
		strcpy(signal->port_name, port);

		// Set flag
		trap->flags |= FTRAPF_SIGNAL;
	}

	// Lock trap list
	lock_listlock(&GUI->function_traps, 1);

	// Add trap to list
	AddTail((struct List *)&GUI->function_traps, (struct Node *)trap);

	// Unlock trap list
	unlock_listlock(&GUI->function_traps);

	return RXERR_OK;
}

// Remove a function trap
short RemFunctionTrap(char *command, char *handler)
{
	FunctionTrap *trap;
	short ret = RXERR_INVALID_TRAP;
	char handler_buf[80];

	// Valid handler?
	if (!handler || !*handler)
		return RXERR_INVALID_NAME;

	// Parse handler
	ParsePattern(handler, handler_buf, 80);

	// Lock trap list
	lock_listlock(&GUI->function_traps, 1);

	// Go through trap list
	for (trap = (FunctionTrap *)GUI->function_traps.list.lh_Head; trap->node.mln_Succ;)
	{
		FunctionTrap *next = (FunctionTrap *)trap->node.mln_Succ;

		// Match command and handler
		if ((*command == '*' || stricmp(trap->command, command) == 0) && MatchPattern(handler_buf, trap->handler))
		{
			// Remove this trap
			Remove((struct Node *)trap);

			// Free trap entry
			FreeVec(trap);

			// Ok
			ret = RXERR_OK;
		}

		trap = next;
	}

	// Unlock trap list
	unlock_listlock(&GUI->function_traps);

	return ret;
}

// Check for a function trap for a particular handler
BOOL FindFunctionTrap(char *command, char *handler, char *port)
{
	FunctionTrap *trap;
	BOOL ret = FALSE;

	// Valid handler?
	if (!handler)
		return 0;

	// Initialise port name
	if (port)
		strcpy(port, handler);

	// Lock trap list
	lock_listlock(&GUI->function_traps, 0);

	// Go through trap list
	for (trap = (FunctionTrap *)GUI->function_traps.list.lh_Head; trap->node.mln_Succ;
		 trap = (FunctionTrap *)trap->node.mln_Succ)
	{
		// Match command and handler
		if (stricmp(trap->command, command) == 0 && strcmp(trap->handler, handler) == 0)
		{
			// Found one
			ret = TRUE;

			// Unique port?
			if (trap->flags & FTRAPF_SIGNAL && port)
			{
				TrapSignal *signal = (TrapSignal *)(trap + 1);

				// Copy port name
				strcpy(port, signal->port_name);
			}
			break;
		}
	}

	// Unlock trap list
	unlock_listlock(&GUI->function_traps);

	return ret;
}

// Lock trap list
APTR LockTrapList(void)
{
	// Lock trap list
	lock_listlock(&GUI->function_traps, 0);
	return &GUI->function_traps;
}

// Unlock trap list
void UnlockTrapList(void)
{
	// Unlock trap list
	unlock_listlock(&GUI->function_traps);
}

// Get next trap for a function
APTR FindTrapEntry(APTR last, char *command, char *port)
{
	FunctionTrap *trap;

	// Start of list?
	if (last == &GUI->function_traps)
		trap = (FunctionTrap *)GUI->function_traps.list.lh_Head;

	// Get next trap
	else if (last)
		trap = (FunctionTrap *)((FunctionTrap *)last)->node.mln_Succ;

	// Invalid
	else
		return 0;

	// Go through traps
	for (; trap->node.mln_Succ; trap = (FunctionTrap *)trap->node.mln_Succ)
	{
		// Match command
		if (stricmp(trap->command, command) == 0)
		{
			// Unique port?
			if (trap->flags & FTRAPF_SIGNAL)
			{
				TrapSignal *signal = (TrapSignal *)(trap + 1);

				// Copy port name
				strcpy(port, signal->port_name);
			}

			// Copy port name
			else
				strcpy(port, trap->handler);
			return trap;
		}
	}

	return 0;
}
