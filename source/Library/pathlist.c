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

#include "dopuslib.h"

// Places to look for a path
static const char FAR *const path_places[] =
	{"Workbench", "Initial CLI", "Shell Process", "New_WShell", "Background CLI", 0};

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	BPTR next;
	BPTR lock;
} PathListEntry;

#ifndef __amigaos3__
	#pragma pack()
#endif

// Get a usable path list
BPTR LIBFUNC L_GetDosPathList(REG(a0, BPTR copy_list))
{
	short num;
	struct Process *proc;
	struct CommandLineInterface *cli;
	PathListEntry *path, *new_entry, *last_entry = 0;
	BPTR new_path = 0;
	APTR file;

	// Start with current process
	proc = (struct Process *)FindTask(0);

	// See if path environment variable exists
	if ((file = L_OpenBuf("env:dopus/paths", MODE_OLDFILE, 4096)))
	{
		char buf[300];

		// Read paths from file
		while (L_ReadBufLine(file, buf, sizeof(buf) - 1) > 0)
		{
			// Allocate a new path entry
			if ((new_entry = AllocVec(sizeof(PathListEntry), 0)))
			{
				// Link to previous
				if (!new_path)
					new_path = MKBADDR(new_entry);
				else
					last_entry->next = MKBADDR(new_entry);
				last_entry = new_entry;
				new_entry->next = 0;

				// Get lock on path
				new_entry->lock = Lock(buf, ACCESS_READ);
			}
		}

		// Close file, if we got a path then return it
		L_CloseBuf(file);
		if (new_path)
			return new_path;
	}

	// Go through all possible places
	for (num = 0;; num++)
	{
		// Get CLI structure, see if it has a path
		if (copy_list ||
			(proc && (cli = (struct CommandLineInterface *)BADDR(proc->pr_CLI)) && (copy_list = cli->cli_CommandDir)))
		{
			// Better forbid for this
			Forbid();

			// Go through path list
			for (path = (PathListEntry *)BADDR(copy_list); path; path = BADDR(path->next))
			{
				// Allocate a new entry
				if ((new_entry = AllocVec(sizeof(PathListEntry), 0)))
				{
					// Link to previous
					if (!new_path)
						new_path = MKBADDR(new_entry);
					else
						last_entry->next = MKBADDR(new_entry);
					last_entry = new_entry;
					new_entry->next = 0;

					// Duplicate lock
					new_entry->lock = DupLock(path->lock);
				}
			}

			// Enable multitasking
			Permit();
			break;
		}

		// Nowhere else to look?
		if (!path_places[num])
			break;

		// Find next process
		else
			proc = (struct Process *)FindTask(path_places[num]);
	}

	// Return new path (if we got one)
	return new_path;
}

// Update our path list
void LIBFUNC L_UpdatePathList(REG(a6, struct MyLibrary *libbase))
{
	struct LibData *data;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Get library data
	data = (struct LibData *)libbase->ml_UserData;

	// Lock path list
	L_GetSemaphore(&data->path_lock, SEMF_EXCLUSIVE, 0);

	// Free path list
	L_FreeDosPathList(data->path_list);

	// Get new path list
	data->path_list = L_GetDosPathList(0);

	// Unlock path list
	L_FreeSemaphore(&data->path_lock);

	// Send command to launcher to reset it
	L_IPC_Command(launcher_ipc, IPC_RESET, 0, 0, 0, NO_PORT_IPC);
}

// Update a process path list
void LIBFUNC L_UpdateMyPaths(REG(a6, struct MyLibrary *libbase))
{
	BPTR pathlist;
	struct Process *proc;
	struct LibData *data;
	struct CommandLineInterface *cli;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Get library data
	data = (struct LibData *)libbase->ml_UserData;

	// Get this process
	proc = (struct Process *)FindTask(0);

	// Get CLI structure
	if (!(cli = (struct CommandLineInterface *)BADDR(proc->pr_CLI)))
		return;

	// Lock path list
	L_GetSemaphore(&data->path_lock, SEMF_SHARED, 0);

	// Get path list copy
	pathlist = L_GetDosPathList(data->path_list);

	// Unlock path list
	L_FreeSemaphore(&data->path_lock);

	// Got valid path list?
	if (pathlist)
	{
		// Free existing process path list
		if (cli->cli_CommandDir)
			L_FreeDosPathList(cli->cli_CommandDir);

		// Store new path list
		cli->cli_CommandDir = pathlist;
	}
}

// Free a path list
void LIBFUNC L_FreeDosPathList(REG(a0, BPTR list))
{
	PathListEntry *path;

	// Valid list?
	if (!(path = (PathListEntry *)BADDR(list)))
		return;

	// Go through list
	for (; path;)
	{
		PathListEntry *next = (PathListEntry *)BADDR(path->next);

		// Unlock this lock
		UnLock(path->lock);

		// Free this entry
		FreeVec(path);

		// Get next
		path = next;
	}
}

// Copy local environment variables to current process
void LIBFUNC L_CopyLocalEnv(REG(a0, struct Library *DOSBase))
{
	short num;
	struct Process *proc = 0;
	struct LocalVar *var;

	// Go through all possible places
	for (num = 0;; num++)
	{
		// Any variables set?
		if (proc && !(IsListEmpty((struct List *)&proc->pr_LocalVars)))
		{
			// Better forbid for this
			Forbid();

			// Go through variable list
			for (var = (struct LocalVar *)proc->pr_LocalVars.mlh_Head; var->lv_Node.ln_Succ;
				 var = (struct LocalVar *)var->lv_Node.ln_Succ)
			{
				// Is this a variable?
				if (var->lv_Node.ln_Type == LV_VAR)
				{
					// Copy this variable
					SetVar(var->lv_Node.ln_Name, var->lv_Value, var->lv_Len, var->lv_Flags | GVF_LOCAL_ONLY);
				}
			}

			// Enable multitasking
			Permit();
			break;
		}

		// Nowhere else to look?
		if (!path_places[num])
			break;

		// Find next process
		else
			proc = (struct Process *)FindTask(path_places[num]);
	}
}

// Get a copy of the Opus path list
BPTR LIBFUNC L_GetOpusPathList(REG(a6, struct MyLibrary *libbase))
{
	BPTR copy;
	struct LibData *data;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Get library data
	data = (struct LibData *)libbase->ml_UserData;

	// Lock path list
	L_GetSemaphore(&data->path_lock, SEMF_SHARED, 0);

	// Copy it
	copy = L_GetDosPathList(data->path_list);

	// Unlock path list
	L_FreeSemaphore(&data->path_lock);

	return copy;
}
