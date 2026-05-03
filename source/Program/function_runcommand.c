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

// RUNCOMMAND internal function
DOPUS_FUNC(function_runcommand)
{
	FunctionEntry *entry;
	Cfg_Function *func;

	// Get first entry
	if (!(entry = function_get_entry(handle)))
		return 0;

	// Build full name
	function_build_source(handle, entry, handle->work_buffer);

	// Load command
	func = function_load_function(handle->work_buffer);

	// Got function?
	if (func)
	{
		// Set flag to free function when done
		func->function.flags2 |= FUNCF2_FREE_FUNCTION;

		// Launch function
		if (function_launch(FUNCTION_RUN_FUNCTION_EXTERNAL, func, 0, 0, 0, 0, 0, 0, handle->arg_passthru, 0, 0))
		{
			// Clear arg passthru since we passed it on
			handle->arg_passthru = 0;
		}
	}

	return 1;
}

// EDITCOMMAND internal function
DOPUS_FUNC(function_editcommand)
{
	FunctionEntry *entry;

	// Get first entry
	if (!(entry = function_get_entry(handle)))
		return 0;

	// Build full name
	function_build_source(handle, entry, handle->work_buffer);

	// Edit the command
	command_new(GUI->backdrop, handle->ipc, handle->work_buffer);
	return 1;
}

// Load a command file
Cfg_Function *function_load_function(char *name)
{
	APTR iff;
	Cfg_Function *func = 0;

	// Open as IFF
	if (!(iff = IFFOpen(name, IFF_READ, ID_OPUS)))
		return 0;

	// Find function chunk
	if (IFFNextChunk(iff, ID_FUNC) == ID_FUNC)
	{
		// Read function
		func = ReadFunction(iff, global_memory_pool, 0, 0);
	}

	// Close IFF file
	IFFClose(iff);
	return func;
}
