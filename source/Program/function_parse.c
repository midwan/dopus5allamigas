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

#define MAXPARSEDEPTH 10

// Parse a function
int function_parse_function(FunctionHandle *handle)
{
	Cfg_Instruction *instruction, *next, *last_ins[MAXPARSEDEPTH];
	InstructionParsed *parse;
	Cfg_Function *function[MAXPARSEDEPTH];
	short user_depth, done_script = -1;
	BOOL extflag = 0;

	// Clear table
	for (user_depth = 0; user_depth < MAXPARSEDEPTH; user_depth++)
	{
		last_ins[user_depth] = 0;
		function[user_depth] = 0;
	}

	// Initialise depth
	user_depth = 0;

	// Go through instructions
	for (instruction = (Cfg_Instruction *)handle->function->instructions.mlh_Head;
		 instruction && instruction->node.mln_Succ;
		 instruction = next)
	{
		char *ptr, buf[512];
		ULONG flags = 0;
		CommandList *command = 0;

		// Get the next instruction
		next = (Cfg_Instruction *)instruction->node.mln_Succ;

		// Get initial string pointer
		ptr = instruction->string;

		// Internal command?
		if (instruction->type == INST_COMMAND)
		{
			// Get command
			if ((command = function_find_internal(&ptr, (handle->function->function.flags2 & FUNCF2_ORIGINAL) ? 1 : 0)))
			{
				// Command that can be overridden by a script?
				if (command->flags & FUNCF_SCRIPT_OVERRIDE && done_script == -1)
				{
					// See if there's a script function
					if ((function[user_depth] = (Cfg_Function *)FindScript_Function(command->name)))
					{
						// Save next instruction
						last_ins[user_depth] = next;

						// Shift into script function
						next = (Cfg_Instruction *)function[user_depth]->instructions.mlh_Head;

						// Increment depth, set flag to say we're in a script function
						done_script = user_depth;
						++user_depth;
						continue;
					}
				}
			}
		}

		// Check nesting depth
		if (user_depth < MAXPARSEDEPTH)
		{
			// Internal command?
			if (command)
			{
				// User command?
				if (command->flags & FUNCF_EXTERNAL_FUNCTION &&
					strnicmp(command->stuff.module_name, "dopus5:commands/", 16) == 0)
				{
					// Load the function
					if ((function[user_depth] = function_load_function(command->stuff.module_name)))
					{
						// Valid instruction list?
						if (!(IsListEmpty((struct List *)&function[user_depth]->instructions)))
						{
							// Save next instruction
							last_ins[user_depth] = next;

							// Shift into that function
							next = (Cfg_Instruction *)function[user_depth]->instructions.mlh_Head;

							// Increment depth
							++user_depth;
							continue;
						}

						// Free the function
						FreeFunction(function[user_depth]);
						function[user_depth] = 0;
					}
				}
			}
		}

		// Skip labels
		if (instruction->type != INST_LABEL)
		{
			// Allocate new entry
			if ((parse = AllocMemH(handle->memory, sizeof(InstructionParsed))))
			{
				// Initialise
				parse->type = instruction->type;

				// Internal command?
				parse->command = command;

				// Parse instruction
				function_parse_instruction(handle, ptr, buf, &flags);

				// Get command flags
				if (parse->command)
					flags |= parse->command->flags;

				// External command
				else
					extflag = 1;

				// Copy argument string
				if (buf[0])
				{
					if ((parse->string = AllocMemH(handle->memory, strlen(buf) + 1)))
					{
						strcpy(parse->string, buf);

						// If it's a command, parse it
						if (parse->command && parse->command->template)
						{
							// ReadArgs parse
							if ((parse->funcargs = ParseArgs(parse->command->template, parse->string)) &&
								parse->command->template_key)
							{
								char *ptr;

								// Can this supply a source?
								if ((ptr = strchr(parse->command->template_key, FUNCKEY_SOURCE)))
								{
									// Source supplied?
									if ((ptr = (char *)parse->funcargs->FA_ArgArray[atoi(ptr + 1)]) &&
										(strchr(ptr, ':') || strchr(ptr, '/')))
									{
										// Clear "need source" flag
										flags &= ~FUNCF_NEED_SOURCE;
									}
								}

								// Can this supply a destination?
								if ((ptr = strchr(parse->command->template_key, FUNCKEY_DIR)))
								{
									// Source supplied?
									if ((ptr = (char *)parse->funcargs->FA_ArgArray[atoi(ptr + 1)]) &&
										(strchr(ptr, ':') || strchr(ptr, '/')))
									{
										// Clear "need dest" flag
										flags &= ~FUNCF_NEED_DEST;
									}
								}

								// Can this supply a file?
								if ((ptr = strchr(parse->command->template_key, FUNCKEY_FILE)) ||
									(ptr = strchr(parse->command->template_key, FUNCKEY_FILENO)))
								{
									// File supplied?
									if (parse->funcargs->FA_ArgArray[atoi(ptr + 1)])
									{
										// Clear "need files" flag
										flags &= ~FUNCF_NEED_ENTRIES;
										flags |= FUNCF_WANT_ENTRIES;
									}
								}

								// New argument?
								if ((ptr = strchr(parse->command->template_key, FUNCKEY_NEW)))
								{
									// New flag set?
									if (parse->funcargs->FA_ArgArray[atoi(ptr + 1)])
										parse->new_arg = 1;
								}
							}
						}
					}
				}

				// Set function flags
				handle->func_flags |= flags;
				parse->flags |= flags;

				// Link instruction in
				AddTail((struct List *)&handle->func_instructions, (struct Node *)parse);
			}
		}

		// Finished a user function?
		if (user_depth > 0 && !next->node.mln_Succ)
		{
			// Decrement depth
			--user_depth;

			// Add flags to collective value
			handle->func_additional_flags |= function[user_depth]->function.flags;

			// Continue on with original instruction
			next = last_ins[user_depth];
			last_ins[user_depth] = 0;

			// Was this function a script?
			if (done_script == user_depth)
			{
				// Clear script flag
				done_script = -1;

				// Unlock script lock
				FreeSemaphore(&GUI->scripts_lock);
			}

			// It was a user function
			else
			{
				// Free function
				FreeFunction(function[user_depth]);
			}

			// Clear function pointer
			function[user_depth] = 0;
		}
	}

	// Free user functions
	for (user_depth = 0; user_depth < MAXPARSEDEPTH; user_depth++)
	{
		// Check not a script function
		if (user_depth != done_script)
			FreeFunction(function[user_depth]);
	}

	// Script function remaining)
	if (done_script > -1)
	{
		// Unlock script lock
		FreeSemaphore(&GUI->scripts_lock);
	}

	// External command?
	if (extflag)
	{
		// Check function flags
		if (handle->function->function.flags & FUNCF_CD_SOURCE)
			handle->func_flags |= FUNCF_NEED_SOURCE;
		if (handle->function->function.flags & FUNCF_CD_DESTINATION)
			handle->func_flags |= FUNCF_NEED_DEST;

		// Need destination? Ok to ask for path
		if (handle->func_flags & FUNCF_NEED_DEST)
			handle->func_flags |= FUNCF_ASK_PATH;
	}

	return 1;
}

// Parse an instruction
void function_parse_instruction(FunctionHandle *handle, char *string, unsigned char *buffer, ULONG *flags)
{
	short func_len, parse_pos, func_pos;

	// Clear flags and buffer
	*flags = 0;
	*buffer = 0;

	// Valid string?
	if (!string || !string[0])
		return;

	// Get length of function string
	func_len = strlen(string);

	// Go through function string
	for (func_pos = 0, parse_pos = 0; func_pos < func_len && parse_pos < 512; func_pos++)
	{
		// Start of a command sequence?
		if (string[func_pos] == '{')  // }
		{
			// Is there enough data left for a valid command?
			if (func_len < func_pos + 2)
			{
				buffer[parse_pos++] = string[func_pos];
				continue;
			}

			// Double-{ indicates a literal { is to be included }}
			if (string[func_pos + 1] == '{')  // }
			{
				buffer[parse_pos++] = '{';	// }
				++func_pos;
				continue;
			}

			// Increment function position
			++func_pos;

			// Look at command type
			switch (string[func_pos])
			{
			// Variable
			case 'v':
				if (string[func_pos] == 'v')
					buffer[parse_pos] = FUNC_VARIABLE;

			// Requester
			case 'R':
				if (string[func_pos] == 'R')
					buffer[parse_pos] = FUNC_REQUESTER;

				// Bump buffer position
				++parse_pos;

				// Look for end of argument
				for (++func_pos; func_pos < func_len && parse_pos < 512; func_pos++)
				{
					// End of argument? {
					if (string[func_pos] == '}')
						break;

					// Store character in function buffer
					buffer[parse_pos++] = string[func_pos];
				}

				// Store "argument end" character
				buffer[parse_pos++] = FUNC_END_ARG;
				continue;

			// Query something
			case 'Q':
				buffer[parse_pos++] = FUNC_QUERY_INFO;
				buffer[parse_pos++] = string[func_pos + 1];

				// Query lister needs a source
				if (string[func_pos + 1] == 'l')
					*flags |= FUNCF_WANT_SOURCE;

				// Query destination needs a.. well.. destination
				else if (string[func_pos + 1] == 'd')
					*flags |= FUNCF_WANT_DEST;

				func_pos += 2;
				continue;

			// One file
			case 'o':

				// Last file?
				if (string[func_pos + 1] == 'u')
				{
					// Got a last file?
					if (handle->func_flags & FUNCF_LAST_FILE_FLAG)
						buffer[parse_pos] = FUNC_LAST_FILE;

					// Current file
					else
					{
						buffer[parse_pos] = FUNC_ONE_FILE;
						handle->func_flags |= FUNCF_LAST_FILE_FLAG;

						// Need or want?
						if (string[func_pos + 2] == '!')
						{
							*flags |= FUNCF_WANT_ENTRIES;
							++func_pos;
						}
						else
							*flags |= FUNCF_NEED_ENTRIES;
					}
					++func_pos;
				}

				// Current file
				else if (handle->func_flags & FUNCF_LAST_FILE_FLAG)
				{
					buffer[parse_pos] = FUNC_LAST_FILE;
					handle->func_flags &= ~FUNCF_LAST_FILE_FLAG;
				}
				else
				{
					buffer[parse_pos] = FUNC_ONE_FILE;

					// Need or want?
					if (string[func_pos + 1] == '!')
					{
						*flags |= FUNCF_WANT_ENTRIES;
						++func_pos;
					}
					else
						*flags |= FUNCF_NEED_ENTRIES;
				}

				// Quotes?
				if (string[func_pos + 1] == '`' || string[func_pos + 1] == '\'')
				{
					buffer[++parse_pos] = FUNC_QUOTES;
					++func_pos;
				}

				// No quotes
				else if (string[func_pos + 1] == '~')
				{
					buffer[++parse_pos] = FUNC_NO_QUOTES;
					++func_pos;
				}

				// Normal
				else
				{
					buffer[++parse_pos] = FUNC_NORMAL;
				}

				// Strip suffix?
				if (string[func_pos + 1] == '-')
				{
					buffer[++parse_pos] = FUNC_STRIP_SUFFIX;
					++func_pos;
				}

				// Bump buffer positions
				++func_pos;
				++parse_pos;
				break;

			// All files
			case 'O':
				buffer[parse_pos++] = FUNC_ALL_FILES;
				if (string[func_pos + 1] == '!')
				{
					*flags |= FUNCF_WANT_ENTRIES;
					++func_pos;
				}
				else
					*flags |= FUNCF_NEED_ENTRIES;

				// Quotes?
				if (string[func_pos + 1] == '`' || string[func_pos + 1] == '\'')
				{
					buffer[parse_pos++] = FUNC_QUOTES;
					++func_pos;
				}

				// No quotes
				else if (string[func_pos + 1] == '~')
				{
					buffer[parse_pos++] = FUNC_NO_QUOTES;
					++func_pos;
				}

				// Normal
				else
				{
					buffer[parse_pos++] = FUNC_NORMAL;
				}
				++func_pos;
				break;

			// One path
			case 'f':

				// Last file?
				if (string[func_pos + 1] == 'u')
				{
					// Got a last file?
					if (handle->func_flags & FUNCF_LAST_FILE_FLAG)
						buffer[parse_pos] = FUNC_LAST_PATH;

					// Current file
					else
					{
						buffer[parse_pos] = FUNC_ONE_PATH;
						handle->func_flags |= FUNCF_LAST_FILE_FLAG;

						// Need or want?
						if (string[func_pos + 2] == '!')
						{
							*flags |= FUNCF_WANT_ENTRIES;
							++func_pos;
						}
						else
							*flags |= FUNCF_NEED_ENTRIES;
					}
					++func_pos;
				}

				// Current file
				else if (handle->func_flags & FUNCF_LAST_FILE_FLAG)
				{
					buffer[parse_pos] = FUNC_LAST_PATH;
					handle->func_flags &= ~FUNCF_LAST_FILE_FLAG;
				}
				else
				{
					buffer[parse_pos] = FUNC_ONE_PATH;

					// Need or want?
					if (string[func_pos + 1] == '!')
					{
						*flags |= FUNCF_WANT_ENTRIES;
						++func_pos;
					}
					else
						*flags |= FUNCF_NEED_ENTRIES;
				}

				// Quotes?
				if (string[func_pos + 1] == '`' || string[func_pos + 1] == '\'')
				{
					buffer[++parse_pos] = FUNC_QUOTES;
					++func_pos;
				}

				// No quotes
				else if (string[func_pos + 1] == '~')
				{
					buffer[++parse_pos] = FUNC_NO_QUOTES;
					++func_pos;
				}

				// Normal
				else
				{
					buffer[++parse_pos] = FUNC_NORMAL;
				}

				// Strip suffix?
				if (string[func_pos + 1] == '-')
				{
					buffer[++parse_pos] = FUNC_STRIP_SUFFIX;
					++func_pos;
				}

				// Bump buffer positions
				++func_pos;
				++parse_pos;
				break;

			// All paths
			case 'F':
				buffer[parse_pos++] = FUNC_ALL_PATHS;
				if (string[func_pos + 1] == '!')
				{
					*flags |= FUNCF_WANT_ENTRIES;
					++func_pos;
				}
				else
					*flags |= FUNCF_NEED_ENTRIES;

				// Quotes?
				if (string[func_pos + 1] == '`' || string[func_pos + 1] == '\'')
				{
					buffer[parse_pos++] = FUNC_QUOTES;
					++func_pos;
				}

				// No quotes
				else if (string[func_pos + 1] == '~')
				{
					buffer[parse_pos++] = FUNC_NO_QUOTES;
					++func_pos;
				}

				// Normal
				else
				{
					buffer[parse_pos++] = FUNC_NORMAL;
				}
				++func_pos;
				break;

			// Source directory
			case 's':
				buffer[parse_pos++] = FUNC_SOURCE;
				if (string[func_pos + 1] == '!')
				{
					*flags |= FUNCF_WANT_SOURCE;
					++func_pos;
				}
				else
					*flags |= FUNCF_NEED_SOURCE;
				++func_pos;
				break;

			// Destination directory
			case 'd':
				buffer[parse_pos++] = FUNC_DEST;
				if (string[func_pos + 1] == '!')
				{
					*flags |= FUNCF_WANT_DEST;
					++func_pos;
				}
				else
					*flags |= FUNCF_NEED_DEST;
				++func_pos;
				break;

			// Public screen
			case 'p':
				buffer[parse_pos++] = FUNC_QUERY_INFO;
				buffer[parse_pos++] = 's';
				++func_pos;
				break;
			}
		}

		// Store character in buffer
		else
			buffer[parse_pos++] = string[func_pos];
	}

	// Null terminate
	buffer[parse_pos] = 0;
}

// Build a function string with arguments and everything
int function_build_instruction(FunctionHandle *handle,
							   InstructionParsed *ins,
							   unsigned char *instruction,
							   char *function_buf)
{
	short quote_extra = 0;
	short pos, func_pos = 0, func_len;
	Lister *source, *dest;
	PathNode *path;
	FunctionEntry *entry = 0;
	short len, source_len, cont_flag = PARSE_END, max_len, limit;
	UBYTE ch;
	BOOL okay;
	BOOL quote_flag = 0, wb_func = 0;

	// Inintialise some buffers
	function_buf[0] = 0;

	// Workbench instruction?
	if (ins->type == INST_WORKBENCH)
		wb_func = 1;

	// If quoting filenames, this is a convenient way to know about it
	if (!(handle->func_parameters.flags & FUNCF_NO_QUOTES))
		quote_extra = 2;

	// Get instruction pointer
	if (!instruction && ins)
		instruction = ins->string;

	// Is there actually anything to parse?
	if (!ins || !instruction || !*instruction)
		return PARSE_INVALID;

	// Only do once?
	if (*instruction == '#')
		++instruction;

	// Get length of source path and instruction string
	source_len = strlen(handle->source_path);
	func_len = strlen(instruction);

	// Get cll limit
	if ((limit = environment->env->settings.command_line_length) < 256)
		limit = 256;

	// Calculate maximum length of string (so to leave space for static information)
	for (pos = 0, max_len = limit - 1; instruction[pos]; pos++)
	{
		// Within quotes?
		if (quote_flag)
		{
			// End quotes?
			if (instruction[pos] == '}')
				quote_flag = 0;
			continue;
		}

		// Start of quotes?
		else if (instruction[pos] == '{')
		{
			// Toggle quote flag
			quote_flag = 1 - quote_flag;
		}

		// If not within quotes, subtract from maximum length
		if (!quote_flag)
			--max_len;
	}

	// Reset quote flag
	quote_flag = 0;

	// Get current paths
	if (!(path = function_path_current(&handle->source_paths)) || !(source = path->lister))
	{
		if (!(source = handle->source_lister))
			source = handle->saved_source_lister;
	}
	if (!(path = function_path_current(&handle->dest_paths)) || !(dest = path->lister))
		dest = handle->dest_lister;

	// Go through parsed function string
	for (pos = 0; pos < func_len && func_pos < limit; pos++)
	{
		char *path_str = 0;
		short path_len = 0;
		BOOL abort = 0;
		short quote_value;

		// Get character
		ch = instruction[pos];

		// Look at character
		switch (ch)
		{
		// Strip suffix
		case FUNC_STRIP_SUFFIX: {
			short cp, quote = 0;

			// Go backwards through string
			for (cp = func_pos; cp >= 0; cp--)
			{
				// Quote?
				if (function_buf[cp] == '\"')
				{
					// Break on second quote
					if (quote)
						break;
					quote = 1;
				}

				// Break on space
				else if (function_buf[cp] == ' ')
					break;

				// Stop on dot
				else if (function_buf[cp] == '.')
				{
					// Terminate string here; need quote?
					if (quote)
						function_buf[cp++] = '\"';

					// End string
					function_buf[cp] = 0;
					func_pos = cp;
					break;
				}
			}
		}
		break;

		// Single pathname
		case FUNC_ONE_PATH:
			path_str = handle->source_path;

		// Single filename
		case FUNC_ONE_FILE:

			// Quotes?
			++pos;
			if (instruction[pos] == FUNC_QUOTES)
				quote_value = 2;
			else if (instruction[pos] == FUNC_NO_QUOTES)
				quote_value = 0;
			else
				quote_value = quote_extra;

			// Get next entry to use
			if ((entry = function_get_entry(handle)))
			{
				// Are quotes ok?
				if (quote_value && !quote_flag && (func_pos == 0 || function_buf[func_pos - 1] == ' '))
				{
					// Add a quote
					function_buf[func_pos++] = '\"';
					function_buf[func_pos] = 0;
					quote_flag = 1;
				}

				// Add name to string
				func_pos += function_add_filename(function_buf,
												  (entry->type != ENTRY_DEVICE) ? path_str : 0,
												  entry->name,
												  1,
												  (wb_func && entry->type > ENTRY_DEVICE));

				// Store name
				strcpy(handle->last_filename, handle->source_path);
				AddPart(handle->last_filename, entry->name, 512);

				// Reload this file?
				if (handle->func_parameters.flags & FUNCF_RELOAD_FILES && entry->entry &&
					!(entry->flags & FUNCENTF_ICON_ACTION))
				{
					// Add for reload
					function_filechange_reloadfile(handle, handle->source_path, entry->name, FFLF_DEFERRED);
				}

				// Say we're done with this entry
				function_end_entry(handle, entry, 1);
			}

			// No entries, and must have them?
			else if (ins->flags & FUNCF_NEED_ENTRIES)
				return PARSE_INVALID;
			break;

		// Last pathname
		case FUNC_LAST_PATH:
		case FUNC_LAST_FILE:

			// Quotes?
			++pos;
			if (instruction[pos] == FUNC_QUOTES)
				quote_value = 2;
			else if (instruction[pos] == FUNC_NO_QUOTES)
				quote_value = 0;
			else
				quote_value = quote_extra;

			// Are quotes ok?
			if (quote_value && !quote_flag && (func_pos == 0 || function_buf[func_pos - 1] == ' '))
			{
				// Add a quote
				function_buf[func_pos++] = '\"';
				function_buf[func_pos] = 0;
				quote_flag = 1;
			}

			// Add filename
			func_pos += function_add_filename(
				function_buf,
				(ch == FUNC_LAST_PATH) ? (char *)handle->last_filename : (char *)FilePart(handle->last_filename),
				0,
				1,
				(wb_func && entry->type > ENTRY_DEVICE));
			break;

		// All selected paths
		case FUNC_ALL_PATHS:
			path_str = handle->source_path;
			path_len = source_len;

		// All selected files
		case FUNC_ALL_FILES:

			// Quotes?
			++pos;
			if (instruction[pos] == FUNC_QUOTES)
				quote_value = 2;
			else if (instruction[pos] == FUNC_NO_QUOTES)
				quote_value = 0;
			else
				quote_value = quote_extra;

			// Don't have cont flag set?
			if (cont_flag)
				break;

			// Get length of function string
			len = strlen(function_buf);

			// Loop until string is full or out of entries
			okay = 0;
			while (len < max_len)
			{
				// Get next entry to use
				if ((entry = function_get_entry(handle)))
				{
					BOOL first_space;

					// Need a preceeding space?
					first_space = okay;

					// Set okay flag
					okay = 1;

					// See if this would fill the buffer
					if ((len + strlen(entry->name) + path_len + quote_value) >= max_len)
						break;

					// Need a preceeding space?
					if (first_space)
					{
						// Add space
						StrConcat(function_buf, " ", max_len);
						++len;
					}

					// Add name to function
					len += function_add_filename(function_buf,
												 (entry->type != ENTRY_DEVICE) ? path_str : 0,
												 entry->name,
												 (quote_value) ? 0 : FUNCF_NO_QUOTES,
												 (wb_func && entry->type > ENTRY_DEVICE));

					// Reload file?
					if (handle->func_parameters.flags & FUNCF_RELOAD_FILES && entry->entry &&
						!(entry->flags & FUNCENTF_ICON_ACTION))
					{
						// Add for reload
						function_filechange_reloadfile(handle, handle->source_path, entry->name, FFLF_DEFERRED);
					}

					// Done with this entry
					function_end_entry(handle, entry, 1);
				}
				else
					break;
			}

			// Get current length of function
			func_pos = strlen(function_buf);

			// More to do?
			if (entry)
				cont_flag = PARSE_MORE_FILES;

			// Outafiles
			else if (!okay)
			{
				// No entries, and must have them?
				if (ins->flags & FUNCF_NEED_ENTRIES)
				{
					cont_flag = PARSE_INVALID;
					abort = 1;
				}
			}
			break;

		// Source directory
		case FUNC_SOURCE:

			// Are quotes ok?
			if (quote_extra && !quote_flag && (func_pos == 0 || function_buf[func_pos - 1] == ' '))
			{
				// Add a quote
				function_buf[func_pos++] = '\"';
				function_buf[func_pos] = 0;
				quote_flag = 1;
			}

			// Add on path to function
			StrConcat(function_buf, handle->source_path, max_len);
			func_pos += strlen(handle->source_path);
			break;

		// Destination directory
		case FUNC_DEST:

			// Are quotes ok?
			if (quote_extra && !quote_flag && (func_pos == 0 || function_buf[func_pos - 1] == ' '))
			{
				// Add a quote
				function_buf[func_pos++] = '\"';
				function_buf[func_pos] = 0;
				quote_flag = 1;
			}

			// Add on path to function
			StrConcat(function_buf, handle->dest_path, max_len);
			func_pos += strlen(handle->dest_path);
			break;

		// Query info
		case FUNC_QUERY_INFO: {
			char *query_data = 0;
			char buf[20];

			// Look at next character
			switch (instruction[++pos])
			{
			// Public screen name
			case 's':
				query_data = get_our_pubscreen();
				break;

			// ARexx port name
			case 'p':
				query_data = GUI->rexx_port_name;
				break;

			// Source lister
			case 'l':
				lsprintf(buf, "%ld", source);
				query_data = buf;
				break;

			// Source lister process
			case 'L':
				lsprintf(buf, "%lx", (source) ? source->ipc->proc : 0);
				query_data = buf;
				break;

			// Destination lister
			case 'd':
				lsprintf(buf, "%ld", dest);
				query_data = buf;
				break;

			// Destination lister process
			case 'D':
				lsprintf(buf, "%lx", (dest) ? dest->ipc->proc : 0);
				query_data = buf;
				break;

			// Argument
			case 'a':
				if (handle->args)
				{
					struct ArgArrayEntry *entry;

					// Get first entry
					entry = (struct ArgArrayEntry *)handle->args->aa_List.mlh_Head;

					// Get data
					if (entry->ae_Node.mln_Succ)
						query_data = entry->ae_String;
				}
				break;
			}

			// Valid data? Add on to function
			if (query_data)
			{
				StrConcat(function_buf, query_data, max_len);
				func_pos += strlen(query_data);
			}
		}
		break;

		// Environment variable
		case FUNC_VARIABLE: {
			short varpos = 0;
			short len;

			// Extract variable name
			for (pos++; pos < func_len; pos++)
			{
				if (instruction[pos] == FUNC_END_ARG)
					break;
				if (varpos < 79)
					handle->work_buffer[varpos++] = instruction[pos];
			}
			handle->work_buffer[varpos] = 0;

			// Get variable
			if ((len = GetVar(handle->work_buffer, handle->work_buffer + 80, 512, 0)) > 0)
			{
				// Add value on to function
				StrConcat(function_buf, handle->work_buffer + 80, max_len);
				func_pos += len;
			}
		}
		break;

		// Request arguments
		case FUNC_REQUESTER: {
			short ret;

			// Handle requester
			ret = func_requester(handle, instruction, function_buf, max_len, func_len, &pos);

			// Aborted?
			if (ret == -1)
			{
				cont_flag = PARSE_ABORT;
				abort = 1;
			}

			// Update position
			else
				func_pos += ret;
		}
		break;

		// Store in buffer
		default:

			// Is this a space, with an outstanding quote flag?
			if (ch == ' ' && quote_flag)
			{
				// Add an end quote
				function_buf[func_pos++] = '\"';
				quote_flag = 0;
			}
			function_buf[func_pos++] = ch;
			function_buf[func_pos] = 0;
			break;
		}

		// Break out?
		if (abort)
			break;
	}

	// Outstanding quote flag?
	if (quote_flag && func_pos < limit)
	{
		// Add an end quote
		function_buf[func_pos++] = '\"';
		function_buf[func_pos] = 0;
	}

	// Check function is null-terminated
	function_buf[limit] = 0;

	// Return flag indicates whether we need to repeat this line
	return cont_flag;
}

// Add a filename to a function string
int function_add_filename(char *func_string, char *filename1, char *filename2, int no_quote, short slash)
{
	short add_len = 0, len, limit;

	// Get length of function string
	len = strlen(func_string);

	// If this name is hard up against other text, we don't quote
	if (len > 0 && func_string[len - 1] != ' ')
		no_quote = 1;

	// Get cll limit
	if ((limit = environment->env->settings.command_line_length) < 256)
		limit = 256;

	// Quote?
	if (!no_quote)
	{
		if (len < limit)
		{
			func_string[len++] = '\"';
			func_string[len] = 0;
			++add_len;
		}
	}

	// Filename one
	if (filename1)
	{
		short pos;

		for (pos = 0; filename1[pos] && len < limit; pos++, len++, add_len++)
			func_string[len] = filename1[pos];
		func_string[len] = 0;
	}

	// Filename two
	if (filename2)
	{
		short pos;

		for (pos = 0; filename2[pos] && len < limit; pos++, len++, add_len++)
			func_string[len] = filename2[pos];
		func_string[len] = 0;
	}

	// Trailing slash?
	if (slash && func_string[len - 1] != ':' && func_string[len - 1] != '/')
	{
		if (len < limit)
		{
			// Add slash
			func_string[len++] = '/';
			func_string[len] = 0;
			++add_len;
		}
	}

	// Quote?
	if (!no_quote)
	{
		if (len < limit)
		{
			func_string[len++] = '\"';
			func_string[len] = 0;
			++add_len;
		}
	}

	// Return length we added
	return add_len;
}

// Build the "default value" for a requester
void function_build_default(FunctionHandle *handle, char *def_string, char *buffer)
{
	short len, str_pos, buf_pos;

	// Get length of string
	len = strlen(def_string);

	// Go through string until end or buffer is full
	for (str_pos = 0, buf_pos = 0; str_pos < len && buf_pos < 511; str_pos++)
	{
		// [] sequence?
		if (def_string[str_pos] == '[' && def_string[str_pos + 2] == ']')
		{
			BOOL valid = 1;
			char *ptr;

			// Look at code
			switch (def_string[str_pos + 1])
			{
			// Pathname
			case 'f':
				strncpy(&buffer[buf_pos], handle->last_filename, 511 - buf_pos);
				break;

			// Filename
			case 'o':
				if ((ptr = FilePart(handle->last_filename)))
					strncpy(&buffer[buf_pos], ptr, 511 - buf_pos);
				break;

			// Source
			case 's':
				strncpy(&buffer[buf_pos], handle->source_path, 511 - buf_pos);
				break;

			// Destination
			case 'd':
				strncpy(&buffer[buf_pos], handle->dest_path, 511 - buf_pos);
				break;

			// Other
			default:
				valid = 0;
				break;
			}

			// Valid sequence?
			if (valid)
			{
				buf_pos = strlen(buffer);
				str_pos += 2;
				continue;
			}
		}

		// Copy character to buffer
		buffer[buf_pos++] = def_string[str_pos];
	}

	// Null-terminate buffer
	buffer[buf_pos] = 0;
}

// See if ReadArgs arguments need parsing
void function_parse_arguments(FunctionHandle *handle, InstructionParsed *ins)
{
	char *ptr, *buf;
	short arg, limit;

	// Got argumnts and a template key?
	if (!ins->funcargs || !ins->command->template_key)
		return;

	// Already done this?
	if (ins->funcargs->FA_DoneArgs)
		return;

	// Set flag to say we've done this
	ins->funcargs->FA_DoneArgs = 1;

	// Get cll limit
	if ((limit = environment->env->settings.command_line_length) < 256)
		limit = 256;

	// Allocate buffer
	if (!(buf = AllocVec(limit + 2, 0)))
		return;

	// Go through template
	for (ptr = ins->command->template_key; *ptr; ptr++)
	{
		// String argument?
		if (*ptr == FUNCKEY_FILE || *ptr == FUNCKEY_FILENO || *ptr == FUNCKEY_SOURCE || *ptr == FUNCKEY_DIR ||
			*ptr == FUNCKEY_ARG)
		{
			// Move to next character
			++ptr;

			// Get argument number
			arg = atoi(ptr);

			// Skip number
			while (isdigit(*(ptr + 1)))
				++ptr;

			// Got a string for this argument?
			if (ins->funcargs->FA_ArgArray[arg])
			{
				// Parse argument
				function_build_instruction(handle, ins, (unsigned char *)ins->funcargs->FA_ArgArray[arg], buf);

				// Has it changed?
				if (strcmp((char *)ins->funcargs->FA_ArgArray[arg], buf))
				{
					// Allocate copy
					if ((ins->funcargs->FA_Arguments[arg] = (IPTR)AllocVec(strlen((char *)buf) + 1, 0)))
					{
						// Copy new argument string
						strcpy((char *)ins->funcargs->FA_Arguments[arg], buf);
					}
				}
			}
		}
	}

	// Free buffer
	FreeVec(buf);
}

// Handle requesters
short func_requester(FunctionHandle *handle,
					 unsigned char *instruction,
					 char *buf,
					 short max_len,
					 short func_len,
					 short *position)
{
	short func_pos;
	short pos;

	// Get position
	pos = *position;

	// Use an existing argument?
	if ((handle->func_iterations > 0 || handle->inst_flags & INSTF_REPEAT) && handle->func_current_arg &&
		handle->func_current_arg->ln_Succ)
	{
		// Add argument to function string
		StrConcat(buf, handle->func_current_arg->ln_Name, max_len);
		func_pos = strlen(handle->func_current_arg->ln_Name);

		// Get next argument
		handle->func_current_arg = handle->func_current_arg->ln_Succ;

		// Skip through parse string to end of argument entry
		for (++pos; instruction[pos] != FUNC_END_ARG && pos < func_len; pos++)
			;
	}

	// Otherwise, prompt for an argument
	else
	{
		char *default_val, *req_title, *temp_buffer;
		BOOL def_flag = 0;
		short arg_pos = 0;
		char type;
		short ret = 0;

		// Initialise buffer pointers
		default_val = handle->work_buffer;
		*default_val = 0;
		req_title = handle->work_buffer + 256;
		*req_title = 0;
		temp_buffer = handle->work_buffer + 512;
		*temp_buffer = 0;

		// Get requester type
		type = instruction[++pos];

		// Go through parse string to get title and default value
		for (++pos; instruction[pos] != FUNC_END_ARG && pos < func_len; pos++)
		{
			// Defining title?
			if (!def_flag)
			{
				// Colon marks the end of the title
				if (instruction[pos] == ':')
				{
					def_flag = 1;
					arg_pos = 0;
				}

				// Otherwise store in title buffer
				else if (arg_pos < 79)
				{
					req_title[arg_pos++] = instruction[pos];
					req_title[arg_pos] = 0;
				}
			}

			// Default value
			else if (arg_pos < 255)
			{
				default_val[arg_pos++] = instruction[pos];
				default_val[arg_pos] = 0;
			}
		}

		// If no valid title, get default title
		if (!req_title[0])
		{
			strcpy(req_title,
				   GetString(&locale,
							 (type == 'f') ? MSG_SELECT_FILE
										   : ((type == 'd') ? MSG_SELECT_DIR
															: ((type == 'F') ? MSG_ENTER_FILE : MSG_ENTER_ARGUMENTS))));
		}

		// If default value supplied, build default string
		if (def_flag)
		{
			function_build_default(handle, default_val, temp_buffer);
		}

		// String requester?
		if (type == 's' || type == 'S')
		{
			// Put up requester
			ret = function_request(handle,
								   req_title,
								   (type == 's') ? SRF_BUFFER : SRF_BUFFER | SRF_SECURE,
								   temp_buffer,
								   256,
								   GetString(&locale, MSG_OKAY),
								   GetString(&locale, MSG_CANCEL),
								   0);
		}

		// File requester?
		else if (type == 'f' || type == 'F' || type == 'd')
		{
			char *file;
			ULONG flags;

			// Get file pointer
			if ((file = FilePart(temp_buffer)))
			{
				// Copy file
				strcpy(default_val, file);
				*file = 0;
			}
			else
				*default_val = 0;

			// Get flags
			flags = FRF_PRIVATEIDCMP;
			if (type == 'F')
				flags |= FRF_DOSAVEMODE;

			// Set coordinates
			*((struct IBox *)&handle->filereq->fr_LeftEdge) = handle->coords;

			// Put up requester
			if ((ret = AslRequestTags(handle->filereq,
									  ASLFR_Screen,
									  GUI->screen_pointer,
									  ASLFR_TitleText,
									  req_title,
									  ASLFR_InitialFile,
									  default_val,
									  ASLFR_InitialDrawer,
									  temp_buffer,
									  ASLFR_Flags1,
									  flags,
									  ASLFR_Flags2,
									  (type == 'd') ? FRF_DRAWERSONLY : 0,
									  TAG_END)))
			{
				// Copy path to buffer
				strcpy(temp_buffer, handle->filereq->fr_Drawer);
				AddPart(temp_buffer, handle->filereq->fr_File, 256);
			}

			// Save coordinates
			handle->coords = *((struct IBox *)&handle->filereq->fr_LeftEdge);
		}

		// Cancelled?
		if (!ret)
		{
			function_abort(handle);
			func_pos = -1;
		}

		// Store argument in list
		else
		{
			struct Node *node;

			// Create new node
			if ((node = AllocMemH(handle->memory, sizeof(struct Node) + strlen(temp_buffer) + 1)))
			{
				// Fill in argument value
				node->ln_Name = (char *)(node + 1);
				strcpy(node->ln_Name, temp_buffer);

				// Add to argument list
				AddTail(&handle->func_arguments, node);
			}

			// Add argument to function string
			StrConcat(buf, temp_buffer, max_len);
			func_pos = strlen(temp_buffer);
		}
	}

	// Save position
	*position = pos;

	return func_pos;
}
