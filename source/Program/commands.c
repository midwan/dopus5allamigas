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

#include <proto/configopus.h>
#include <proto/module.h>

extern const char *const dopus_modules[];
const char *const module_exclusions[] = {"configopus.module",
										 "read.module",
										 "show.module",
										 "play.module",
										 "listerformat.module",
										 "icon.module",
										 "register.module",
										 "print.module",
										 "diskcopy.module",
										 "format.module",
										 "about.module",
										 "update.module",
										 0};

// New default function
Cfg_Function *new_default_function(char *func, APTR memory)
{
	Cfg_Function *function;

	// Create function
	if ((function = NewFunction(memory, 0)))
	{
		Cfg_Instruction *ins;

		// Create instruction
		if ((ins = NewInstruction(memory, INST_COMMAND, func)))
			AddTail((struct List *)&function->instructions, (struct Node *)ins);
	}

	return function;
}

// Update disk-based commands
void update_commands(ULONG flag)
{
	// Just rexx commands?
	if (flag == SNIFF_REXX)
		init_commands_scan(SCAN_REXX);

	// Rexx and modules?
	else if (flag == SNIFF_BOTH)
		init_commands_scan((GUI->flags & GUIF_REXX) ? SCAN_BOTH : SCAN_MODULES);

	// User
	else if (flag == SNIFF_USER)
		init_commands_scan(SCAN_USER);
}

// Initialises the module and command discovery scan.
// See documents/dopus5/ModuleDiscovery.md for details on the discovery workflow.
void init_commands_scan(short type)
{
	struct AnchorPath *anchor;
	ModuleNode *node;
	long error;
	short a, pat;
	char *ptr;
	char *patterns[3] = {0};

	// Allocate anchor path
	if (!(anchor = AllocVec(sizeof(struct AnchorPath) + 256, MEMF_CLEAR)))
		return;

	// Initialise anchor path
	anchor->ap_Strlen = 256;
	anchor->ap_Flags = APF_DOWILD;

	// Get pointer to match on
	if (type == SCAN_BOTH)
	{
		patterns[0] = "PROGDIR:Modules/#?.(module|dopus5)";
		patterns[1] = "DOpus5:Modules/#?.(module|dopus5)";
	}
	else if (type == SCAN_MODULES)
	{
		patterns[0] = "PROGDIR:Modules/#?.module";
		patterns[1] = "DOpus5:Modules/#?.module";
	}
	else if (type == SCAN_USER)
		patterns[0] = "DOpus5:Commands/~(#?.info)";
	else
	{
		patterns[0] = "PROGDIR:Modules/#?.dopus5";
		patterns[1] = "DOpus5:Modules/#?.dopus5";
	}

	for (pat = 0; patterns[pat]; pat++)
	{
		ptr = patterns[pat];

		// Search for modules
		error = MatchFirst(ptr, anchor);

		// Continue while there's files
		while (!error)
		{
			BOOL ok = 1, real_module = 0;
			char *name_ptr = 0;

			// Skip the basedata.lha placeholder used to keep DOpus5:Commands
			// archived (lha drops empty directories on extraction). We don't
			// want it registered as a phantom user command.
			if (type == SCAN_USER && stricmp(anchor->ap_Info.fib_FileName, ".dummy") == 0)
			{
				error = MatchNext(anchor);
				continue;
			}

			// A user command?
			if (type == SCAN_USER)
			{
				// Use full name
				name_ptr = anchor->ap_Buf;
			}

			// A real module?
			else if (sufcmp(anchor->ap_Info.fib_FileName, ".module"))
			{
				// Set flag
				real_module = 1;

				// See if module is in the exclusion list
				for (a = 0; module_exclusions[a]; a++)
				{
					if (stricmp(anchor->ap_Info.fib_FileName, module_exclusions[a]) == 0)
					{
						ok = 0;
						break;
					}
				}
			}

			// Not an excluded module?
			if (ok)
			{
				// Get filename if name not already set
				if (!name_ptr)
					name_ptr = anchor->ap_Info.fib_FileName;

				// Lock modules list
				lock_listlock(&GUI->modules_list, TRUE);

				// Module already in list?
				if ((node = (ModuleNode *)FindNameI(&GUI->modules_list.list, name_ptr)))
				{
					// Check datestamp
					if (CompareDates(&node->date, &anchor->ap_Info.fib_Date) != 0)
					{
						// Update datestamp, ok to reread
						node->date = anchor->ap_Info.fib_Date;

						// Expunge functions for this module
						command_expunge(name_ptr);
					}

					// Don't reread
					else
						ok = 0;

					// Set 'temp' flag
					node->flags |= MNF_TEMP;
				}

				// Allocate new module
				else if ((node = AllocMemH(global_memory_pool, sizeof(ModuleNode) + strlen(name_ptr) + 1)))
				{
					// Fill out node, add to list
					node->node.ln_Name = (char *)(node + 1);
					strcpy(node->node.ln_Name, name_ptr);
					node->date = anchor->ap_Info.fib_Date;
					AddTail(&GUI->modules_list.list, &node->node);

					// Set 'temp' flag
					node->flags |= MNF_TEMP;
				}

				// Unlock modules list
				unlock_listlock(&GUI->modules_list);
			}

			// Not ok to read?
			if (!ok)
			{
			}

			// User command?
			else if (type == SCAN_USER)
			{
				// Add the command
				add_command(anchor->ap_Info.fib_FileName, anchor->ap_Info.fib_Comment, 0, anchor->ap_Buf, 0, 0, 0, 0, 0);
			}

			// ARexx module?
			else if (!real_module)
			{
				// Function to run
				lsprintf(anchor->ap_Buf, "DOpus5:Modules/%s %s init", anchor->ap_Info.fib_FileName, GUI->rexx_port_name);

				// Run rexx thing
				rexx_send_command(anchor->ap_Buf, FALSE);
			}

			// Otherwise
			else
			{
				struct Library *ModuleBase = NULL;
#ifdef __amigaos4__
				struct ModuleIFace *IModule = NULL;
#endif
				short ver = 0;
				// See if this is one of our modules
				for (a = 0; dopus_modules[a]; a++)
					if (stricmp(anchor->ap_Info.fib_FileName, dopus_modules[a]) == 0)
					{
						// Need newest version
						ver = LIB_VERSION;
						break;
					}

					// Try to open library
#ifdef __amigaos4__
				if (OpenLibIFace(anchor->ap_Buf, (APTR)&ModuleBase, (APTR)&IModule, ver))
#else
	#ifdef __AROS__
				if (!(ModuleBase = OpenLibrary(anchor->ap_Buf, ver)) && ver &&
					stricmp(anchor->ap_Info.fib_FileName, "ftp.module") == 0)
				{
					// AROS can fail the versioned open for ftp.module during command discovery.
					// Runtime command dispatch applies the same fallback in OpenModule().
					ModuleBase = OpenLibrary(anchor->ap_Buf, 0);
				}
				if (ModuleBase)
	#else
				if ((ModuleBase = OpenLibrary(anchor->ap_Buf, ver)))
	#endif
#endif
				{
					ModuleInfo *info;

					// Ask module to identify itself.
					// This retrieves module-wide metadata (name, function count, flags).
					if ((info = Module_Identify(-1)))
					{
						short num;
						CommandList *command;
						char helpname[256], *helpptr = 0;

						// Help available?
						if (info->flags & MODULEF_HELP_AVAILABLE && info->name)
						{
							// Copy module name, strip suffix
							strcpy(helpname, info->name);
							helpptr = helpname + strlen(helpname) - 1;
							while (helpptr >= helpname && *helpptr != '.')
								--helpptr;
							if (*helpptr == '.')
								*helpptr = 0;

							// Add '.guide' suffix
							strcat(helpname, ".guide");

							// Get pointer to help name
							helpptr = helpname;
						}

						// Go through module's functions
						for (num = 0; num < info->function_count; num++)
						{
							// Add command
							if ((command = add_command(info->function[num].name,
													   (char *)Module_Identify(num),
													   info->function[num].template,
													   anchor->ap_Info.fib_FileName,
													   info->function[num].flags,
													   0,
													   0,
													   helpptr,
													   0)))
							{
								// Set function ID
								command->function = info->function[num].id;

								// Got template?
								if (command->template && *command->template)
								{
									char *ptr;

									// Newline marks template key
									if ((ptr = strchr(command->template, '\n')))
									{
										// Set template key pointer, and clear join
										command->template_key = ptr + 1;
										*ptr = 0;
									}
								}
							}
						}

						// Does module want to be called?
						if (info->flags & MODULEF_CALL_STARTUP)
						{
							// Run synchronously?
							if (info->flags & MODULEF_STARTUP_SYNC)
							{
								// Launch on our context
								Module_Entry(0,
											 0,
											 &main_ipc,
											 &main_ipc,
											 FUNCID_STARTUP,
											 (IPTR)GET_CALLBACK(function_external_hook));
							}

							// Run module function in background
							else if (misc_startup("dopus_module_init", MODULE_STARTUP, 0, ModuleBase, 0))
								ModuleBase = 0;
						}
					}
// Close module
#ifdef __amigaos4__
					DropInterface((struct Interface *)IModule);
#endif
					CloseLibrary(ModuleBase);
				}
			}

			// Find next file in directory
			error = MatchNext(anchor);
		}

		// Clean up match stuff
		MatchEnd(anchor);
	}

	FreeVec(anchor);

	// Lock modules list
	lock_listlock(&GUI->modules_list, TRUE);

	// Go through list
	for (node = (ModuleNode *)GUI->modules_list.list.lh_Head; node->node.ln_Succ;)
	{
		ModuleNode *next = (ModuleNode *)node->node.ln_Succ;

		// Wasn't found in directory?
		if (!(node->flags & MNF_TEMP))
		{
			// Module or rexx as appropriate
			if ((sufcmp(node->node.ln_Name, ".dopus5") && type & SCAN_REXX) ||
				(sufcmp(node->node.ln_Name, ".module") && type & SCAN_MODULES))
			{
				// Remove node
				Remove((struct Node *)node);

				// Expunge the module
				command_expunge(node->node.ln_Name);

				// Free the entry
				FreeMemH(node);
			}
		}

		// Otherwise, clear flag
		else
			node->flags &= ~MNF_TEMP;

		// Get next
		node = next;
	}

	// Unlock modules list
	unlock_listlock(&GUI->modules_list);
}

// Expunge functions
void command_expunge(char *name)
{
	CommandList *cmd, *next;
	struct Library *ModuleBase;
#ifdef __amigaos4__
	struct ModuleIFace *IModule;
#endif

	// Lock command list
	lock_listlock(&GUI->command_list, TRUE);

	// Scan command list
	for (cmd = (CommandList *)GUI->command_list.list.lh_Head; cmd->node.mln_Succ; cmd = next)
	{
		// Cache next
		next = (CommandList *)cmd->node.mln_Succ;

		// Does this match the module?
		if (cmd->flags & FUNCF_EXTERNAL_FUNCTION && stricmp(cmd->stuff.module_name, name) == 0)
		{
			// Remove from list
			Remove((struct Node *)cmd);

			// Remove PopUp extensions
			remove_popup_ext(cmd->name);

			// Free entry
			FreeMemH(cmd);
		}
	}

	// Unlock command list
	unlock_listlock(&GUI->command_list);

	// Try to open module
	if (sufcmp(name, ".module") &&
#ifdef __amigaos4__
		(OpenLibIFace(name, (APTR)&ModuleBase, (APTR)&IModule, LIB_VERSION)))
#else
		(ModuleBase = OpenLibrary(name, LIB_VERSION)))
#endif
	{
		// Expunge it
		Module_Expunge();

#ifdef __amigaos4__
		DropInterface((struct Interface *)IModule);
#endif
		CloseLibrary(ModuleBase);
	}
}

// Add a command
CommandList *add_command(char *name,
						 char *desc,
						 char *temp,
						 char *file,
						 ULONG flags,
						 char *menu,
						 Att_List *type_list,
						 char *help_name,
						 ULONG ext_flags)
{
	CommandList *command, *old;
	short size;
	char *ptr;

	// Get size
	size = sizeof(CommandList);
	size += strlen(name) + 1;
	if (desc)
		size += strlen(desc) + 1;
	if (temp)
		size += strlen(temp) + 1;
	if (file)
		size += strlen(file) + 1;

	// Allocate command
	if (!(command = AllocMemH(global_memory_pool, size)))
		return 0;

	// Pointer to buffer
	ptr = (char *)(command + 1);

	// Fill in name
	command->name = ptr;
	strcpy(command->name, name);
	ptr += strlen(name) + 1;

	// Description
	if (desc)
	{
		command->desc = (IPTR)ptr;
		strcpy((char *)command->desc, desc);
		ptr += strlen(desc) + 1;
	}

	// Template
	if (temp)
	{
		command->template = ptr;
		strcpy(command->template, temp);
		ptr += strlen(temp) + 1;
	}

	// Module name
	if (file)
	{
		command->stuff.module_name = ptr;
		strcpy(command->stuff.module_name, file);
	}

	// Set flags
	command->flags = flags | FUNCF_EXTERNAL_FUNCTION;

	// Help available?
	if (help_name && *help_name)
	{
		// Allocate help name copy
		if ((command->help_name = AllocMemH(global_memory_pool, strlen(help_name) + 1)))
			strcpy(command->help_name, help_name);
	}

	// Lock command list
	lock_listlock(&GUI->command_list, TRUE);

	// See if this command exists
	if ((old = (CommandList *)FindNameI((struct List *)&GUI->command_list, command->name)))
	{
		// Remove it
		Remove((struct Node *)old);

		// Was the original command private?
		if (old->flags & FUNCF_PRIVATE)
		{
			// Make the new command private too
			command->flags |= FUNCF_PRIVATE;
		}

		// External module?
		if (old->flags & FUNCF_EXTERNAL_FUNCTION)
		{
			// Free entry
			FreeMemH(old);
		}

		// Otherwise, add it to the secondary list
		else
		{
			// Lock original command list
			lock_listlock(&GUI->original_cmd_list, TRUE);

			// Add to list
			AddTail((struct List *)&GUI->original_cmd_list, (struct Node *)old);

			// Unlock list
			unlock_listlock(&GUI->original_cmd_list);
		}
	}

	// Add to command list
	AddTail((struct List *)&GUI->command_list, (struct Node *)command);

	// Unlock command list
	unlock_listlock(&GUI->command_list);

	// Got a menu entry?
	if (menu && *menu && type_list)
		add_popup_ext(menu, type_list, command->name, ext_flags);
	return command;
}

// Find an internal function
CommandList *function_find_internal(char **name, short original)
{
	CommandList *command;
	char *nameptr;
	short a;

	// Invalid string?
	if (!(nameptr = *name) || !*nameptr)
		return 0;

	// Do this twice
	for (a = 0; a < 2; a++)
	{
		// Original?
		if (original)
		{
			// Lock it and get head
			lock_listlock(&GUI->original_cmd_list, FALSE);
			command = (CommandList *)GUI->original_cmd_list.list.lh_Head;
		}

		// Command list
		else
		{
			// Lock it and get head
			lock_listlock(&GUI->command_list, FALSE);
			command = (CommandList *)GUI->command_list.list.lh_Head;
		}

		// Go through command list
		for (; command->node.mln_Succ; command = (CommandList *)command->node.mln_Succ)
		{
			short len;

			// Valid name?
			if (command->name)
			{
				// Get length of this command name
				len = strlen(command->name);

				// See if command string matches
				if (strnicmp(command->name, nameptr, len) == 0 && (nameptr[len] == ' ' || nameptr[len] == 0))
				{
					// Bump name pointer to start of arguments (if any)
					nameptr += len;
					while (*nameptr == ' ')
						++nameptr;
					if (!*nameptr)
						nameptr = 0;

					// Return pointer to command
					*name = nameptr;

					// Unlock list
					unlock_listlock((original) ? &GUI->original_cmd_list : &GUI->command_list);
					return command;
				}
			}
		}

		// Unlock command list
		unlock_listlock((original) ? &GUI->original_cmd_list : &GUI->command_list);

		// If doing original, loop around for a try at the main list
		if (original)
			original = 0;
		else
			break;
	}

	// Not found
	return 0;
}

// Create a new command
void command_new(BackdropInfo *info, IPCData *ipc, char *filename)
{
	Cfg_Function *func, *edit_func = 0;
	D_S(struct FileInfoBlock, fib)
	char buffer[256];
	APTR iff = 0;
#ifndef __amigaos3__
	struct Library *ConfigOpusBase;
	#ifdef __amigaos4__
	struct ConfigOpusIFace *IConfigOpus;
	#endif
#endif

	// Initialise buffers
	fib->fib_Comment[0] = 0;

	// Given filename?
	if (filename)
	{
		// Can't open?
		if (!(iff = IFFOpen(filename, IFF_READ, ID_OPUS)))
			return;

		// Find function chunk
		if (IFFNextChunk(iff, ID_FUNC) == ID_FUNC)
		{
			// Read function
			edit_func = ReadFunction(iff, global_memory_pool, 0, 0);
		}

		// Close IFF file
		IFFClose(iff);
		iff = 0;

		// Couldn't get function?
		if (!edit_func)
		{
			DisplayBeep(info->window->WScreen);
			return;
		}

		// Get current comment
		GetFileInfo(filename, fib);
	}

	// Open config library
	if (!(ConfigOpusBase = OpenModule(config_name))
#ifdef __amigaos4__
		|| !(IConfigOpus = (struct ConfigOpusIFace *)GetInterface(ConfigOpusBase, "main", 1, NULL))
#endif
	)
	{
		if (ConfigOpusBase)
			CloseLibrary(ConfigOpusBase);
		FreeFunction(edit_func);
		return;
	}

	// Create a new command
	func = Config_EditFunction(
		ipc, &main_ipc, info->window, edit_func, global_memory_pool, (IPTR)&GUI->command_list.list);

	// Free edit function
	FreeFunction(edit_func);

	// Success?
	if (func)
	{
		short leaveout = 1;

		// Initialise file buffer
		if (filename)
			strcpy(buffer, filename);
		else
			*buffer = 0;

		// Ask user for a description
		if (super_request_args(info->window,
							   GetString(&locale, MSG_COMMAND_DESCRIPTION),
							   SRF_BUFFER | SRF_CHECKMARK,
							   fib->fib_Comment,
							   79,
							   GetString(&locale, MSG_COMMAND_LEAVEOUT),
							   &leaveout,
							   GetString(&locale, MSG_OKAY),
							   GetString(&locale, MSG_CANCEL),
							   0))
		{
			// Don't leave out?
			if (!leaveout)
				edit_func = (Cfg_Function *)1;

			// Loop to open file
			while (!iff)
			{
				// Get file to save (unless given filename)
				if (!filename && !(request_file(info->window,
												GetString(&locale, MSG_COMMAND_SAVE),
												buffer,
												"DOpus5:Commands/",
												FRF_DOSAVEMODE,
												0)))
					break;

				// Try to open file
				if (!(iff = IFFOpen(buffer, IFF_WRITE, ID_OPUS)))
				{
					// No longer use supplied filename
					filename = 0;

					// Show error
					if (!(error_request(info->window,
										FALSE,
										GetString(&locale, MSG_SAVING_COMMAND),
										(char *)-1,
										FilePart(buffer),
										"\n",
										0)))
						break;
				}
			}
		}
	}

	// Got file?
	if (iff)
	{
		// Save function
		SaveFunction(iff, func);

		// Close file
		IFFClose(iff);

		// Set the comment
		SetComment(buffer, fib->fib_Comment);

		// Only add as a leftout if not editing an existing function
		if (!edit_func)
		{
			// Leave the new function out
			if (backdrop_leave_out(info, buffer, BLOF_PERMANENT | BLOF_REFRESH, -1, -1))
			{
				// Save leftout list
				backdrop_save_leftouts(info);
			}
		}
	}

	// Free function
	FreeFunction(func);

// Close module
#ifdef __amigaos4__
	DropInterface((struct Interface *)IConfigOpus);
#endif
	CloseLibrary(ConfigOpusBase);
}

// Remove a command
void command_remove(char *name)
{
	CommandList *cmd, *next;

	// Lock command list
	lock_listlock(&GUI->command_list, TRUE);

	// Scan command list
	for (cmd = (CommandList *)GUI->command_list.list.lh_Head; cmd->node.mln_Succ; cmd = next)
	{
		// Cache next
		next = (CommandList *)cmd->node.mln_Succ;

		// Does this match the command?
		if (cmd->flags & FUNCF_EXTERNAL_FUNCTION && stricmp(cmd->name, name) == 0)
		{
			// Remove from list
			Remove((struct Node *)cmd);

			// Remove PopUp extensions
			remove_popup_ext(cmd->name);

			// Free entry
			FreeMemH(cmd);
		}
	}

	// Unlock command list
	unlock_listlock(&GUI->command_list);
}
