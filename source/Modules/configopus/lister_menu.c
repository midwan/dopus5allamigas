#include "config_lib.h"
#include "lister_menu.h"
#include "config_buttons.h"

BOOL buttoned_function_empty(Cfg_ButtonFunction *);

ULONG LIBFUNC L_Config_Menu(REG(a0, char *menu_name),
							REG(a1, IPCData *ipc),
							REG(a2, IPCData *owner_ipc),
							REG(a3, struct Screen *screen),
                            REG(d0, Cfg_ButtonBank *def_bank),
                            REG(d1, char *title),
                            REG(d2, IPTR command_list),
                            REG(d3, char *default_name),
                            REG(d4, short type),
                            REG(d5, Att_List *script_list))
{
	lister_menu_data *data;
	IPCMessage *quit_msg = 0;
	short undo_flag = 0, pending_quit = 0;
	Cfg_Button *button;
	Cfg_ButtonBank *ret_code = 0;

    // Allocate data
	if (!(data = AllocVec(sizeof(lister_menu_data), MEMF_CLEAR)) || !(data->drag.timer = AllocTimer(UNIT_VBLANK, 0)))
	{
		FreeVec(data);
		return 0;
	}

	// Initialise data
	strcpy(data->menu_name, menu_name);
	data->ipc = ipc;
	data->owner_ipc = owner_ipc;
	InitListLock(&data->proc_list, 0);
	NewList(&data->edit_list);
	data->title = title;
	data->command_list = command_list;
	data->app_port = CreateMsgPort();
	data->default_name = default_name;
	data->type = type;
	data->script_list = script_list;

	// Initial dimensions
	data->pos_valid = LoadPos((type == TYPE_HOTKEYS) ? "dopus/windows/hotkeys" : "dopus/windows/scripts",
							  &data->window_pos,
							  &data->font_size);

	// Load menu
	if (!(lister_menu_load_menu(data, def_bank, 0)))
	{
		lister_menu_close(data, 1);
		return 0;
	}

	// Open window
	if (!(lister_menu_open(data, screen)))
	{
		lister_menu_close(data, 1);
		return 0;
	}

	// Backup bank
	data->restore_bank = CopyButtonBank(data->bank);
	strcpy(data->last_save_name, data->menu_name);

	// Event loop
	FOREVER
	{
		struct IntuiMessage *msg;
		IPCMessage *imsg;
		BOOL quit_flag = 0;

		// Check drag
		if (config_drag_check(&data->drag))
		{
			// End drag
			lister_menu_end_drag(data, 0);
		}

		// AppMessages?
		if (data->app_window)
		{
			struct AppMessage *msg;

			while ((msg = (struct AppMessage *)GetMsg(data->app_port)))
			{
				short num, count = 0;
				Cfg_ButtonFunction *function = 0;
				Att_Node *node = 0;

				// Go through arguments
				for (num = 0; num < msg->am_NumArgs; num++)
				{
					// File?
					if (msg->am_ArgList[num].wa_Name && *msg->am_ArgList[num].wa_Name)
					{
						// Is this the first file?
						if (count++ == 0)
						{
							Cfg_Button *button;

							// Create a new entry
							if (!(node = lister_menu_add_item(data, 0)))
								break;

							// Detach list
							SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

							// Set new name
							Att_ChangeNodeName(node, msg->am_ArgList[num].wa_Name);

							// Reattach list
							SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)data->menu_list);

							// Get button pointer
							button = (Cfg_Button *)node->data;

							// Create a new function
							if ((function = NewButtonFunction(data->bank->memory, 0)))
							{
								// Add to button
								AddTail((struct List *)&button->function_list, (struct Node *)function);
							}
						}

						// Got a function?
						if (function)
						{
							Cfg_Instruction *ins;

							// Create instruction
							if ((ins = instruction_from_wbarg(&msg->am_ArgList[num], data->bank->memory)))
							{
								// Add to function list
								AddTail((struct List *)&function->instructions, (struct Node *)ins);
							}
						}
					}
				}

				// Did we create something?
				if (node)
				{
					// Launch editor
					lister_menu_edit_item(data);

					// Refresh name field
					lister_menu_set_name(data, node->node.ln_Name);
				}

				// Reply message
				ReplyMsg((struct Message *)msg);
			}
		}

		// Any IPC messages?
		while ((imsg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Look at command
			switch (imsg->command)
			{
			// Quit immediately
			case IPC_QUIT:
				quit_flag = 1;
				quit_msg = imsg;
				imsg = 0;
				break;

			// Hide
			case IPC_HIDE:

				if (data->window)
				{
					// Store window position
					store_window_pos(data->window, &data->window_pos);
					data->pos_valid = 1;

					// Tell children to hide
					IPC_ListCommand(&data->proc_list, IPC_HIDE, 0, 0, 0);

					// End drag (if we're dragging)
					lister_menu_end_drag(data, 0);

					// Close window
					if (data->app_window)
					{
						RemoveAppWindow(data->app_window);
						data->app_window = 0;
					}
					CloseConfigWindow(data->window);
					data->window = 0;
				}
				break;

			// Show
			case IPC_SHOW:

				// Re-open window
				if (!(lister_menu_open(data, (struct Screen *)imsg->data)))
				{
					quit_flag = 1;
				}

				// Tell children to appear
				IPC_ListCommand(&data->proc_list, IPC_SHOW, 0, (IPTR)data->window, 0);
				break;

			// Activate
			case IPC_ACTIVATE:
				if (data->window)
				{
					WindowToFront(data->window);
					ActivateWindow(data->window);
				}
				break;

			// Editor returning a function
			case FUNCTIONEDIT_RETURN:
				if (lister_menu_receive_edit(data, (FunctionReturn *)imsg->data))
					data->change = 1;
				break;

			// Process saying goodbye
			case IPC_GOODBYE: {
				IPCData *bye;
				edit_node *node;

				// Get process
				bye = (IPCData *)imsg->data;

				// See if it's a button editor
				for (node = (edit_node *)data->edit_list.lh_Head; node->node.ln_Succ;
					 node = (edit_node *)node->node.ln_Succ)
				{
					// Was it an editor?
					if (bye == node->editor)
					{
						// Remove edit node
						Remove(&node->node);
						FreeVec(node);
						break;
					}
				}

				// All processes gone and pending quit?
				if (pending_quit && (IsListEmpty(&data->proc_list.list)))
				{
					quit_flag = 1;
					pending_quit = 2;
				}
			}
			break;

			// Get a copy of a button
			case BUTTONEDIT_CLIP_BUTTON:

				// Copy button
				if ((button = CopyButton((Cfg_Button *)imsg->data, data->bank->memory, 0)))
				{
					Cfg_ButtonFunction *func;

					// End any edit
					lister_menu_end_edit(data, 0, 0);

					// No selection
					SetGadgetValue(data->objlist, GAD_LISTER_MENU_ITEMS, (ULONG)-1);

					// Detach list
					SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

					// Get first function
					func = (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list,
																  FTYPE_LEFT_BUTTON);

					// Add a new node
					if ((data->sel_item = lister_menu_new_item(data, func, button)))
					{
						// Add button to bank list
						AddTail(&data->bank->buttons, &button->node);
					}
					else
						FreeButton(button);

					// Attach list
					SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)data->menu_list);

					// Select new node
					SetGadgetValue(
						data->objlist, GAD_LISTER_MENU_ITEMS, Att_FindNodeNumber(data->menu_list, data->sel_item));

					// Fix gadgets
					lister_menu_fix_gadgets(data);

					// Set change flag
					data->change = 1;
				}

				// Free incoming button
				FreeButton((Cfg_Button *)imsg->data);
				break;
			}

			// Reply message
			IPC_Reply(imsg);

			// Check quit flag
			if (quit_flag)
				break;
		}

		if (pending_quit == 2)
			break;

		// Intuimessages
		if (data->window)
		{
			while ((msg = GetWindowMsg(data->window->UserPort)))
			{
				struct IntuiMessage msg_copy;
				UWORD gadgetid = 0;

				// Copy message and reply (unless IDCMPUPDATE)
				msg_copy = *msg;
				if (msg->Class != IDCMP_IDCMPUPDATE)
				{
					ReplyWindowMsg(msg);
					msg = 0;
				}

				// Ignore messages if a quit is pending
				if (pending_quit)
					continue;

				// Get gadget ID
				if (msg_copy.Class == IDCMP_GADGETUP || msg_copy.Class == IDCMP_GADGETDOWN)
					gadgetid = ((struct Gadget *)msg_copy.IAddress)->GadgetID;

				// Look at message
				switch (msg_copy.Class)
				{
				// Close window
				case IDCMP_CLOSEWINDOW:
					quit_flag = 1;
					undo_flag = 1;
					break;

				// BOOPSI message
				case IDCMP_IDCMPUPDATE:

					// Scripts don't support dragging
					if (data->type != TYPE_SCRIPTS)
					{
						struct TagItem *tags = (struct TagItem *)msg_copy.IAddress;

						// Lister?
						if (GetTagData(GA_ID, 0, tags) == GAD_LISTER_MENU_ITEMS)
						{
							short item;

							// Get item
							if ((item = GetTagData(DLV_DragNotify, -1, tags)) != -1)
							{
								// Start the drag
								config_drag_start(&data->drag, data->menu_list, item, tags, TRUE);
							}
						}
					}
					break;

				// Intuitick
				case IDCMP_INTUITICKS:
					++data->drag.tick_count;
					break;

				// Mouse move
				case IDCMP_MOUSEMOVE:

					// Handle drag move
					config_drag_move(&data->drag);
					break;

				// Inactive window does menu down
				case IDCMP_INACTIVEWINDOW:
					msg_copy.Code = MENUDOWN;

				// Mouse buttons
				case IDCMP_MOUSEBUTTONS:

					// Valid drag info?
					if (data->drag.drag)
					{
						short ok = -1;

						// Dropped ok?
						if (msg_copy.Code == SELECTUP)
						{
							// Remember last position
							data->drag.drag_x = data->window->WScreen->MouseX;
							data->drag.drag_y = data->window->WScreen->MouseY;
							ok = 1;
						}

						// Aborted
						else if (msg_copy.Code == MENUDOWN)
						{
							// Set abort flag
							ok = 0;
						}

						// Want to end drag?
						if (ok != -1)
							lister_menu_end_drag(data, ok);
					}
					break;

				// Menu pick
				case IDCMP_MENUPICK: {
					struct MenuItem *item;

					// Get item
					if (!(item = ItemAddress(data->window->MenuStrip, msg_copy.Code)))
						break;

					// Treat as gadget
					gadgetid = MENUID(item);
				}

				// Gadget
				case IDCMP_GADGETUP:
				case IDCMP_GADGETDOWN:

					// Look at gadget ID
					switch (gadgetid)
					{
					// Cancel
					case GAD_LISTER_MENU_CANCEL:
						undo_flag = 1;

					// Use
					case GAD_LISTER_MENU_USE:
						quit_flag = 1;
						break;

					// Save
					case GAD_LISTER_MENU_SAVE:

						// Try to save
						if (lister_menu_save(data, MENU_LISTER_MENU_SAVE))
							quit_flag = 1;
						break;

					// Menu item selected
					case GAD_LISTER_MENU_ITEMS: {
						Att_Node *node;

						// Get selection
						if (!(node = Att_FindNode(data->menu_list, msg_copy.Code)))
							break;

						// Same as last selection?
						if (node == data->sel_item)
						{
							// Double-click?
							if (DoubleClick(data->seconds, data->micros, msg_copy.Seconds, msg_copy.Micros))
							{
								// Edit item
								lister_menu_edit_item(data);
								data->seconds = 0;
								break;
							}
						}

						// Store double-click time
						data->seconds = msg_copy.Seconds;
						data->micros = msg_copy.Micros;

						// Edit this item
						if (data->sel_item != node)
						{
							lister_menu_end_edit(data, 0, 0);
							data->sel_item = node;
							lister_menu_fix_gadgets(data);
						}
					}
					break;

					// Item name
					case GAD_LISTER_MENU_NAME:
						lister_menu_end_edit(data, 2, 0);
						break;

					// New item
					case GAD_LISTER_MENU_ADD:
						if (lister_menu_add_item(data, 0))
							data->flags |= LMF_NEW_ENTRY;
						break;

					// Insert item
					case GAD_LISTER_MENU_INSERT:
						if (lister_menu_add_item(data, data->sel_item))
							data->flags |= LMF_NEW_ENTRY;
						break;

					// Duplicate item
					case GAD_LISTER_MENU_DUPLICATE:
						lister_menu_add_item(data, (Att_Node *)-1);
						break;

					// Delete item
					case GAD_LISTER_MENU_REMOVE:
						lister_menu_del_item(data, data->sel_item);
						break;

					// Edit item
					case GAD_LISTER_MENU_EDIT:
						lister_menu_edit_item(data);
						break;

					// Save
					case MENU_LISTER_MENU_SAVE:
					case MENU_LISTER_MENU_SAVEAS:
						lister_menu_save(data, gadgetid);
						break;

					// New/Open
					case MENU_LISTER_MENU_NEW:
					case MENU_LISTER_MENU_OPEN:
						if (!(lister_menu_load(data, (gadgetid == MENU_LISTER_MENU_NEW))))
						{
							quit_flag = 1;
						}
						break;

					// Restore
					case MENU_LISTER_RESTORE:

						// Got backed up bank?
						if (data->restore_bank)
						{
							// Check change
							if (!(lister_menu_check_change(data, 0)))
								break;

							// Use it
							lister_menu_load_menu(data, data->restore_bank, 0);
							lister_menu_update_bank(data);
						}
						break;

					// Last saved
					case MENU_LISTER_LAST_SAVED:
						lister_menu_load(data, -1);
						break;

					// Defaults
					case MENU_LISTER_RESET_DEFAULTS:
						lister_menu_load(data, -2);
						break;

					// System-global
					case GAD_HOTKEYS_GLOBAL:

						if (data->sel_item)
						{
							Cfg_Button *button;

							// Get button
							button = (Cfg_Button *)data->sel_item->data;

							// Set or clear flag
							if (msg_copy.Code)
								button->button.flags |= BUTNF_GLOBAL;
							else
								button->button.flags &= ~BUTNF_GLOBAL;
						}
						break;
					}
					break;

				// Vanilla key
				case IDCMP_VANILLAKEY:

					// Return pressed?
					if (msg_copy.Code == '\r')
					{
						// Edit item
						lister_menu_edit_item(data);
					}
					break;

				// Key press
				case IDCMP_RAWKEY:

					// Help?
					if (msg_copy.Code == 0x5f && !(msg_copy.Qualifier & VALID_QUALIFIERS))
					{
						// Set busy pointer
						SetWindowBusy(data->window);

						// Send help command
						IPC_Command(data->owner_ipc,
									IPC_HELP,
									(1 << 31),
									(data->type == TYPE_HOTKEYS) ? "Hotkey Editor" : "Scripts Editor",
									0,
									REPLY_NO_PORT);

						// Clear busy pointer
						ClearWindowBusy(data->window);
					}
					break;
				}

				// Reply to any outstanding message
				if (msg)
					ReplyWindowMsg(msg);

				// Check quit flag
				if (quit_flag)
					break;
			}
		}

		// Check quit flag
		if (quit_flag)
		{
			if (!pending_quit)
			{
				// Make window busy
				if (data->window)
					SetWindowBusy(data->window);

				// Cancel?
				if (undo_flag)
				{
					// Check changed
					if (!(lister_menu_check_change(data, 0)))
					{
						// Cancelled
						ClearWindowBusy(data->window);
						undo_flag = 0;
						continue;
					}
				}

				// Kill all processes
				if (!(IPC_ListQuit(&data->proc_list, 0, !undo_flag, FALSE)))
					break;
				pending_quit = 1;
			}
		}

		// Wait for an event
		Wait(1 << ipc->command_port->mp_SigBit | 1 << data->drag.timer->port->mp_SigBit |
			 ((data->app_window) ? (1 << data->app_port->mp_SigBit) : 0) |
			 ((data->window) ? (1 << data->window->UserPort->mp_SigBit) : 0));
	}

	// No undo?
	if (!undo_flag)
	{
		// Build button list
		lister_menu_build_buttons(data);
		ret_code = data->bank;

		// Copy new bank name
		strcpy(menu_name, data->menu_name);

		// Have we changed?
		if (data->change)
			data->bank->window.flags |= BTNWF_CHANGED;
	}

	// Close up
	lister_menu_close(data, (ret_code > (Cfg_ButtonBank *)1) ? 0 : 1);

	// Reply quit message
	IPC_Reply(quit_msg);

	return (IPTR)ret_code;
}

// Open window
BOOL lister_menu_open(lister_menu_data *data, struct Screen *screen)
{
	// Already open?
	if (data->window)
	{
		// Bring to front
		WindowToFront(data->window);
		return 1;
	}

	// Fill in new window
	data->newwin.parent = screen;
	data->newwin.dims = &data->dims;
	data->newwin.title = data->title;
	data->newwin.locale = locale;
	data->newwin.flags =
		WINDOW_VISITOR | WINDOW_SCREEN_PARENT | WINDOW_REQ_FILL | WINDOW_AUTO_KEYS | WINDOW_SIZE_BOTTOM;

	// Initial dimensions
	data->dims = (data->type == TYPE_HOTKEYS) ? hotkeys_window : scripts_window;

	// Reopening?
	if (data->pos_valid)
	{
		struct Screen *pub = 0;

		// Get position
		data->dims.char_dim.Left = 0;
		data->dims.char_dim.Top = 0;
		data->dims.fine_dim.Left = data->window_pos.Left;
		data->dims.fine_dim.Top = data->window_pos.Top;

		// Can get size if font is the same
		if (screen || (pub = LockPubScreen(0)))
		{
			// Get screen
			if (!screen)
				screen = pub;

			// Is font size the same?
			if (data->font_size == screen->RastPort.TxHeight)
			{
				// Get dimension
				data->dims.char_dim.Width = 0;
				data->dims.char_dim.Height = 0;
				data->dims.fine_dim.Width = data->window_pos.Width;
				data->dims.fine_dim.Height = data->window_pos.Height;
			}

			// Unlock screen
			if (pub)
				UnlockPubScreen(0, pub);
		}
	}

	// Open window and add objects
	if (!(data->window = OpenConfigWindow(&data->newwin)))
		return 0;

	// Store window position
	store_window_pos(data->window, &data->old_pos);

	// Set minimum size
	SetConfigWindowLimits(data->window, &scripts_window, 0);

	// Store font size
	data->font_size = data->window->WScreen->RastPort.TxHeight;

	// Store window pointer for dragging
	data->drag.window = data->window;

	// Add objects
	if (!(data->objlist =
			  AddObjectList(data->window, (data->type == TYPE_HOTKEYS) ? hotkeys_objects : scripts_objects)))
		return 0;

	// Stuff for non-scripts
	if (data->type != TYPE_SCRIPTS)
	{
		// Add AppWindow
		if (data->app_port)
		{
			data->app_window = AddAppWindowA(0, 0, data->window, data->app_port, 0);
		}

		// Set window ID
		SetWindowID(data->window, 0, WINDOW_BUTTON_CONFIG, (struct MsgPort *)data->ipc);
	}

	// Add menus
	AddWindowMenus(data->window, _lister_menu_menus);

	// Attach menu list
	SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)data->menu_list);
	data->sel_item = 0;

	// Fix function colours
	lister_menu_fix_functionlist(data);

	// Fix gadgets
	lister_menu_fix_gadgets(data);
	return 1;
}

// Close lister menu editor
void lister_menu_close(lister_menu_data *data, BOOL free)
{
	if (data)
	{
		// End any drag in progress
		lister_menu_end_drag(data, 0);

		// Store window position
		if (data->window)
			store_window_pos(data->window, &data->window_pos);

		// See if position has changed
		if (data->window_pos.Left != data->old_pos.Left || data->window_pos.Top != data->old_pos.Top ||
			data->window_pos.Width != data->old_pos.Width || data->window_pos.Height != data->old_pos.Height)
		{
			SavePos((data->type == TYPE_HOTKEYS) ? "dopus/windows/hotkeys" : "dopus/windows/scripts",
					&data->window_pos,
					data->font_size);
		}

		// Close window
		RemoveAppWindow(data->app_window);
		CloseConfigWindow(data->window);
		data->window = 0;

		// Free AppPort
		if (data->app_port)
		{
			struct Message *msg;
			while ((msg = GetMsg(data->app_port)))
				ReplyMsg(msg);
			DeleteMsgPort(data->app_port);
		}

		// Free button bank
		if (free)
			CloseButtonBank(data->bank);
		CloseButtonBank(data->restore_bank);

		// Free menu list
		Att_RemList(data->menu_list, 0);

		// Close timer
		FreeTimer(data->drag.timer);

		// Free data
		FreeVec(data);
	}
}

// Load menu
Cfg_ButtonBank *lister_menu_load_menu(lister_menu_data *data, Cfg_ButtonBank *bank, char *path)
{
	// Free existing menu bank
	CloseButtonBank(data->bank);
	data->bank = 0;

	// Detach list
	if (data->window)
		SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

	// Free existing menu list
	Att_RemList(data->menu_list, 0);
	data->menu_list = 0;

	// Create new list
	if (!(data->menu_list = Att_NewList(0)))
		return 0;

	// Bank supplied?
	if (bank)
		data->bank = CopyButtonBank(bank);

	// Get path
	if (!path)
		path = data->menu_name;

	// No valid bank
	if (!data->bank)
	{
		// Clear name
		data->menu_name[0] = 0;

		// Try to open bank
		if (path[0] && (data->bank = OpenButtonBank(path)))
			strcpy(data->menu_name, path);
	}

	// Check scripts bank
	if (data->type == TYPE_SCRIPTS)
		scripts_check_bank(data);

	// No valid bank?
	else if (!data->bank)
	{
		// Create a new bank
		if ((data->bank = NewButtonBank(0, 0)))
		{
			// Initialise bank
			strcpy(data->bank->window.name, (data->type == TYPE_MENU) ? "Menu" : "Hotkeys");
			data->bank->window.pos.Width = 128;
			data->bank->window.pos.Height = -1;
			data->bank->window.columns = 1;
			data->bank->window.rows = 0;
		}

		return data->bank;
	}

	// Got a valid bank?
	if (data->bank)
	{
		Cfg_Button *button;

		// Go through buttons
		for (button = (Cfg_Button *)data->bank->buttons.lh_Head; button->node.ln_Succ;
			 button = (Cfg_Button *)button->node.ln_Succ)
		{
			Cfg_ButtonFunction *func;

			// Find first function
			func = (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON);

			// Add node to list
			lister_menu_new_item(data, func, button);
		}
	}

	return data->bank;
}

// Fix gadgets
void lister_menu_fix_gadgets(lister_menu_data *data)
{
	// Fix disable state
	DisableObject(data->objlist, GAD_LISTER_MENU_DUPLICATE, (BOOL)(!data->sel_item));
	DisableObject(data->objlist, GAD_LISTER_MENU_INSERT, (BOOL)(!data->sel_item));
	DisableObject(data->objlist, GAD_LISTER_MENU_REMOVE, (BOOL)(!data->sel_item));
	DisableObject(data->objlist, GAD_LISTER_MENU_UP, (BOOL)(!data->sel_item));
	DisableObject(data->objlist, GAD_LISTER_MENU_DOWN, (BOOL)(!data->sel_item));
	DisableObject(data->objlist, GAD_LISTER_MENU_EDIT, (BOOL)(!data->sel_item));
	DisableObject(data->objlist, GAD_LISTER_MENU_NAME, (BOOL)(!data->sel_item));
	DisableObject(data->objlist, GAD_HOTKEYS_GLOBAL, (BOOL)(!data->sel_item));

	// Selected object?
	if (data->sel_item && data->type != TYPE_SCRIPTS)
	{
		// Copy name to name field
		lister_menu_set_name(data, data->sel_item->node.ln_Name);

		// Activate name field
		ActivateStrGad(GADGET(GetObject(data->objlist, GAD_LISTER_MENU_NAME)), data->window);

		// Set global state
		SetGadgetValue(
			data->objlist, GAD_HOTKEYS_GLOBAL, ((Cfg_Button *)data->sel_item->data)->button.flags & BUTNF_GLOBAL);
	}

	// Clear name to name field
	else
		SetGadgetValue(data->objlist, GAD_LISTER_MENU_NAME, 0);

	// No items in list?
	DisableObject(data->objlist, GAD_LISTER_MENU_ITEMS, (BOOL)IsListEmpty((struct List *)data->menu_list));
}

// End edit of an item
void lister_menu_end_edit(lister_menu_data *data, short disable, Att_Node *node)
{
	BOOL force = 0;

	// No node supplied?
	if (!node)
		node = data->sel_item;
	else
		force = 1;

	// Valid item?
	if (node && data->type != TYPE_SCRIPTS)
	{
		char *name, buf[336], *ptr = 0;

		// Get new name
		name = (char *)GetGadgetValue(data->objlist, GAD_LISTER_MENU_NAME);

		// Get current name
		if (node->node.ln_Name)
		{
			// Copy name
			strcpy(buf, node->node.ln_Name);

			// Find tab and strip it
			if ((ptr = strchr(buf, '\t')))
				*ptr = 0;
			ptr = buf;
		}

		// Has name changed?
		if (force || !ptr || strcmp(ptr, name))
		{
			Cfg_ButtonFunction *func;

			// Detach list
			SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

			// Copy new name
			strcpy(buf, name);

			// Hotkey?
			if (data->type == TYPE_HOTKEYS && node->data &&
				(func = (Cfg_ButtonFunction *)FindFunctionType(
					 (struct List *)&((Cfg_Button *)node->data)->function_list, FTYPE_LEFT_BUTTON)) &&
				func->function.code != 0xffff)
			{
				char key[80];

				// Build key string
				BuildKeyString(func->function.code,
							   func->function.qual,
							   func->function.qual_mask,
							   func->function.qual_same,
							   key + 1);

				// String starts with a tab
				key[0] = '\t';

				// Add key string to label
				strcat(buf, key);
			}

			// Store new name
			Att_ChangeNodeName(node, buf);

			// Reattach list
			SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)data->menu_list);

			// Set change flag
			data->change = 1;
		}
	}

	// Disable gadgets?
	if (disable)
	{
		// Edit new entry?
		if (data->flags & LMF_NEW_ENTRY && disable == 2)
		{
			// Edit this item
			lister_menu_edit_item(data);
		}

		// Otherwise
		else
		{
			// No selection
			data->sel_item = 0;
			lister_menu_fix_gadgets(data);

			// No lister selection
			SetGadgetValue(data->objlist, GAD_LISTER_MENU_ITEMS, (ULONG)-1);
		}
	}

	// Clear "new entry" flag
	data->flags &= ~LMF_NEW_ENTRY;
}

// Edit an item
void lister_menu_edit_item(lister_menu_data *data)
{
	Cfg_Button *button;
	Cfg_Function *function;
	FunctionStartup *startup;
	BOOL success = 0;
	edit_node *node;
	IPCData *ipc;

	// Get selected item
	if (!data->sel_item || !(button = (Cfg_Button *)data->sel_item->data))
		return;

	// See if this button is already being edited
	for (node = (edit_node *)data->edit_list.lh_Head; node->node.ln_Succ; node = (edit_node *)node->node.ln_Succ)
	{
		// Is it in the list?
		if (node->button == button)
		{
			// Tell it to activate
			IPC_Command(node->editor, IPC_ACTIVATE, 0, 0, 0, 0);
			return;
		}
	}

	// Allocate startup data
	if (!(startup = AllocVec(sizeof(FunctionStartup), MEMF_CLEAR)))
		return;

	// Copy function from button if it exists
	if ((function = FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON)))
		function = CopyFunction(function, 0, 0);

	// Otherwise, allocate a new function
	else
		function = NewFunction(0, 0);

	// Valid function?
	if (function)
	{
		// Initialise startup data
		function_editor_init(startup, data->command_list);
		startup->window = data->window;
		startup->function = function;
		startup->owner_ipc = data->ipc;
		startup->main_owner = data->owner_ipc;
		startup->object = button;

		// Launch editor
		if ((IPC_Launch(&data->proc_list,
						&ipc,
						"dopus_function_editor",
						IPC_NATIVE(FunctionEditor),
						STACK_DEFAULT,
						(IPTR)startup,
						(struct Library *)DOSBase)) &&
			ipc)
		{
			success = 1;

			// Add to edit list
			if ((node = AllocVec(sizeof(edit_node), MEMF_CLEAR)))
			{
				node->button = button;
				node->editor = ipc;
				AddTail(&data->edit_list, &node->node);
			}
		}
	}

	// Free data if not successful
	if (!success)
	{
		FreeVec(startup);
		FreeFunction(function);
	}
}

// Receive an edited function
BOOL lister_menu_receive_edit(lister_menu_data *data, FunctionReturn *ret)
{
	Cfg_Button *button;
	Att_Node *node;
	BOOL success = 0;

	// Try to find button in list
	if ((node = Att_FindNodeData(data->menu_list, (IPTR)ret->object)) && (button = (Cfg_Button *)node->data))
	{
		Cfg_ButtonFunction *func;

		// Find old function
		if ((func = (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON)))
		{
			// Free old function
			FreeInstructionList((Cfg_Function *)func);
		}

		// Allocate new one
		else if (!(func = NewButtonFunction(0, FTYPE_LEFT_BUTTON)))
			return 0;

		// Add to list
		else
			AddTail((struct List *)&button->function_list, (struct Node *)func);

		// Copy new function in
		CopyFunction(ret->function, 0, (Cfg_Function *)func);

		// Update list for hotkeys
		if (data->type == TYPE_HOTKEYS)
			lister_menu_end_edit(data, 0, node);

		// Fix function colours
		lister_menu_fix_functionlist(data);
		success = 1;
	}

	return success;
}

// Save menu
BOOL lister_menu_save(lister_menu_data *data, unsigned short gadgetid)
{
	short ok = 1, loop;
	char path[256];
	BOOL ret = 0;

	// Make window busy
	SetWindowBusy(data->window);

	// Copy path
	strcpy(path, data->menu_name);

	// Loop until successful
	do
	{
		// Clear loop flag
		loop = 0;

		// Need to ask for a file?
		if (gadgetid == MENU_LISTER_MENU_SAVEAS || !path[0])
		{
			char *ptr = 0;
			char file[40];

			// Invalid path?
			if (!path[0])
				strcpy(path, "DOpus5:Buttons/");

			// Get filename
			else if ((ptr = FilePart(path)))
			{
				strcpy(file, ptr);
				*ptr = 0;
				ptr = file;
			}

			// Display file requester
			if ((ok = AslRequestTags(
					 DATA(data->window)->request,
					 ASLFR_Window,
					 data->window,
					 ASLFR_TitleText,
					 GetString(locale, (data->type == TYPE_HOTKEYS) ? MSG_HOTKEYS_ENTER_NAME : MSG_SCRIPTS_ENTER_NAME),
					 ASLFR_InitialFile,
					 (ptr) ? ptr : "",
					 ASLFR_InitialDrawer,
					 path,
					 ASLFR_Flags1,
					 FRF_PRIVATEIDCMP | FRF_DOSAVEMODE,
					 TAG_END)))
			{
				// Get new path
				strcpy(path, DATA(data->window)->request->fr_Drawer);
				AddPart(path, DATA(data->window)->request->fr_File, 256);
			}
		}

		// Ok to save?
		if (ok)
		{
			short err;

			// Build button list
			lister_menu_build_buttons(data);

			// Save buttons
			if (!(err = SaveButtonBank(data->bank, path)))
			{
				// Store button path
				strcpy(data->menu_name, path);
				strcpy(data->last_save_name, path);

				// Clear change flag
				data->change = 0;

				// Set return change flag
				data->ret_change = 1;
				ret = 1;
			}

			// Error?
			else
			{
				// Show error
				loop = error_saving(err, data->window);
			}
		}
	} while (loop);

	// Make window unbusy
	ClearWindowBusy(data->window);

	return ret;
}

// Check if current menu has changed
short lister_menu_check_change(lister_menu_data *data, BOOL can_save)
{
	short ret;
	char *name;

	// If not changed, ok
	if (!data->change || !data->window)
		return 1;

	// Get name
	if (!(name = FilePart(data->menu_name)) || !name[0])
		name = GetString(locale, MSG_UNTITLED);

	// Display requester
	if (!(ret = SimpleRequestTags(
			  data->window,
			  0,
			  GetString(locale, (can_save) ? MSG_SAVE_DISCARD_CANCEL : MSG_DISCARD_CANCEL),
			  GetString(locale,
						(can_save)
							? ((data->type == TYPE_HOTKEYS) ? MSG_HOTKEYS_CHANGED : MSG_SCRIPTS_CHANGED)
							: ((data->type == TYPE_HOTKEYS) ? MSG_HOTKEYS_CHANGED_NOSAVE : MSG_SCRIPTS_CHANGED_NOSAVE)),
			  name)))
		return 0;

	// Clear change flag
	data->change = 0;

	// Save?
	if (can_save && ret == 1)
		lister_menu_save(data, MENU_LISTER_MENU_SAVE);
	return 1;
}

// Load a new menu
BOOL lister_menu_load(lister_menu_data *data, short new)
{
	char path[256], file[32], *ptr;

	// Check if current menu has changed
	if (!(lister_menu_check_change(data, (new < 0) ? 0 : 1)))
		return 1;

	// Load old?
	if (new == -1)
	{
		// Get old name
		strcpy(path, data->last_save_name);
	}

	// Load defaults
	else if (new == -2)
	{
		// Default name
		strcpy(path, "DOpus5:Buttons/");
		strcat(path, data->default_name);
		strcat(path, "_default");
	}

	// Load new menu?
	else if (!new)
	{
		BOOL ok;

		// Make window busy
		SetWindowBusy(data->window);

		// Copy path
		strcpy(path, data->menu_name);

		// Get file
		if ((ptr = PathPart(path)) && *ptr)
		{
			strcpy(file, ptr + 1);
			if (*ptr == '/' || *ptr == ':')
				*ptr = 0;
		}
		else
			file[0] = 0;

		// Invalid path?
		if (!path[0])
			strcpy(path, "DOpus5:Buttons/");

		// Display file requester
		if ((ok = AslRequestTags(
				 DATA(data->window)->request,
				 ASLFR_Window,
				 data->window,
				 ASLFR_TitleText,
				 GetString(locale, (data->type == TYPE_HOTKEYS) ? MSG_HOTKEYS_SELECT_FILE : MSG_SCRIPTS_SELECT_FILE),
				 ASLFR_InitialFile,
				 file,
				 ASLFR_InitialDrawer,
				 path,
				 ASLFR_Flags1,
				 FRF_PRIVATEIDCMP,
				 TAG_END)))
		{
			// Get new path
			strcpy(path, DATA(data->window)->request->fr_Drawer);
			AddPart(path, DATA(data->window)->request->fr_File, 256);
		}

		// Make window unbusy
		ClearWindowBusy(data->window);

		// Cancelled?
		if (!ok)
			return 1;
	}

	// New bank
	else
		path[0] = 0;

	// Detach list
	SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

	// Store new bank name
	if (new != -2)
		strcpy(data->menu_name, path);

	// Load new bank
	if (!(lister_menu_load_menu(data, 0, path)))
		return 0;

	// Update bank
	lister_menu_update_bank(data);
	return 1;
}

// Update bank display
void lister_menu_update_bank(lister_menu_data *data)
{
	// Fix gadgets
	SetGadgetValue(data->objlist, GAD_LISTER_MENU_ITEMS, (ULONG)-1);
	data->sel_item = 0;
	lister_menu_fix_gadgets(data);

	// Fix function list colours
	if (data->type == TYPE_SCRIPTS)
		lister_menu_fix_functionlist(data);

	// Attach list
	else
		SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)data->menu_list);

	// Clear change flag
	data->change = 0;
}

// End drag
void lister_menu_end_drag(lister_menu_data *data, BOOL ok)
{
	struct Window *window;
	ULONG id = 0;
	IPCData *ipc = 0;
	short item;

	// Not dragging something?
	if (!data->drag.drag)
		return;

	// End drag
	if (!(window = config_drag_end(&data->drag, ok)))
		return;
	ok = 0;

	// Get drag item
	item = Att_FindNodeNumber(data->menu_list, data->drag.drag_node);

	// Not dropped on our own window?
	if (window != data->window)
	{
		// Get window ID
		id = GetWindowID(window);

		// Forbid to get IPC
		Forbid();
		if (!(ipc = (IPCData *)GetWindowAppPort(window)))
			Permit();
	}

	// Unlock layer
	UnlockLayerInfo(&data->window->WScreen->LayerInfo);

	// Dropped on our window?
	if (window == data->window)
	{
		short sel;

		// Hotkeys mode doesn't support changing position
		if (data->type != TYPE_HOTKEYS)
		{
			// Get selection
			sel = functioned_get_line(data->window,
									  GetObject(data->objlist, GAD_LISTER_MENU_ITEMS),
									  data->drag.drag_x,
									  data->drag.drag_y,
									  (struct Library *)IntuitionBase);

			// Valid selection?
			if (sel != item)
			{
				Att_Node *before;

				// Stop editing
				lister_menu_end_edit(data, 1, 0);

				// Detach existing list
				SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

				// Remove it
				Remove((struct Node *)data->drag.drag_node);

				// Get "before" node
				if ((before = Att_FindNode(data->menu_list, sel)))
				{
					// Position node
					Insert((struct List *)data->menu_list, (struct Node *)data->drag.drag_node, before->node.ln_Pred);
				}

				// Add after
				else
					AddTail((struct List *)data->menu_list, (struct Node *)data->drag.drag_node);

				// Attach list to gadget
				SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, data->menu_list);
			}
		}

		ok = 1;
	}

	// Got an IPC port?
	else if (ipc)
	{
		// Button editor or a button bank?
		if (id == WINDOW_BUTTON_CONFIG || id == WINDOW_BUTTONS || id == WINDOW_FUNCTION_EDITOR)
		{
			Cfg_Button *button;
			Point *pos;

			// Store position (screen relative)
			if ((pos = AllocVec(sizeof(Point), 0)))
			{
				pos->x = data->drag.drag_x;
				pos->y = data->drag.drag_y;
			}

			// Copy button
			if ((button = CopyButton((Cfg_Button *)data->drag.drag_node->data, 0, 0)))
			{
				// Send button
				IPC_Command(ipc, BUTTONEDIT_CLIP_BUTTON, 0, button, pos, 0);
				ok = 1;
			}
			else
				FreeVec(pos);
		}

		// Permit now we've sent the message
		Permit();
	}

	// Failed?
	if (!ok)
		DisplayBeep(data->window->WScreen);
}

// Add an item
Att_Node *lister_menu_add_item(lister_menu_data *data, Att_Node *insert)
{
	Cfg_Button *button;
	Cfg_ButtonFunction *func = 0;

	// Duplicate?
	if (insert == (Att_Node *)-1)
	{
		// Copy this button
		if (!(button = CopyButton((Cfg_Button *)data->sel_item->data, data->bank->memory, 0)))
			return 0;

		// Find first function
		func = (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON);
		insert = 0;
	}

	// Create a new button
	else if (!(button = NewButton(data->bank->memory)))
		return 0;

	// End any edit
	lister_menu_end_edit(data, 1, 0);

	// Detach list
	SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

	// Add a new node
	if ((data->sel_item = lister_menu_new_item(data, func, button)))
	{
		// Insert?
		if (insert)
		{
			// Reposition item
			Att_PosNode(data->menu_list, data->sel_item, insert);
		}

		// Add button to bank list
		AddTail(&data->bank->buttons, &button->node);
	}
	else
		FreeButton(button);

	// Attach list
	SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)data->menu_list);

	// Fix gadgets
	lister_menu_fix_gadgets(data);

	// Select new node
	SetGadgetValue(data->objlist, GAD_LISTER_MENU_ITEMS, Att_FindNodeNumber(data->menu_list, data->sel_item));

	// Set change flag
	data->change = 1;

	return data->sel_item;
}

// Build button list
void lister_menu_build_buttons(lister_menu_data *data)
{
	Att_Node *node;

	// Initialise button list
	NewList(&data->bank->buttons);

	// Go through entries
	for (node = (Att_Node *)data->menu_list->list.lh_Head, data->bank->window.rows = 0; node->node.ln_Succ;
		 node = (Att_Node *)node->node.ln_Succ, ++data->bank->window.rows)
	{
		Cfg_Button *button = (Cfg_Button *)node->data;
		Cfg_ButtonFunction *func;

		// Invalid button?
		if (!button)
			continue;

		// Find first function
		if (!(func = (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON)))
		{
			// Don't add empty scripts
			if (data->type == TYPE_SCRIPTS)
				continue;

			// Create a new function
			if ((func = NewButtonFunction(data->bank->memory, FTYPE_LEFT_BUTTON)))
				AddTail((struct List *)&button->function_list, (struct Node *)func);
			else
				continue;
		}

		// Don't add empty scripts
		else if (data->type == TYPE_SCRIPTS && IsListEmpty((struct List *)&func->instructions))
			continue;

		// Add this button to the button list
		AddTail(&data->bank->buttons, (struct Node *)button);

		// For hotkeys, we have to take tabs out of names
		if (func && data->type != TYPE_SCRIPTS)
		{
			// Free existing label and name
			FreeMemH(func->label);
			FreeMemH(func->node.ln_Name);
			func->label = 0;
			func->node.ln_Name = 0;

			// Copy new name
			if (node->node.ln_Name)
			{
				char *ptr;
				short len;

				// Find tab in name and strip it
				if ((ptr = strchr(node->node.ln_Name, '\t')))
					*ptr = 0;

				// Get length
				len = strlen(node->node.ln_Name) + 1;

				// Allocate name
				if ((func->node.ln_Name = AllocMemH(data->bank->memory, len)))
					strcpy(func->node.ln_Name, node->node.ln_Name);

				// Allocate label
				if ((func->label = AllocMemH(data->bank->memory, len)))
					strcpy(func->label, node->node.ln_Name);

				// Restore tab
				if (ptr)
					*ptr = '\t';
			}
		}
	}
}

// Add item to list
Att_Node *lister_menu_new_item(lister_menu_data *data, Cfg_ButtonFunction *func, Cfg_Button *button)
{
	char label[336];

	// Valid function?
	if (func)
	{
		char *ptr;

		// Use name for scripts
		if (data->type == TYPE_SCRIPTS && func->node.ln_Name)
			strcpy(label, func->node.ln_Name);

		// Copy label
		else if ((ptr = function_label(func)))
			strcpy(label, ptr);
		else
			label[0] = 0;

		// Hotkey?
		if (data->type == TYPE_HOTKEYS && func->function.code != 0xffff)
		{
			char key[80];

			// Build key string
			BuildKeyString(
				func->function.code, func->function.qual, func->function.qual_mask, func->function.qual_same, key + 1);

			// String starts with a tab
			key[0] = '\t';

			// Add key string to label
			strcat(label, key);
		}
	}

	// Create node
	return Att_NewNode(
		data->menu_list, (func) ? label : 0, (IPTR)button, (data->type == TYPE_HOTKEYS) ? 0 : ADDNODE_SORT);
}

// Fill out name field
void lister_menu_set_name(lister_menu_data *data, char *name)
{
	char buf[256], *ptr;

	// Valid name?
	if (name)
	{
		// Copy name
		strcpy(buf, name);

		// Find tab and clear it
		if ((ptr = strchr(buf, '\t')))
			*ptr = 0;
		name = buf;
	}

	// Set gadget
	SetGadgetValue(data->objlist, GAD_LISTER_MENU_NAME, (IPTR)name);
}

/* label   = english name
   ln_Name = localised name */

// Check bank for scripts
void scripts_check_bank(lister_menu_data *data)
{
	Att_Node *node;
	short a;
	struct DOpusLocale english;

	// Copy locale and clear catalog pointer
	english = *locale;
	english.li_Catalog = 0;

	// Create bank if we don't have one
	if (!data->bank)
	{
		// Create bank
		if (!(data->bank = NewButtonBank(0, 0)))
			return;

		// Initialise bank
		strcpy(data->bank->window.name, "Scripts");
		data->bank->window.pos.Width = 128;
		data->bank->window.pos.Height = -1;
		data->bank->window.columns = 1;
		data->bank->window.rows = 0;
	}

	// Add script entries
	for (a = MSG_SCRIPTS_STARTUP, node = (Att_Node *)data->script_list->list.lh_Head; node->node.ln_Succ;
		 node = (Att_Node *)node->node.ln_Succ, a++)
	{
		BOOL ok = 1;
		Cfg_ButtonFunction *func = 0;
		char *english_name, *locale_name;
		Cfg_Button *button;

		// Get name (in English)
		english_name = (a < MSG_SCRIPTS_LAST) ? (char *)GetString(&english, a) : node->node.ln_Name;

		// Different locale?
		if (locale->li_Catalog && a < MSG_SCRIPTS_LAST)
			locale_name = GetString(locale, a);
		else
			locale_name = english_name;

		// See if button already exists
		for (button = (Cfg_Button *)data->bank->buttons.lh_Head; button->node.ln_Succ;
			 button = (Cfg_Button *)button->node.ln_Succ)
		{
			// Get first function
			if ((func =
					 (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON)))
			{
				// String compare
				if (func->label && stricmp(func->label, english_name) == 0)
				{
					// Already exists
					ok = 0;
					break;
				}
			}
		}

		// If button doesn't exist, create it
		if (ok)
		{
			// Create button
			if ((button = NewButtonWithFunc(data->bank->memory, english_name, FTYPE_LEFT_BUTTON)))
			{
				// Add button to bank
				AddTail(&data->bank->buttons, &button->node);
				++data->bank->window.rows;

				// Get function pointer
				func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;
			}
			else
				func = 0;
		}

		// See if function name needs updating
		if (func && (!func->node.ln_Name || stricmp(func->node.ln_Name, locale_name) != 0))
		{
			// Free current name
			FreeMemH(func->node.ln_Name);

			// Allocate and copy locale name
			if ((func->node.ln_Name = AllocMemH(data->bank->memory, strlen(locale_name) + 1)))
				strcpy(func->node.ln_Name, locale_name);
		}
	}

	// Default name
	strcpy(data->menu_name, "DOpus5:Buttons/Scripts");
}

// Fix colours in function list
void lister_menu_fix_functionlist(lister_menu_data *data)
{
	Att_Node *node;

	// Don't do anything except for scripts
	if (data->type != TYPE_SCRIPTS || !data->window)
		return;

	// Detach function list
	SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

	// Go through function list
	for (node = (Att_Node *)data->menu_list->list.lh_Head; node->node.ln_Succ; node = (Att_Node *)node->node.ln_Succ)
	{
		Cfg_Button *button;

		// Reset colour
		node->node.lve_Flags &= ~LVEF_USE_PEN;

		// Get button pointer
		if ((button = ((Cfg_Button *)node->data)))
		{
			Cfg_Function *func;

			// Find function
			if ((func = FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON)))
			{
				// Not empty
				if (!(IsListEmpty((struct List *)&func->instructions)))
				{
					// Set colour
					node->node.lve_Flags |= LVEF_USE_PEN;
					node->node.lve_Pen = DRAWINFO(data->window)->dri_Pens[HIGHLIGHTTEXTPEN];
				}
			}
		}
	}

	// Attach function list
	SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, data->menu_list);
}

// Delete item
void lister_menu_del_item(lister_menu_data *data, Att_Node *item)
{
	Cfg_Button *button;
	edit_node *node;

	// Valid selection?
	if (!item)
		return;

	// Get button from selection
	if ((button = (Cfg_Button *)item->data))
	{
		// See if this button is being edited
		for (node = (edit_node *)data->edit_list.lh_Head; node->node.ln_Succ; node = (edit_node *)node->node.ln_Succ)
		{
			// Is it in the list?
			if (node->button == button)
			{
				// Tell it to quit
				IPC_Command(node->editor, IPC_QUIT, 0, 0, 0, 0);
				break;
			}
		}

		// Scripts?
		if (data->type == TYPE_SCRIPTS)
		{
			Cfg_ButtonFunction *func;

			// Don't delete the button, just clear it out
			if ((func = (Cfg_ButtonFunction *)button->function_list.mlh_Head) && func->node.ln_Succ)
			{
				// Free the function
				FreeInstructionList((Cfg_Function *)func);
			}

			// Set change flag
			data->change = 1;

			// Fix function list colours
			lister_menu_fix_functionlist(data);
			return;
		}

		// Free button
		if (button->node.ln_Succ)
			Remove(&button->node);
		FreeButton(button);
	}

	// If not scripts, remove from list
	if (data->type != TYPE_SCRIPTS)
	{
		// Detach list
		SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)~0);

		// Remove selection
		Att_RemNode(item);

		// Attach list
		SetGadgetChoices(data->objlist, GAD_LISTER_MENU_ITEMS, (APTR)data->menu_list);

		// No lister selection
		SetGadgetValue(data->objlist, GAD_LISTER_MENU_ITEMS, (ULONG)-1);

		// Fix gadgets
		data->sel_item = 0;
		lister_menu_fix_gadgets(data);
	}

	// Set change flag
	data->change = 1;
}
