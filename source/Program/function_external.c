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

// Run an external instruction
int function_external_command(FunctionHandle *handle, InstructionParsed *instruction)
{
	char *func_string;
	struct Node *current_arg, *last_arg;
	short cont = PARSE_MORE_FILES, limit;
	PathNode *path;

	// See if recursive flag is set
	if (handle->func_parameters.flags & FUNCF_RECURSE_DIRS)
		handle->instruction_flags = INSTF_RECURSE_DIRS;
	else
		handle->instruction_flags = 0;

	// Need a source path and don't have one?
	if (handle->func_flags & FUNCF_NEED_SOURCE && !(handle->func_flags & FUNCF_GOT_SOURCE))
		return 0;

	// Need a destination path and don't have one?
	if (handle->func_flags & FUNCF_NEED_DEST && !(handle->func_flags & FUNCF_GOT_DEST))
		return 0;

	// If we needs files, check there are some
	if ((instruction->flags & FUNCF_NEED_ENTRIES) && !(function_current_entry(handle)))
		return 0;

	// Get first path
	path = function_path_current(&handle->dest_paths);

	// Get cll limit
	if ((limit = environment->env->settings.command_line_length) < 256)
		limit = 256;

	// Allocate buffer for command line
	if (!(func_string = AllocVec(limit + 1, 0)))
		return 0;

	// Go through destination paths
	do
	{
		// Store current argument pointer
		last_arg = handle->func_current_arg;
		current_arg = handle->func_current_arg;

		// Get destination path
		if (path)
			strcpy(handle->dest_path, path->path);

		// Loop until this instruction is finished
		while (cont == PARSE_MORE_FILES)
		{
			// Build string
			if ((cont = function_build_instruction(handle, instruction, 0, func_string)) == PARSE_ABORT)
			{
				handle->inst_flags &= ~INSTF_REPEAT;
				FreeVec(func_string);
				return 0;
			}

			/*
						// If we needs files, check there are some
						if (instruction->flags&FUNCF_NEED_ENTRIES &&
							!(function_current_entry(handle)))
						{
							cont=PARSE_INVALID;
						}
			*/

			// Valid?
			if (cont >= PARSE_OK)
			{
				// Write a line to the script file
				function_write_script(handle, func_string, instruction->type);

				// Store last argument
				last_arg = handle->func_current_arg;

				// Get next argument to use
				if (current_arg)
					handle->func_current_arg = handle->func_current_arg->ln_Succ;

				// If no valid argument, get previous or first
				if (!handle->func_current_arg || !handle->func_current_arg->ln_Succ)
				{
					if (current_arg)
						handle->func_current_arg = current_arg;
					else
						handle->func_current_arg = handle->func_arguments.lh_Head;
				}

				// Set repeat flag
				handle->inst_flags |= INSTF_REPEAT;
			}
		}

		// Clear repeat flag
		handle->inst_flags &= ~INSTF_REPEAT;

		// Restore argument pointer
		handle->func_current_arg = last_arg;

		// Invalid?
		if (cont == PARSE_INVALID)
		{
			FreeVec(func_string);
			return -1;
		}

		// Done with this path
		if (path)
		{
			function_path_end(handle, &handle->dest_paths, 0);
			path = function_path_next(&handle->dest_paths);
		}
	} while (path);

	FreeVec(func_string);
	return 1;
}
