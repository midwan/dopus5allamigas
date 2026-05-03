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

// Open script file
int function_open_script(FunctionHandle *handle)
{
	unsigned short a;
	ULONG micros, secs;

	// If script file is already open, return
	if (handle->script_file)
		return 1;

	// Get temporary key
	handle->temp_key = (ULONG)handle->ipc;
	CurrentTime(&secs, &micros);
	if (micros)
		handle->temp_key *= micros;
	handle->temp_key += secs;

	// Create a temporary script file; try up to 99 times
	for (a = 0; a < 99; a++)
	{
		// Build script name
		lsprintf(handle->script_name, "T:dopus-%lx-tmp", handle->temp_key + a);

		// Try to open file
		if ((handle->script_file = OpenBuf(handle->script_name, MODE_NEWFILE, 2048)))
			break;
	}

	// If we failed to create script file, return
	if (!handle->script_file)
		return 0;

	// Make sure script bit is set
	SetProtection(handle->script_name, FIBF_SCRIPT);

	// CD source?
	if (handle->func_parameters.flags & FUNCF_CD_SOURCE)
		strcpy(handle->temp_buffer + 4, handle->source_path);

	// CD destination?
	else if (handle->func_parameters.flags & FUNCF_CD_DESTINATION)
		strcpy(handle->temp_buffer + 4, handle->dest_path);

	// No CD
	else
		handle->temp_buffer[4] = 0;

	// Directory to CD to?
	if (handle->temp_buffer[4])
	{
		// Fill out command
		handle->temp_buffer[0] = 'c';
		handle->temp_buffer[1] = 'd';
		handle->temp_buffer[2] = ' ';
		handle->temp_buffer[3] = '\"';
		strcat(handle->temp_buffer, "\"");

		// Write command
		function_write_script(handle, handle->temp_buffer, INST_AMIGADOS);
	}

	// Write stack line
	lsprintf(handle->temp_buffer, "stack %ld", environment->env->default_stack);
	function_write_script(handle, handle->temp_buffer, INST_AMIGADOS);
	return 1;
}

static char const *const script_type_intro[] = {
	0,					   // INST_COMMAND
	0,					   // INST_AMIGADOS
	"Dopus5:C/dopusrt5 ",  // INST_WORKBENCH
	"execute ",			   // INST_SCRIPT
	"rx ",				   // INST_AREXX
};

// Write a line to the script file
int function_write_script(FunctionHandle *handle, char *script_line, short type)
{
	// Check script file is open
	if (!(function_open_script(handle)))
		return 0;

	// Write type introducer
	if (script_type_intro[type])
	{
		// Kludge to handle async workbench stuff
		if (type == INST_WORKBENCH && strnicmp(script_line, "run ", 4) == 0)
		{
			// Async workbench
			script_line += 4;
			WriteBuf(handle->script_file, "run >nil: <nil: ", -1);
		}
		WriteBuf(handle->script_file, script_type_intro[type], -1);
	}

	/*
	D(bug("> "));
	if (script_type_intro[type]) D(bug("%s",script_type_intro[type]));
	D(bug("%s\n",script_line));
	*/

	// Write line to script file
	WriteBuf(handle->script_file, script_line, -1);

	// Write newline
	WriteBuf(handle->script_file, "\n", 1);

	return 1;
}

// Close script file and run function
void function_close_script(FunctionHandle *handle, int run_okay)
{
	// Is script file open?
	if (handle->script_file)
	{
		BPTR output = 0, input;

		// Was function successful?
		if (run_okay)
		{
			// Click to close?
			if (handle->func_parameters.flags & FUNCF_WAIT_CLOSE)
			{
				// Write command
				function_write_script(handle, "echo \"\"", INST_AMIGADOS);
				lsprintf(handle->temp_buffer, "echo \"%s\" noline", GetString(&locale, MSG_PRESS_MOUSE_BUTTON));
				function_write_script(handle, handle->temp_buffer, INST_AMIGADOS);
			}
		}

		// Close script
		CloseBuf(handle->script_file);
		handle->script_file = 0;

		// Run function?
		if (run_okay)
		{
			short need_window = 0;

			// Open an output file?
			if (handle->func_parameters.flags & FUNCF_OUTPUT_FILE)
			{
				// If async, change to window
				if (handle->func_parameters.flags & FUNCF_RUN_ASYNC)
					need_window = 1;

				// Otherwise
				else
				{
					// Build output filename
					lsprintf(handle->temp_name, "t:dopus-%lx-out", handle->temp_key);

					// Open output file
					if (!(output = Open(handle->temp_name, MODE_NEWFILE)))
					{
						// Will change to output window if this fails
						handle->temp_name[0] = 0;
						need_window = 1;
					}
				}
			}

			// Open an output window?
			if (handle->func_parameters.flags & FUNCF_OUTPUT_WINDOW || need_window == 1)
			{
				// Copy handle
				strcpy(handle->work_buffer, handle->output_handle);

				// Close button?
				if (handle->func_parameters.flags & FUNCF_WAIT_CLOSE)
				{
					char *ptr;

					// If there's a /SCREEN option, we must insert before it
					if ((ptr = strstri(handle->work_buffer, "/SCREEN ")))
					{
						// Copy remainder of string to temporary buffer
						strcpy(handle->work_buffer + 512, ptr);

						// Copy close string
						strcpy(ptr, "/CLOSE/WAIT");

						// Copy /SCREEN back onto end of string
						strcat(ptr, handle->work_buffer + 512);
					}

					// Otherwise, just tack it on to the end
					else
						strcat(handle->work_buffer, "/CLOSE/WAIT");
				}

				// Open window
				output = Open(handle->work_buffer, MODE_OLDFILE);
			}

			// No output, get nil
			if (!output)
				output = Open("nil:", MODE_OLDFILE);

			// Build command to run script
			lsprintf(handle->temp_buffer, "execute %s\n", handle->script_name);

			// See if output handle is interactive
			if (IsInteractive(output))
			{
				// Run script file with input/output channels
				input = output;
				output = 0;
			}

			// Otherwise, run with output and nil: input
			else
				input = Open("nil:", MODE_OLDFILE);

			// Launch script
			if (CLI_Launch(handle->temp_buffer,
						   (struct Screen *)-1,
						   0,
						   input,
						   output,
						   (!(handle->func_parameters.flags & FUNCF_RUN_ASYNC)) | LAUNCHF_USE_STACK,
						   environment->env->default_stack))
			{
				// Not run async?
				if (!(handle->func_parameters.flags & FUNCF_RUN_ASYNC))
				{
					// Rescan source?
					if (handle->func_parameters.flags & FUNCF_RESCAN_SOURCE)
					{
						PathNode *path = function_path_current(&handle->source_paths);
						if (path)
							path->flags |= LISTNF_RESCAN;
					}

					// Rescan destination?
					if (handle->func_parameters.flags & FUNCF_RESCAN_DEST)
					{
						PathNode *path = function_path_current(&handle->dest_paths);
						if (path)
							path->flags |= LISTNF_RESCAN;
					}

					// Delete script file
					DeleteFile(handle->script_name);

					// View temporary file (if any)
					if (handle->temp_name[0])
					{
						Att_List *list;

						// Create list
						if ((list = Att_NewList(0)))
						{
							Att_Node *node;

							// Add node with filename
							if ((node = Att_NewNode(list, handle->temp_name, 0, 0)))
							{
								struct read_startup *startup;

								// Set flag to indicate delete file
								node->node.ln_Pri = 1;

								// Allocate startup
								if ((startup = AllocVec(sizeof(struct read_startup), MEMF_CLEAR)))
								{
									// Read file
									startup->files = (struct List *)list;
									if (misc_startup("dopus_read", FUNC_READ, 0, startup, FALSE))
										/* */ return;
									FreeVec(startup);
								}
							}

							// Failed
							Att_RemList(list, 0);
						}

						// Failed to read; delete file
						DeleteFile(handle->temp_name);
					}
				}
			}
		}
	}
}
