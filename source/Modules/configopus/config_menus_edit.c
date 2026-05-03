#include "config_lib.h"
#include "config_menus.h"
#include "config_buttons.h"

// Got a new name
void config_menus_new_name(config_menus_data *data, short type)
{
	Att_Node *sel;
	Cfg_ButtonFunction *func;
	short which;
	char *name;
	short flags;

	// Get selection
	which = GetGadgetValue(data->objlist, GAD_MENUS_MENU + type);
	sel = Att_FindNode(data->menu_list[type], which);

	// Not found?
	if (!sel)
		return;

	// Get function pointer
	func = ((menu_node *)sel->data)->func;

	// Get new name pointer
	name = (char *)GetGadgetValue(data->objlist, GAD_MENUS_MENU_NAME + type);

	// Has name not changed?
	if (sel->node.ln_Name && strcmp(name, sel->node.ln_Name) == 0)
		return;

	// Detach list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

	// Change node name
	Att_ChangeNodeName(sel, name);

	// Remember node flags
	flags = ((menu_node *)sel->data)->flags;

	// Separator?
	if (strcmp(name, "---") == 0)
	{
		// Set flag
		((menu_node *)sel->data)->flags |= MNF_SEP;
	}

	// Clear flag
	else
		((menu_node *)sel->data)->flags &= ~MNF_SEP;

	// Flags changed?
	if (flags != ((menu_node *)sel->data)->flags)
	{
		// Check disable for sub-items
		if (type == MENU_ITEM)
		{
			// Disable/enable sub-item list
			DisableObject(data->objlist, GAD_MENUS_SUB, !(flags & MNF_SEP));
		}
	}

	// Reattach list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// Got a function?
	if (func)
	{
		// Free the old function label
		FreeMemH(func->label);
		func->label = 0;

		// Allocate new label copy
		if (name && (func->label = AllocMemH(data->bank->memory, strlen(name) + 1)))
		{
			// Copy label
			strcpy(func->label, name);
		}

		// Clear name pointer (use label)
		FreeMemH(func->node.ln_Name);
		func->node.ln_Name = 0;
	}

	// Set change flag
	data->change = 1;
}

// Edit a function
void config_menus_edit_item(config_menus_data *data, short type)
{
	Att_Node *node;
	menu_node *edit, *look;
	FunctionStartup *startup;
	Cfg_Function *edit_func;
	BOOL success = 0;

	// Can only edit menus for startmenus/lister menu
	if (type == MENU_MENU && data->type != MENUTYPE_START && data->type != MENUTYPE_LISTER)
		return;

	// Get selected item
	if (!(node = Att_FindNode(data->menu_list[type], GetGadgetValue(data->objlist, GAD_MENUS_MENU + type))))
		return;

	// Get node pointer
	edit = (menu_node *)node->data;

	// Can't edit separators
	if (edit->flags & MNF_SEP)
	{
		DisplayBeep(data->window->WScreen);
		return;
	}

	// See if this button is already being edited
	if ((look = config_menus_find_editor(data, edit->button, edit->func)))
	{
		// Activate the editor
		IPC_Command(look->ipc, IPC_ACTIVATE, 0, 0, 0, 0);
		return;
	}

	// Allocate startup data
	if (!(startup = AllocVec(sizeof(FunctionStartup), MEMF_CLEAR)))
		return;

	// Copy function (if it exists)
	if (edit->func)
		edit_func = CopyFunction((Cfg_Function *)edit->func, 0, 0);

	// Otherwise, allocate a new function
	else
		edit_func = NewFunction(0, 0);

	// Valid function to edit?
	if (edit_func)
	{
		// Allocate edit node
		if ((look = AllocVec(sizeof(menu_node), MEMF_CLEAR)))
		{
			// Fill out edit node
			look->button = edit->button;
			look->func = edit->func;

			// Initialise startup data
			function_editor_init(startup, data->command_list);
			startup->window = data->window;
			startup->function = edit_func;
			startup->owner_ipc = data->ipc;
			startup->main_owner = data->owner_ipc;
			startup->object = look;

			// Need image field?
			if (data->type == MENUTYPE_START)
				startup->flags |= FUNCEDF_LABEL | FUNCEDF_IMAGE;

			// Launch editor
			if ((IPC_Launch(&data->proc_list,
							&look->ipc,
							"dopus_function_editor",
							(ULONG)IPC_NATIVE(FunctionEditor),
							STACK_DEFAULT,
							(ULONG)startup,
							(struct Library *)DOSBase)) &&
				look->ipc)
			{
				// Add to edit list
				AddTail(&data->edit_list, (struct Node *)look);
				success = 1;
			}

			// Failed
			else
				FreeVec(look);
		}
	}

	// Free data if not successful
	if (!success)
	{
		FreeVec(startup);
		FreeFunction(edit_func);
	}
}

// Receive an edited function
void config_menus_receive_edit(config_menus_data *data, FunctionReturn *ret)
{
	menu_node *ret_node;
	Cfg_Button *button;

	// Get node pointer
	ret_node = (menu_node *)ret->object;

	// Try to find button in list
	for (button = (Cfg_Button *)data->bank->buttons.lh_Head; button->node.ln_Succ;
		 button = (Cfg_Button *)button->node.ln_Succ)
	{
		// Match with button in edit node
		if (button == ret_node->button)
		{
			Cfg_ButtonFunction *func;

			// Find function in button
			for (func = (Cfg_ButtonFunction *)button->function_list.mlh_Head; func->node.ln_Succ;
				 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
			{
				// Match with function in edit node
				if (func == ret_node->func)
				{
					// Free old function
					FreeInstructionList((Cfg_Function *)func);

					// Copy new function in
					CopyFunction(ret->function, 0, (Cfg_Function *)func);

					// Set change flag
					data->change = 1;
					return;
				}
			}
		}
	}
}

// Find an editor for a button/function
menu_node *config_menus_find_editor(config_menus_data *data, Cfg_Button *button, Cfg_ButtonFunction *func)
{
	menu_node *look;

	// Go through edit list
	for (look = (menu_node *)data->edit_list.lh_Head; look->node.mln_Succ; look = (menu_node *)look->node.mln_Succ)
	{
		// See if this function matches
		if (look->button == button && look->func == func)
			return look;
	}

	return 0;
}

// Kill any editors for a button
void config_menu_kill_button_editors(config_menus_data *data, Cfg_Button *button)
{
	menu_node *look;

	// Go through edit list
	for (look = (menu_node *)data->edit_list.lh_Head; look->node.mln_Succ; look = (menu_node *)look->node.mln_Succ)
	{
		// See if this function matches the button
		if (look->button == button)
		{
			// Kill the editor
			IPC_Command(look->ipc, IPC_QUIT, 0, 0, 0, 0);
		}
	}
}

// Handle goodbye from a function editor
void config_menus_editor_goodbye(config_menus_data *data, IPCData *editor)
{
	menu_node *look;

	// Go through edit list
	for (look = (menu_node *)data->edit_list.lh_Head; look->node.mln_Succ; look = (menu_node *)look->node.mln_Succ)
	{
		// See if this node matches the editor
		if (look->ipc == editor)
		{
			// Remove and free edit node
			Remove((struct Node *)look);
			FreeVec(look);
			break;
		}
	}
}

// Check if menus have changed
BOOL config_menus_check_change(config_menus_data *data, BOOL can_save)
{
	char *name;
	short ret;

	// If not changed, ok
	if (!data->change || !data->window || !data->bank)
		return 1;

	// Get name
	if (!(name = FilePart(data->bank->path)) || !*name)
		name = GetString(locale, MSG_UNTITLED);

	// Make window busy
	SetWindowBusy(data->window);

	// Display requester
	ret = SimpleRequestTags(data->window,
							0,
							GetString(locale, (can_save) ? MSG_SAVE_DISCARD_CANCEL : MSG_DISCARD_CANCEL),
							GetString(locale, (can_save) ? MSG_LISTER_MENU_CHANGED : MSG_LISTER_MENU_CHANGED_NOSAVE),
							name);

	// Make window unbusy
	ClearWindowBusy(data->window);

	// Cancelled?
	if (!ret)
		return 0;

	// Save?
	if (ret == 1 && can_save)
	{
		// Try to save
		if (!(config_menus_save(data, 0)))
			return 0;
	}

	return 1;
}

// Move an item up or down
void config_menus_move(config_menus_data *data, short type, unsigned short code, unsigned short qual)
{
	short sel;
	Att_Node *node;
	BOOL move = 0, sort = 0;

	// Get selection
	if (!(node = Att_FindNode(data->menu_list[type], (sel = GetGadgetValue(data->objlist, GAD_MENUS_MENU + type)))))
		return;

	// Moving?
	if (qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
	{
		// Set flag
		move = 1;

		// Sorting?
		if (qual & IEQUALIFIER_CONTROL)
			sort = 1;

		// Remove list
		SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);
	}

	// Move up?
	if (code == CURSORUP)
	{
		// Sort?
		if (sort)
			config_menus_sort(data, type, 1);

		// Not at the head of the list?
		else if (node->node.ln_Pred->ln_Pred)
		{
			// Moving?
			if (move)
			{
				// Swap with the one above
				config_menus_swap(data, node, (Att_Node *)node->node.ln_Pred, type, 0);

				// Set change flag
				data->change = 1;
			}

			// Move selection up
			--sel;
		}
	}

	// Move down
	else
	{
		// Sort?
		if (sort)
			config_menus_sort(data, type, 0);

		// Not at the end of the list
		else if (node->node.ln_Succ->ln_Succ)
		{
			// Moving?
			if (move)
			{
				// Swap with the one below
				config_menus_swap(data, node, (Att_Node *)node->node.ln_Succ, type, 0);

				// Set change flag
				data->change = 1;
			}

			// Move selection down
			++sel;
		}
	}

	// Set selection
	SetGadgetValue(data->objlist, GAD_MENUS_MENU + type, sel);

	// Reattach list
	if (move)
		SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// New selection
	if (!move || sort)
		config_menus_select_item(data, type, 1);
}

// Swap two nodes
void config_menus_swap(config_menus_data *data, Att_Node *node1, Att_Node *node2, short type, Att_Node *last_node)
{
	struct List temp[2];

	// Swapping?
	if (node2)
	{
		// Swap the two list nodes (easy!)
		SwapListNodes((struct List *)data->menu_list[type], (struct Node *)node2, (struct Node *)node1);
	}

	// Add to end of list
	else
	{
		// Remove from list
		Remove((struct Node *)node1);

		// Add to end
		AddTail((struct List *)data->menu_list[type], (struct Node *)node1);
	}

	// Swapping a whole menu? (gulp)
	if (type == MENU_MENU)
	{
		Att_Node *node;
		short a;

		// Build temporary lists
		for (a = 0, node = node1; a < 2 && node; a++, node = node2)
		{
			Cfg_Button *button, *next;

			// Initialise list
			NewList(&temp[a]);

			// Go through buttons that come under this menu
			for (button = ((menu_node *)node->data)->button; button->node.ln_Succ; button = next)
			{
				// Cache next button
				next = (Cfg_Button *)button->node.ln_Succ;

				// Skip title
				if (button == ((menu_node *)node->data)->button)
					continue;

				// Break out if we hit another title
				if (button->button.flags & BUTNF_TITLE)
					break;

				// Remove and add to temp list
				Remove((struct Node *)button);
				AddTail(&temp[a], (struct Node *)button);
			}
		}
	}

	// Is an item or a menu?
	if (type == MENU_ITEM || type == MENU_MENU)
	{
		// Swapping?
		if (node2)
		{
			// Swap the two buttons
			SwapListNodes(&data->bank->buttons,
						  (struct Node *)((menu_node *)node2->data)->button,
						  (struct Node *)((menu_node *)node1->data)->button);
		}

		// Add to end of list
		else
		{
			// Remove from list
			Remove((struct Node *)((menu_node *)node1->data)->button);

			// If a menu, add to end of list
			if (type == MENU_MENU)
			{
				// Add to end
				AddTail(&data->bank->buttons, (struct Node *)((menu_node *)node1->data)->button);
			}

			// Otherwise
			else if (last_node)
			{
				// Insert after last node
				Insert(&data->bank->buttons,
					   (struct Node *)((menu_node *)node1->data)->button,
					   (struct Node *)((menu_node *)last_node->data)->button);
			}
		}
	}

	// A sub-item
	else
	{
		// Swapping?
		if (node2)
		{
			// Swap the two functions
			SwapListNodes((struct List *)&((menu_node *)node2->data)->button->function_list,
						  (struct Node *)((menu_node *)node2->data)->func,
						  (struct Node *)((menu_node *)node1->data)->func);
		}

		// Add to end of list
		else
		{
			// Remove from list
			Remove((struct Node *)((menu_node *)node1->data)->func);

			// Add to end of function list
			AddTail((struct List *)&((menu_node *)node1->data)->button->function_list,
					(struct Node *)((menu_node *)node1->data)->func);
		}
	}

	// Swapping a menu?
	if (type == MENU_MENU)
	{
		Att_Node *node;
		Cfg_Button *last;
		short a;

		// Move contents of temporary lists back to main ones
		for (a = 0, node = node1; a < 2 && node; a++, node = node2)
		{
			Cfg_Button *button, *next;

			// Get parent button
			last = ((menu_node *)node->data)->button;

			// Go through lists
			for (button = (Cfg_Button *)temp[a].lh_Head; button->node.ln_Succ; button = next)
			{
				// Cache next button
				next = (Cfg_Button *)button->node.ln_Succ;

				// Add this button after the last
				Insert(&data->bank->buttons, (struct Node *)button, (struct Node *)last);

				// Remember last button
				last = button;
			}
		}
	}
}

// Sort a menu list
void config_menus_sort(config_menus_data *data, short type, short ascending)
{
	short n, gap, i, j;
	Att_Node **nodes, *node;

	// Get number of nodes
	n = Att_NodeCount(data->menu_list[type]);

	// Allocate array of nodes
	if (n < 2 || !(nodes = AllocVec(sizeof(Att_Node *) * n, MEMF_CLEAR)))
		return;

	// Fill in array
	for (node = (Att_Node *)data->menu_list[type]->list.lh_Head, i = 0; node->node.ln_Succ;
		 node = (Att_Node *)node->node.ln_Succ, i++)
	{
		nodes[i] = node;
	}

	// Shell sort
	for (gap = n >> 1; gap > 0; gap >>= 1)
	{
		for (i = gap; i < n; i++)
		{
			for (j = i - gap; j >= 0; j -= gap)
			{
				short val;

				// Compare node strings
				val = stricmp(nodes[j]->node.ln_Name, nodes[j + gap]->node.ln_Name);

				// Test against ascending/descending
				if ((ascending && val < 0) || (!ascending && val >= 0))
					break;

				// Swap the nodes
				config_menus_swap(data, nodes[j], nodes[j + gap], type, 0);

				// Swap the nodes
				node = nodes[j];
				nodes[j] = nodes[j + gap];
				nodes[j + gap] = node;
			}
		}
	}

	// Free node array
	FreeVec(nodes);
}
