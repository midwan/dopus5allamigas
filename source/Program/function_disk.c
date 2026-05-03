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
#include <proto/module.h>

// DISKCOPY, FORMAT internal functions
DOPUS_FUNC(function_disk)
{
	struct Library *ModuleBase = NULL;
#ifdef __amigaos4__
	struct ModuleIFace *IModule;
#endif
	struct List list;
	struct Node *node;
	short ret = 0;

	// Initialise list
	NewList(&list);

	// Any arguments?
	if (handle->args)
	{
		struct ArgArrayEntry *arg;

		// Add arg nodes
		for (arg = (struct ArgArrayEntry *)handle->args->aa_List.mlh_Head; arg->ae_Node.mln_Succ;
			 arg = (struct ArgArrayEntry *)arg->ae_Node.mln_Succ)
		{
			if ((node = AllocMemH(handle->memory, sizeof(struct Node))))
			{
				node->ln_Name = arg->ae_String;
				AddTail(&list, node);
			}
		}
	}

	// Parsed arguments?
	else if (instruction->funcargs)
	{
		// Got a disk?
		if (instruction->funcargs->FA_Arguments[0])
		{
			// Add a node
			if ((node = AllocMemH(handle->memory, sizeof(struct Node))))
			{
				node->ln_Name = (char *)instruction->funcargs->FA_Arguments[0];
				AddTail(&list, node);
			}
		}
	}

	// Open disk module
	switch (command->function)
	{
	// Diskcopy
	case FUNC_DISKCOPY:
		ModuleBase = OpenModule("diskcopy.module");
#ifdef __amigaos4__
		if (!(IModule = (struct ModuleIFace *)GetInterface(ModuleBase, "main", 1, NULL)))
		{
			CloseLibrary(ModuleBase);
			ModuleBase = NULL;
		}
#endif
		break;

	// Format
	case FUNC_FORMAT:
		ModuleBase = OpenModule("format.module");
#ifdef __amigaos4__
		if (!(IModule = (struct ModuleIFace *)GetInterface(ModuleBase, "main", 1, NULL)))
		{
			CloseLibrary(ModuleBase);
			ModuleBase = NULL;
		}
#endif
		break;
	}

	if (ModuleBase)
	{
		// Call module
		ret = Module_Entry(&list, GUI->screen_pointer, handle->ipc, &main_ipc, 0, 0);
#ifdef __amigaos4__
		DropInterface((struct Interface *)IModule);
#endif
		CloseLibrary(ModuleBase);
	}

	return ret;
}
