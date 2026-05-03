#include "config_lib.h"
#include "config_menus.h"
#include "config_buttons.h"

// Find out which lister a drop occurred on
short config_menus_which_list(config_menus_data *data, short x, short y)
{
	short pos;
	GL_Object *object;

	// Find out where the drop occurred
	for (pos = GAD_MENUS_MENU; pos <= GAD_MENUS_SUB; pos++)
	{
		// In this area?
		if (CheckObjectArea((object = GetObject(data->objlist, pos)), x, y))
		{
			// If gadget is disabled, can't drop on it
			if (GADGET(object)->Flags & GFLG_DISABLED)
				return -1;

			// Return list number
			return (short)(pos - GAD_MENUS_MENU);
		}
	}

	return -1;
}

// Handle an AppMessage
void config_menus_appmsg(config_menus_data *data, struct AppMessage *msg)
{
	short type, num, ret = 0;

	// No files?
	if (msg->am_NumArgs < 1)
		return;

	// Not in a valid list?
	if ((type = config_menus_which_list(data, msg->am_MouseX, msg->am_MouseY)) == -1)
	{
		// Can't drop here
		DisplayBeep(data->window->WScreen);
		return;
	}

	// Make window busy
	SetWindowBusy(data->window);

	// Detach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

	// Go through files
	for (num = 0; num < msg->am_NumArgs; num++)
	{
		char path[256];

		// Get path
		GetWBArgPath(&msg->am_ArgList[num], path, 256);

		// Handle import
		if (!(config_menus_import_bank(data, type, path, 0)))
		{
			// Dropped in item or sub-item list?
			if (type == MENU_ITEM || type == MENU_SUB)
			{
				// Add function
				config_menus_add_func(data, type, &msg->am_ArgList[num]);
				ret = 1;
			}
			else
			{
				// Not a valid file
				SimpleRequestTags(data->window,
								  0,
								  GetString(locale, MSG_OK),
								  GetString(locale, MSG_INVALID_BUTTON_FILE),
								  FilePart(path));
				break;
			}
		}
		else
			ret = 1;
	}

	// Attach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// Refresh?
	if (ret)
	{
		// Select the last node
		SetGadgetValue(data->objlist, GAD_MENUS_MENU + type, Att_NodeCount(data->menu_list[type]) - 1);

		// Select item
		config_menus_select_item(data, type, 1);
	}

	// Clear window busy
	ClearWindowBusy(data->window);
}

// Import a button bank or a button
BOOL config_menus_import_bank(config_menus_data *data, short type, char *path, Cfg_Button *import_button)
{
	Cfg_ButtonBank *import = 0;
	Cfg_Button *button, *new_button, *add_pos = 0;

	// Load button bank
	if (!import_button && !(import = OpenButtonBank(path)))
		return 0;

	// Check bank isn't empty
	if (!import_button && IsListEmpty(&import->buttons))
	{
		// It is for some reason
		CloseButtonBank(import);
		return 1;
	}

	// Import as a menu?
	if (type == MENU_MENU)
	{
		// Get first button in bank
		button = (Cfg_Button *)import->buttons.lh_Head;

		// If this isn't a title, create one
		if (!(button->button.flags & BUTNF_TITLE))
		{
			// Create title button
			if ((new_button = NewButtonWithFunc(data->bank->memory, FilePart(path), FTYPE_LEFT_BUTTON)))
			{
				// Set title flag
				new_button->button.flags |= BUTNF_TITLE;

				// Add to end of list
				AddTail(&data->bank->buttons, &new_button->node);

				// Increment button row count
				data->bank->window.rows++;

				// Add to menu list
				config_menus_new_node(
					data->menu_list[MENU_MENU], new_button, (Cfg_ButtonFunction *)new_button->function_list.mlh_Head);

				// Set change flag
				data->change = 1;
			}
		}
	}

	// Import as item?
	else if (type == MENU_ITEM)
	{
		Att_Node *node;

		// Is list empty?
		if (IsListEmpty((struct List *)data->menu_list[MENU_ITEM]))
		{
			// Get parent node
			if (!(node = Att_FindNode(data->menu_list[MENU_MENU], GetGadgetValue(data->objlist, GAD_MENUS_MENU))))
			{
				// Error
				CloseButtonBank(import);
				return 1;
			}
		}

		// Get last item node
		else
			node = (Att_Node *)data->menu_list[MENU_ITEM]->list.lh_TailPred;

		// Add after this button
		add_pos = ((menu_node *)node->data)->button;
	}

	// Import as sub-item
	else
	{
		Att_Node *node;

		// Get parent node
		if (!(node = Att_FindNode(data->menu_list[MENU_ITEM], GetGadgetValue(data->objlist, GAD_MENUS_ITEM))))
		{
			// Error
			CloseButtonBank(import);
			return 1;
		}

		// Add to this button
		add_pos = ((menu_node *)node->data)->button;
	}

	// Go through buttons in import bank
	if (import_button)
		button = import_button;
	else
		button = (Cfg_Button *)import->buttons.lh_Head;
	do
	{
		// Clear 'new button' pointer
		new_button = 0;

		// Import as menu or item?
		if (type == MENU_MENU || type == MENU_ITEM)
		{
			// Skip title if adding as an item (unless a StartMenu)
			if (type == MENU_ITEM && button->button.flags & BUTNF_TITLE)
			{
				if (import_button)
					break;
				button = (Cfg_Button *)button->node.ln_Succ;
				continue;
			}

			// Copy button
			if ((new_button = CopyButton(button, data->bank->memory, 0)))
			{
				// If adding as an item, add after last one
				if (type == MENU_ITEM)
				{
					// Insert gadget
					Insert(&data->bank->buttons, &new_button->node, (struct Node *)add_pos);

					// Remember button for next time
					add_pos = new_button;
				}

				// Adding as menu
				else
				{
					// Add to end of list
					AddTail(&data->bank->buttons, &new_button->node);

					// If this is not a title, we don't add it
					if (!(new_button->button.flags & BUTNF_TITLE))
						new_button = 0;
				}

				// Increment button row count
				data->bank->window.rows++;

				// Set change flag
				data->change = 1;
			}
		}

		// Import as sub-item
		else
		{
			Cfg_ButtonFunction *func, *copy;

			// Go through button functions
			for (func = (Cfg_ButtonFunction *)button->function_list.mlh_Head; func->node.ln_Succ;
				 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
			{
				// Ignore empty functions
				if ((!function_label(func) || !*function_label(func)) &&
					IsListEmpty((struct List *)&func->instructions))
					continue;

				// Allocate copy function
				if ((copy = NewButtonFunction(data->bank->memory, 0)))
				{
					// Copy function
					CopyButtonFunction(func, data->bank->memory, copy);

					// Add function to button
					AddTail((struct List *)&add_pos->function_list, &copy->node);

					// Add to menu list
					config_menus_new_node(data->menu_list[MENU_SUB], add_pos, copy);

					// Set change flag
					data->change = 1;
				}
			}
		}

		// Button to add?
		if (new_button)
		{
			// Add to menu list
			config_menus_new_node(
				data->menu_list[type], new_button, (Cfg_ButtonFunction *)new_button->function_list.mlh_Head);
		}

		// Get next
		if (import_button)
			break;
		button = (Cfg_Button *)button->node.ln_Succ;

	} while (button->node.ln_Succ);

	// Free import bank
	CloseButtonBank(import);

	return 1;
}

// Button dropped on editor
BOOL config_menus_clip_button(config_menus_data *data, Cfg_Button *button, Point *pos)
{
	short type, ret;

	// See where dropped
	if (!pos ||
		(type = config_menus_which_list(data, pos->x - data->window->LeftEdge, pos->y - data->window->TopEdge)) == -1)
	{
		// Can't drop here
		DisplayBeep(data->window->WScreen);
		return 0;
	}

	// Dropped on menu list?
	if (type == MENU_MENU)
	{
		short line;

		// Get line we dropped over
		if ((line = functioned_get_line(data->window,
										GetObject(data->objlist, GAD_MENUS_MENU),
										pos->x,
										pos->y,
										(struct Library *)IntuitionBase)) == -1)
		{
			// Can't drop here
			DisplayBeep(data->window->WScreen);
			return 0;
		}

		// Select this item
		SetGadgetValue(data->objlist, GAD_MENUS_MENU, line);
		config_menus_select_item(data, MENU_MENU, 0);

		// Handle drop as an item
		type = MENU_ITEM;
	}

	// Make window busy
	SetWindowBusy(data->window);

	// Detach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

	// Import the button
	ret = config_menus_import_bank(data, type, 0, button);

	// Attach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// Refresh?
	if (ret)
	{
		// Select the last node
		SetGadgetValue(data->objlist, GAD_MENUS_MENU + type, Att_NodeCount(data->menu_list[type]) - 1);

		// Select item
		config_menus_select_item(data, type, 1);
	}

	// Clear window busy
	ClearWindowBusy(data->window);
	return 1;
}

// End a drag operation
void config_menus_end_drag(config_menus_data *data, short drop, unsigned short qual)
{
	struct Window *window;
	ULONG id = 0;
	IPCData *ipc = 0;
	BOOL ok = 0;
	short type;

	// Not dragging something?
	if (!data->drag.drag)
		return;

	// End drag
	if (!(window = config_drag_end(&data->drag, drop)))
		return;

	// Not dropped on our window?
	if (window != data->window)
	{
		// Get window ID
		id = GetWindowID(window);

		// Forbid to get IPC
		Forbid();
		if (!(ipc = (IPCData *)GetWindowAppPort(window)))
			Permit();
	}

	// Unlock layerinfo
	UnlockLayerInfo(&data->window->WScreen->LayerInfo);

	// What were we dragging?
	for (type = MENU_MENU; type <= MENU_SUB; type++)
	{
		// This type?
		if (data->drag.drag_node->list == data->menu_list[type])
			break;
	}

	// Dropped on our own window?
	if (window == data->window)
	{
		// Handle movement of menu item
		config_menus_move_item(data,
							   type,
							   data->drag.drag_node,
							   data->drag.drag_x - data->window->LeftEdge,
							   data->drag.drag_y - data->window->TopEdge,
							   qual);
		return;
	}

	// Got IPC?
	if (ipc)
	{
		// Can only do this for buttons or functions
		if (type == MENU_ITEM || type == MENU_SUB)
		{
			// Button editor or a button bank?
			if (id == WINDOW_BUTTON_CONFIG || id == WINDOW_BUTTONS || id == WINDOW_FUNCTION_EDITOR)
			{
				// Send the button
				ok = config_drag_send_button(&data->drag,
											 ipc,
											 ((menu_node *)data->drag.drag_node->data)->button,
											 (type == MENU_SUB) ? ((menu_node *)data->drag.drag_node->data)->func : 0);
			}
		}

		// Permit now we've sent the message
		Permit();
	}

	// Failed?
	if (!ok)
		DisplayBeep(data->window->WScreen);
}

// Re-position an item via drag and drop
void config_menus_move_item(config_menus_data *data,
							short type,
							Att_Node *drag_node,
							short x,
							short y,
							unsigned short qual)
{
	short drop_type;
	short drop_line;
	Att_Node *drop_node = 0, *last_node = 0;
	Cfg_Button *send_button;

	// Get type we dropped over
	if ((drop_type = config_menus_which_list(data, x, y)) == -1)
		return;

	// Can't drag a whole menu unless within the same list
	if (type == MENU_MENU && drop_type != MENU_MENU)
	{
		// Error
		DisplayBeep(data->window->WScreen);
		return;
	}

	// Don't need line if dropped from somewhere to menu
	if (drop_type != MENU_MENU || type == MENU_MENU)
	{
		// Get line we dropped over
		drop_line = functioned_get_line(data->window,
										GetObject(data->objlist, GAD_MENUS_MENU + drop_type),
										data->window->LeftEdge + x,
										data->window->TopEdge + y,
										(struct Library *)IntuitionBase);

		// Get node we dropped over
		if (!(drop_node = Att_FindNode(data->menu_list[drop_type], drop_line)))
		{
			// Want to add to end (except sub-items)
			if (drop_type != MENU_SUB)
			{
				// Didn't drop over a node; get last node in list
				last_node = (Att_Node *)data->menu_list[drop_type]->list.lh_TailPred;

				// Invalid?
				if (!last_node || !last_node->node.ln_Succ)
				{
					// If a menu, ignore it
					if (drop_type == MENU_MENU)
						return;

					// Get parent
					if (!(last_node = Att_FindNode(data->menu_list[drop_type - 1],
												   GetGadgetValue(data->objlist, GAD_MENUS_MENU + drop_type - 1))))
						return;
				}
			}
		}

		// If we dropped onto ourselves, ignore
		if (drop_node == drag_node)
			return;

		// Did we drop to a 'child' list?
		if (drop_type > type)
		{
			// If we dropped the selected entry, we can't do it (can't drop into ourself)
			if (Att_FindNode(data->menu_list[type], GetGadgetValue(data->objlist, GAD_MENUS_MENU + type)) == drag_node)
			{
				// Error
				DisplayBeep(data->window->WScreen);
				return;
			}
		}
	}

	// Detach list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

	// Dropped into same list?
	if (drop_type == type)
	{
		// Handle the swap
		config_menus_swap(data, drag_node, drop_node, type, last_node);

		// Set change flag
		data->change = 1;
	}

	// Get button to send to ourselves
	else if ((send_button = config_drag_get_button(((menu_node *)drag_node->data)->button,
												   (type == MENU_ITEM) ? 0 : ((menu_node *)drag_node->data)->func)))
	{
		Point pos;
		Cfg_Button *button;
		Cfg_ButtonFunction *func;
		BOOL del;

		// Get button/function pointers
		button = ((menu_node *)drag_node->data)->button;
		func = ((menu_node *)drag_node->data)->func;

		// Get fake position
		pos.x = data->window->LeftEdge + x;
		pos.y = data->window->TopEdge + y;

		// Handle copy of button
		del = config_menus_clip_button(data, send_button, &pos);

		// Free temp button
		FreeButton(send_button);

		// Unless shift is down we delete the original
		if (del && !(qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)))
		{
			// Delete whole button?
			if (type == MENU_ITEM && button)
			{
				// Remove button from list
				Remove((struct Node *)button);

				// Free button
				FreeButton(button);
			}

			// Delete one function
			else if (type == MENU_SUB && func)
			{
				// Remove function from list
				Remove((struct Node *)func);

				// Free function
				FreeButtonFunction(func);
			}

			// See if node is still valid
			if (Att_FindNodeNumber(data->menu_list[type], drag_node) > -1)
			{
				// Remove node from menu list
				FreeVec((APTR)drag_node->data);
				Att_RemNode(drag_node);
			}
		}
	}

	// Reattach list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// Fix selection
	config_menus_select_item(data, type, 1);
}

// Add function from drop
void config_menus_add_func(config_menus_data *data, short type, struct WBArg *arg)
{
	Cfg_Instruction *ins;
	Cfg_Button *button;
	APTR item;

	// Create instruction
	if (!(ins = instruction_from_wbarg(arg, data->bank->memory)))
		return;

	// Create button
	if (!(button = button_create_drop(0, data->bank->memory, arg, ins, 0, (struct Library *)DOSBase, DOpusBase)))
	{
		// Failed
		FreeInstruction(ins);
		return;
	}

	// Need button?
	if (type == MENU_ITEM)
	{
		// Adding button
		item = button;
	}

	// Only need function
	else
	{
		Cfg_ButtonFunction *func;

		// Get function
		func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

		// Remove function
		Remove((struct Node *)func);

		// Free button
		FreeButton(button);

		// Get item pointer
		item = func;
	}

	// Add to list
	config_menus_add_item(data, type, 0, 0, item);
}
