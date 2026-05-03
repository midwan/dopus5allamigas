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

// Build PopUp menu from a button bank
PopUpHandle *popup_from_bank(Cfg_ButtonBank *bank, short *last_id)
{
	PopUpHandle *handle;
	PopUpItem *item = 0;
	Cfg_Button *button;
	short id, depth = 0;

	if (last_id)
		*last_id = 0;

	// No bank?
	if (!bank)
		return 0;

	// Create handle
	if (!(handle = PopUpNewHandle(0, 0, &locale)))
		return 0;

	// Go through menu bank
	for (button = (Cfg_Button *)bank->buttons.lh_Head, id = 0; button->node.ln_Succ;
		 button = (Cfg_Button *)button->node.ln_Succ)
	{
		Cfg_ButtonFunction *func;
		Cfg_Instruction *ins;
		char *label;
		BOOL sep = FALSE;
		short ins_count;

		/*
				D(bug("button flags %lx (title %lx item %lx sub %lx)\n",button->button.flags,
					button->button.flags&BUTNF_TITLE,button->button.flags&BUTNF_ITEM,button->button.flags&BUTNF_SUB));
				for (func=(Cfg_ButtonFunction
		   *)button->function_list.mlh_Head;func->node.ln_Succ;func=(Cfg_ButtonFunction *)func->node.ln_Succ)
				{
					Cfg_Instruction *ins;

					D(bug("name %s label %s flags %lx type
		   %ld\n",func->node.ln_Name,func->label,func->function.flags2,func->function.func_type)); for
		   (ins=(Cfg_Instruction *)func->instructions.mlh_Head;ins->node.mln_Succ;ins=(Cfg_Instruction
		   *)ins->node.mln_Succ) D(bug("\t%ld %s\n",ins->type,ins->string));
				}
				D(bug("------------\n"));
		*/

		// Get first function
		func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;
		if (!func->node.ln_Succ)
			continue;

		// Count the instructions
		for (ins = (Cfg_Instruction *)func->instructions.mlh_Head, ins_count = 0; ins->node.mln_Succ;
			 ins = (Cfg_Instruction *)ins->node.mln_Succ)
		{
			// Increment count if not a label
			if (ins->type != INST_LABEL)
				++ins_count;
		}

		// Get label pointer, see if this is a separator
		if (!(label = function_label(func)) || !*label)
			continue;
		if (strncmp(label, "---", 3) == 0)
			sep = TRUE;

		// Menu title?
		if (button->button.flags & BUTNF_TITLE)
		{
			// End existing sub-menu
			if (depth == 1)
			{
				PopUpEndSub(handle);
				depth = 0;
			}

			// Separator?
			if (sep)
				PopUpSeparator(handle);

			// Add a new item
			else if ((item = PopUpNewItem(handle, (ULONG)label, id++, POPUPF_STRING)))
			{
				// Save pointer to function in item data
				item->data = func;
			}
			continue;
		}

		// Entering item?
		if (depth == 0 && item)
		{
			// Store item data in userdata field (since subitem info will overwrite data field)
			item->userdata = item->data;
			item->flags |= POPUPF_USERDATA;

			// Go into sub-item
			if (PopUpItemSub(handle, item))
				item->flags |= POPUPF_SUB;
			depth = 1;
		}

		// Separator?
		if (sep)
			PopUpSeparator(handle);

		// Empty function, with sub-items?
		else if (ins_count < 1 && func->node.ln_Succ->ln_Succ)
		{
			// Add parent item
			if ((item = PopUpNewItem(handle, (ULONG)label, id++, POPUPF_STRING | POPUPF_SUB | POPUPF_USERDATA)))
			{
				// Initialise item for sub-items
				item->userdata = func;
				if (PopUpItemSub(handle, item))
				{
					// Go through sub-items
					for (func = (Cfg_ButtonFunction *)func->node.ln_Succ; func->node.ln_Succ;
						 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
					{
						// Get label pointer, see if this is a separator
						if (!(label = function_label(func)) || !*label)
							continue;
						if (strncmp(label, "---", 3) == 0)
							PopUpSeparator(handle);
						else if ((item = PopUpNewItem(handle, (ULONG)label, id++, POPUPF_STRING)))
						{
							// Save pointer to function in item data
							item->data = func;
						}
					}

					// End sub-items
					PopUpEndSub(handle);
				}
			}
		}

		// Create item
		else if ((item = PopUpNewItem(handle, (ULONG)label, id++, POPUPF_STRING)))
		{
			// Save pointer to function in item data
			item->data = func;
		}
	}

	// End existing sub-menu
	if (depth == 1)
		PopUpEndSub(handle);

	// Set flags
	if (bank->window.flags & BTNWF_NO_SCALE_IMAGES)
		handle->ph_Menu.flags |= POPUPMF_NO_SCALE;
	else
		handle->ph_Menu.flags &= ~POPUPMF_NO_SCALE;

	// Return ID
	if (last_id)
		*last_id = id;
	return handle;
}
