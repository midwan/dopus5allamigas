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

// Do a filetype on a function (? :-)
void function_filetype(FunctionHandle *handle)
{
	Cfg_Filetype *type, *last_type = 0;
	Cfg_Function *func, *type_function = 0, *old_function;
	struct List filechange_list;
	struct Node *node, *next = 0;
	unsigned short qual;

	// Extract qualifier
	qual = (handle->data >> 16) & 0xffff;
	handle->data &= 0xffff;

	// Backup function pointer
	old_function = handle->function;

	// Lock filetype list
	lock_listlock(&GUI->filetypes, FALSE);

	// Steal the filechange list
	NewList(&filechange_list);
	for (node = handle->filechange.lh_Head; node->ln_Succ; node = next)
	{
		next = node->ln_Succ;
		Remove(node);
		AddTail(&filechange_list, node);
	}

	// Got arguments?
	if (handle->args)
	{
		struct ArgArrayEntry *arg;

		// Go through list
		for (arg = (struct ArgArrayEntry *)handle->args->aa_List.mlh_Head; arg->ae_Node.mln_Succ;
			 arg = (struct ArgArrayEntry *)arg->ae_Node.mln_Succ)
		{
			// Match filetype for this file
			if (!(type = filetype_identify(arg->ae_String, handle->data, 0, qual)))
			{
				// Couldn't match. Was this a drag & drop operation?
				if (handle->flags & FUNCF_DRAG_DROP)
				{
					// Use "default" filetype (copy)
					type = default_filetype;
				}
			}

			// Got filetype?
			if (type)
			{
				// Get appropriate function
				if ((func = FindFunctionType(&type->function_list, handle->data)))
				{
					ExternalEntry *entry;

					// Different to the last type?
					if (last_type && type != last_type)
					{
						// Anything in the external list?
						if (!(IsListEmpty((struct List *)&handle->external_list)) && type_function)
						{
							Cfg_Function *copy;

							// Initialise handle function
							function_handle_init(handle, 1);

							// Copy the function
							if ((copy = CopyFunction(type_function, handle->memory, 0)))
							{
								// Set function pointer
								handle->function = copy;

								// Unlock the filetype list
								unlock_listlock(&GUI->filetypes);

								// Run function
								function_run_function(handle);

								// Free the function
								FreeFunction(copy);

								// Lock filetype list again
								lock_listlock(&GUI->filetypes, FALSE);

								// Steal new filechanges
								for (node = handle->filechange.lh_Head; node->ln_Succ; node = next)
								{
									next = node->ln_Succ;
									Remove(node);
									AddTail(&filechange_list, node);
								}
							}

							// Clear external list
							NewList((struct List *)&handle->external_list);
						}
					}

					// Save type pointer
					last_type = type;

					// Save function pointer
					type_function = func;

					// Create a new external entry
					if ((entry = new_external_entry(handle, arg->ae_String)))
					{
						// Add to external entry list
						AddTail((struct List *)&handle->external_list, (struct Node *)entry);
					}
				}
			}
		}
	}

	// Anything left in the external list?
	if (!(IsListEmpty((struct List *)&handle->external_list)) && type_function)
	{
		Cfg_Function *copy;

		// Initialise handle function
		function_handle_init(handle, 1);

		// Copy the function
		if ((copy = CopyFunction(type_function, handle->memory, 0)))
		{
			// Set function pointer
			handle->function = copy;

			// Unlock the filetype list
			unlock_listlock(&GUI->filetypes);

			// Run function
			function_run_function(handle);

			// Free the function
			FreeFunction(copy);

			// Lock filetype list again
			lock_listlock(&GUI->filetypes, FALSE);

			// Steal new filechanges
			for (node = handle->filechange.lh_Head; node->ln_Succ; node = next)
			{
				next = node->ln_Succ;
				Remove(node);
				AddTail(&filechange_list, node);
			}
		}
	}

	// Unlock the filetype list
	unlock_listlock(&GUI->filetypes);

	// Restore function pointer
	handle->function = old_function;

	// Copy filechange list back
	for (node = filechange_list.lh_Head; node->ln_Succ; node = next)
	{
		next = node->ln_Succ;
		Remove(node);
		AddTail(&handle->filechange, node);
	}
}
