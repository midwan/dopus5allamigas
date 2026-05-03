#include "config_lib.h"
#include "config_menus.h"
#include "config_buttons.h"

ULONG LIBFUNC L_Config_Menus(REG(a0, IPCData *ipc),
							 REG(a1, IPCData *owner_ipc),
							 REG(a2, struct Screen *screen),
							 REG(a3, Cfg_ButtonBank *def_bank),
							 REG(d0, ULONG command_list),
							 REG(d1, ULONG type),
							 REG(d2, char *menu_path))
{
	config_menus_data *data;
	IPCMessage *quit_msg = 0;
	short undo_flag = 0, pending_quit = 0, save_flag = 0;
	ULONG ret_code = 0;

	// Allocate data
	if (!(data = AllocVec(sizeof(config_menus_data), MEMF_CLEAR)) || !(data->drag.timer = AllocTimer(UNIT_VBLANK, 0)))
	{
		FreeVec(data);
		return 0;
	}

	// Initialise data
	data->ipc = ipc;
	data->owner_ipc = owner_ipc;
	data->command_list = command_list;
	data->type = type;
	InitListLock(&data->proc_list, 0);
	NewList(&data->edit_list);

	// For menu keys, second list is always active
	if (data->type == MENUTYPE_KEYS)
		data->active = 1;

	// Load button bank, open window
	if (!(config_menus_load_bank(data, def_bank, 0)) || !(config_menus_open(data, screen)))
	{
		// Failed
		config_menus_close(data);
		return 0;
	}

	// Backup bank
	data->restore_bank = CopyButtonBank(data->bank);

	// Get path
	strcpy(data->last_saved, data->bank->path);

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
			config_menus_end_drag(data, 0, 0);
		}

		// AppWindow open?
		if (data->app_window)
		{
			struct AppMessage *amsg;

			// Any AppMessages?
			while ((amsg = (struct AppMessage *)GetMsg(data->app_port)))
			{
				// Activate our window
				ActivateWindow(data->window);

				// Handle AppMessage
				config_menus_appmsg(data, amsg);

				// Reply the message
				ReplyMsg((struct Message *)amsg);
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

				// Window is open?
				if (data->window)
				{
					// Store window position
					data->window_pos = *((Point *)&data->window->LeftEdge);
					data->pos_valid = 1;

					// Tell children to hide
					IPC_ListCommand(&data->proc_list, IPC_HIDE, 0, 0, 0);

					// End drag
					config_menus_end_drag(data, 0, 0);

					// Close window
					CloseConfigWindow(data->window);
					data->window = 0;
				}
				break;

			// Show
			case IPC_SHOW:

				// Re-open window
				if (!(config_menus_open(data, (struct Screen *)imsg->data)))
				{
					quit_flag = 1;
				}

				// Tell children to appear
				else
					IPC_ListCommand(&data->proc_list, IPC_SHOW, 0, (ULONG)data->window, 0);
				break;

			// Activate
			case IPC_ACTIVATE:

				// If window is open, bring to the front
				if (data->window)
				{
					WindowToFront(data->window);
					ActivateWindow(data->window);
				}
				break;

			// Process saying goodbye
			case IPC_GOODBYE:

				// Handle function editor goodbye
				config_menus_editor_goodbye(data, (IPCData *)imsg->data);

				// All processes gone and pending quit?
				if (pending_quit && (IsListEmpty(&data->proc_list.list)))
				{
					quit_flag = 1;
					pending_quit = 2;
				}
				break;

			// Editor returning a function
			case FUNCTIONEDIT_RETURN:

				// Receive the function
				config_menus_receive_edit(data, (FunctionReturn *)imsg->data);
				break;

			// Get a copy of a button
			case BUTTONEDIT_CLIP_BUTTON:

				// Receive the button
				config_menus_clip_button(data, (Cfg_Button *)imsg->data, (Point *)imsg->data_free);

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

				// Intuitick
				case IDCMP_INTUITICKS:
					++data->drag.tick_count;
					break;

				// Mouse move
				case IDCMP_MOUSEMOVE:

					// Handle drag move
					config_drag_move(&data->drag);
					break;

				// Mouse buttons
				case IDCMP_MOUSEBUTTONS:

					// Dragging something?
					if (data->drag.drag)
					{
						short ok = -1;

						// Dropped?
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
							config_menus_end_drag(data, ok, msg_copy.Qualifier);
					}
					break;

				// Menu
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
					case GAD_MENUS_CANCEL:
					case GAD_LISTER_MENU_CANCEL:
						undo_flag = 1;

					// Use
					case GAD_MENUS_USE:
						quit_flag = 1;
						break;

					// Save
					case GAD_MENUS_SAVE:
						quit_flag = 1;
						save_flag = 1;
						break;

					// Menu selected
					case GAD_MENUS_MENU:
					case GAD_MENUS_ITEM:
					case GAD_MENUS_SUB:

						// Activate this list
						config_menus_active(data, gadgetid - GAD_MENUS_MENU);

						// Select it
						config_menus_select_item(data, gadgetid - GAD_MENUS_MENU, 1);
						break;

					// Add a new menu
					case GAD_MENUS_MENU_ADD:

						// Activate this list
						config_menus_active(data, MENU_MENU);

						// Add it
						config_menus_add_menu(data, FALSE, msg_copy.Qualifier);
						break;

					// Add a new item or sub-item
					case GAD_MENUS_ITEM_ADD:
					case GAD_MENUS_SUB_ADD:

						// Activate this list
						config_menus_active(data, gadgetid - GAD_MENUS_MENU_ADD);

						// Add it
						config_menus_add_item(data, gadgetid - GAD_MENUS_MENU_ADD, FALSE, msg_copy.Qualifier, 0);
						break;

					// Delete a menu
					case GAD_MENUS_MENU_DEL:

						// Go busy
						SetWindowBusy(data->window);

						// Kill it
						config_menus_del_menu(data);

						// Go unbusy
						ClearWindowBusy(data->window);
						break;

					// Delete an item
					case GAD_MENUS_ITEM_DEL:
					case GAD_MENUS_SUB_DEL:

						// Nuke it
						config_menus_del_item(data, gadgetid - GAD_MENUS_MENU_DEL);
						break;

					// Copy a whole menu
					case GAD_MENUS_MENU_COPY:

						// Go busy
						SetWindowBusy(data->window);

						// Activate this list
						config_menus_active(data, MENU_MENU);

						// Add it
						config_menus_add_menu(data, TRUE, msg_copy.Qualifier);

						// Go unbusy
						ClearWindowBusy(data->window);
						break;

					// Copy an item
					case GAD_MENUS_ITEM_COPY:
					case GAD_MENUS_SUB_COPY:

						// Activate this list
						config_menus_active(data, gadgetid - GAD_MENUS_MENU_COPY);

						// Copy it
						config_menus_add_item(data, gadgetid - GAD_MENUS_MENU_COPY, TRUE, msg_copy.Qualifier, 0);
						break;

					// Change name
					case GAD_MENUS_MENU_NAME:
					case GAD_MENUS_ITEM_NAME:
					case GAD_MENUS_SUB_NAME:

						// Get new name
						config_menus_new_name(data, gadgetid - GAD_MENUS_MENU_NAME);
						break;

					// Edit
					case GAD_MENUS_MENU_EDIT:
					case GAD_MENUS_ITEM_EDIT:
					case GAD_MENUS_SUB_EDIT:

						// Activate this list
						config_menus_active(data, gadgetid - GAD_MENUS_MENU_EDIT);

						// Edit selection
						config_menus_edit_item(data, gadgetid - GAD_MENUS_MENU_EDIT);
						break;

					// Move up
					case GAD_MENUS_MENU_UP:
					case GAD_MENUS_ITEM_UP:
					case GAD_MENUS_SUB_UP:

						// Move item
						config_menus_move(
							data, gadgetid - GAD_MENUS_MENU_UP, CURSORUP, IEQUALIFIER_LSHIFT | msg_copy.Qualifier);
						break;

					// Move down
					case GAD_MENUS_MENU_DOWN:
					case GAD_MENUS_ITEM_DOWN:
					case GAD_MENUS_SUB_DOWN:

						// Move item
						config_menus_move(
							data, gadgetid - GAD_MENUS_MENU_DOWN, CURSORDOWN, IEQUALIFIER_LSHIFT | msg_copy.Qualifier);
						break;

					// New/Open/Default/Last Saved
					case MENU_LISTER_MENU_NEW:
					case MENU_LISTER_MENU_OPEN:
					case MENU_LISTER_RESET_DEFAULTS:
					case MENU_LISTER_LAST_SAVED:

						// Open new menu
						config_menus_load(data, gadgetid);
						break;

					// Restore
					case MENU_LISTER_RESTORE:

						// Restore menu
						config_menus_restore(data);
						break;

					// Save
					case MENU_LISTER_MENU_SAVE:
					case MENU_LISTER_MENU_SAVEAS:

						// Save menu
						config_menus_save(data, gadgetid - MENU_LISTER_MENU_SAVE);
						break;
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
						IPC_Command(data->owner_ipc, IPC_HELP, (1 << 31), "Menu Editor", 0, REPLY_NO_PORT);

						// Clear busy pointer
						ClearWindowBusy(data->window);
						break;
					}

					// Cursor up/down
					else if (msg_copy.Code == CURSORUP || msg_copy.Code == CURSORDOWN)
					{
						// Move item
						config_menus_move(data, data->active, msg_copy.Code, msg_copy.Qualifier);
						break;
					}

					// Cursor left/right
					else if (msg_copy.Code == CURSORLEFT || msg_copy.Code == CURSORRIGHT)
					{
						// Left becomes shift-tab
						if (msg_copy.Code == CURSORLEFT)
							msg_copy.Qualifier = IEQUALIFIER_LSHIFT;
						else
							msg_copy.Qualifier = 0;

						// Set code to tab
						msg_copy.Code = 0x42;
					}

				case IDCMP_VANILLAKEY:

					// No keys for menu keys
					if (data->type == MENUTYPE_KEYS)
						break;

					// Tab?
					if ((msg_copy.Class == IDCMP_RAWKEY && msg_copy.Code == 0x42) ||
						(msg_copy.Class == IDCMP_VANILLAKEY && msg_copy.Code == '\t'))
					{
						// Increment active
						if (msg_copy.Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT))
							--data->active;
						else
							++data->active;

						// Check wrap-around
						if (data->active < 0)
							data->active = 2;
						else if (data->active > 2)
							data->active = 0;

						// Show active lister
						config_menus_active(data, data->active);
						break;
					}

					// Vanilla from now on
					if (msg_copy.Class != IDCMP_VANILLAKEY)
						break;

					// Enter?
					else if (msg_copy.Code == '\r')
					{
						// Edit selection
						config_menus_edit_item(data, data->active);
					}

					// Space activates name field
					else if (msg_copy.Code == ' ')
					{
						// Activate name field
						ActivateStrGad(GADGET(GetObject(data->objlist, GAD_MENUS_MENU_NAME + data->active)),
									   data->window);
					}

					// Delete does delete
					else if (msg_copy.Code == 0x7f)
					{
						// Check list isn't disabled
						if (GADGET(GetObject(data->objlist, GAD_MENUS_MENU_DEL + data->active))->Flags & GFLG_DISABLED)
							break;

						// Menu?
						if (data->active == MENU_MENU)
						{
							// Go busy
							SetWindowBusy(data->window);

							// Kill menu
							config_menus_del_menu(data);

							// Go unbusy
							ClearWindowBusy(data->window);
						}

						// Nuke the item
						else
							config_menus_del_item(data, data->active);
						break;
					}

					// Plus does add
					else if (msg_copy.Code == '+' || msg_copy.Code == '=')
					{
						// Check list isn't disabled
						if (GADGET(GetObject(data->objlist, GAD_MENUS_MENU_ADD + data->active))->Flags & GFLG_DISABLED)
							break;

						// Add menu?
						if (data->active == MENU_MENU)
							config_menus_add_menu(data, FALSE, msg_copy.Qualifier);

						// Add item
						else
							config_menus_add_item(data, data->active, FALSE, msg_copy.Qualifier, 0);
					}
					break;

				// BOOPSI message
				case IDCMP_IDCMPUPDATE:

					// Do nothing for menu keys
					if (data->type != MENUTYPE_KEYS)
					{
						struct TagItem *tags = (struct TagItem *)msg_copy.IAddress;
						short id, item;

						// Get ID
						id = GetTagData(GA_ID, 0, tags);

						// Drag?
						if ((item = GetTagData(DLV_DragNotify, -1, tags)) != -1)
						{
							// Start the drag
							config_drag_start(&data->drag, data->menu_list[id - GAD_MENUS_MENU], item, tags, TRUE);
							break;
						}

						// Look at gadget ID
						switch (id)
						{
						// Select something?
						case GAD_MENUS_MENU:
						case GAD_MENUS_ITEM:
						case GAD_MENUS_SUB:

							// Double-click?
							if (GetTagData(DLV_DoubleClick, 0, tags))
							{
								// Edit it
								config_menus_edit_item(data, id - GAD_MENUS_MENU);
							}
							break;
						}
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
			// No quit pending?
			if (!pending_quit)
			{
				// Cancel?
				if (undo_flag)
				{
					// Check changed
					if (!(config_menus_check_change(data, FALSE)))
					{
						// Cancelled
						undo_flag = 0;
						continue;
					}
				}

				// Make window busy
				SetWindowBusy(data->window);

				// Kill all processes
				if (!(IPC_ListQuit(&data->proc_list, 0, !undo_flag, FALSE)))
					pending_quit = 2;
				else
					pending_quit = 1;
			}

			// Otherwise, all gone?
			if (pending_quit == 2)
			{
				// Save?
				if (save_flag)
				{
					// Try to save
					if (!(config_menus_save(data, 0)))
					{
						// Quit fails
						pending_quit = 0;
						save_flag = 0;

						// Make window unbusy and go on
						ClearWindowBusy(data->window);
						continue;
					}
				}
				break;
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
		// Return pointer to bank
		ret_code = (ULONG)data->bank;

		// Return new path
		if (menu_path)
			strcpy(menu_path, data->bank->path);

		// Have we changed?
		if (data->change)
			data->bank->window.flags |= BTNWF_CHANGED;

		// Clear bank pointer so it won't be freed
		data->bank = 0;
	}

	// Close up
	config_menus_close(data);

	// Reply quit message
	IPC_Reply(quit_msg);

	return ret_code;
}

// Open window
BOOL config_menus_open(config_menus_data *data, struct Screen *screen)
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
	data->newwin.title = GetString(locale, config_menus_title[data->type << 1]);
	data->newwin.locale = locale;
	data->newwin.flags = WINDOW_VISITOR | WINDOW_SCREEN_PARENT | WINDOW_REQ_FILL | WINDOW_AUTO_KEYS;

	// Initial dimensions
	data->dims = (data->type == MENUTYPE_KEYS) ? config_menukeys_window : config_menu_window;

	// Reopening?
	if (data->pos_valid)
	{
		data->dims.char_dim.Left = 0;
		data->dims.char_dim.Top = 0;
		data->dims.fine_dim.Left = data->window_pos.x;
		data->dims.fine_dim.Top = data->window_pos.y;
	}

	// Open window and add objects
	if (!(data->window = OpenConfigWindow(&data->newwin)) ||
		!(data->objlist = AddObjectList(data->window,
										(data->type == MENUTYPE_KEYS) ? config_menukeys_objects : config_menu_objects)))
		return 0;

	// Store window pointer for dragging
	data->drag.window = data->window;

	// Add AppWindow
	if ((data->app_port = CreateMsgPort()))
	{
		data->app_window = AddAppWindowA(0, 0, data->window, data->app_port, 0);
	}

	// Set window ID
	SetWindowID(data->window, 0, WINDOW_BUTTON_CONFIG, (struct MsgPort *)data->ipc);

	// Add menus
	AddWindowMenus(data->window, _lister_menu_menus);

	// Initialise menus
	config_menus_init(data);

	// Make first list active
	config_menus_active(data, 0);

	return 1;
}

// Initialise menu display
void config_menus_init(config_menus_data *data)
{
	// Add menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU, data->menu_list[MENU_MENU]);

	// Select first menu
	SetGadgetValue(data->objlist, GAD_MENUS_MENU, 0);

	// Check disable
	config_menus_check_disable(data, MENU_MENU);

	// Build item list
	config_menus_build_list(data, MENU_ITEM);

	// Select first item
	SetGadgetValue(data->objlist, GAD_MENUS_ITEM, 0);

	// Build sub-item list
	config_menus_build_list(data, MENU_SUB);
}

// Close menu editor
void config_menus_close(config_menus_data *data)
{
	if (data)
	{
		short a;

		// End any drag in progress
		config_menus_end_drag(data, 0, 0);

		// Remove App window
		RemoveAppWindow(data->app_window);

		// Close window
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

		// Free button banks
		CloseButtonBank(data->bank);
		CloseButtonBank(data->restore_bank);

		// Free lists
		for (a = 0; a < 3; a++)
			Att_RemList(data->menu_list[a], REMLIST_FREEDATA);

		// Close timer
		FreeTimer(data->drag.timer);

		// Free data
		FreeVec(data);
	}
}

// Build item list
void config_menus_build_list(config_menus_data *data, short type)
{
	Att_Node *parent_sel;
	short which;
	BOOL disable = 0;

	// Detach list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

	// Free list contents
	Att_RemList(data->menu_list[type], REMLIST_SAVELIST | REMLIST_FREEDATA);

	// Get parent selection
	which = GetGadgetValue(data->objlist, GAD_MENUS_LAYOUT + type);
	parent_sel = Att_FindNode(data->menu_list[type - 1], which);

	// Got parent?
	if (parent_sel)
	{
		Cfg_Button *button;
		Cfg_ButtonFunction *func;

		// Copy name to parent field
		SetGadgetValue(data->objlist, GAD_MENUS_MENU_NAME + type - 1, (ULONG)parent_sel->node.ln_Name);

		// Get parent button
		button = ((menu_node *)parent_sel->data)->button;

		// Doing sub-items?
		if (type == MENU_SUB)
		{
			short count;

			// Go through functions
			for (func = (Cfg_ButtonFunction *)button->function_list.mlh_Head, count = 0; func->node.ln_Succ;
				 func = (Cfg_ButtonFunction *)func->node.ln_Succ, count++)
			{
				// If first function is a separator, disable sub-items
				if (count == 0 && function_label(func) && strncmp(function_label(func), "---", 3) == 0)
				{
					disable = 1;
					break;
				}

				// Ignore empty functions
				if ((!function_label(func) || !*function_label(func)) &&
					IsListEmpty((struct List *)&func->instructions))
					continue;

				// Ignore first function
				if (count == 0)
					continue;

				// Add to menu list
				config_menus_new_node(data->menu_list[type], button, func);
			}
		}

		// Otherwise
		else
		{
			BOOL first = 1;

			// Go through buttons
			for (; button->node.ln_Succ; button = (Cfg_Button *)button->node.ln_Succ)
			{
				// Empty button?
				if (IsListEmpty((struct List *)&button->function_list))
					continue;

				// Find first function, if it's the first see if it's a separator
				func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;
				if (first && function_label(func) && strncmp(function_label(func), "---", 3) == 0)
					disable = 1;
				first = 0;

				// First button?
				if (button == ((menu_node *)parent_sel->data)->button)
				{
					// Skip over parent
					continue;
				}

				// If we hit a menu title, break out
				if (button->button.flags & BUTNF_TITLE)
					break;

				// Add to menu list
				config_menus_new_node(data->menu_list[type], button, func);
			}
		}
	}

	// No parent; disable
	else
		disable = 1;

	// Enable/disable list
	DisableObject(data->objlist, GAD_MENUS_MENU + type, disable);

	// Reattach list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// Check disable of list gadgets
	config_menus_check_disable(data, type);
}

// Check disable of list gadgets
void config_menus_check_disable(config_menus_data *data, short type)
{
	GL_Object *object;
	short disable_all = 0, disable_change = 0, disable_add = 0, disable_move = 0, disable_edit = 0;

	// Find parent list
	object = GetObject(data->objlist, GAD_MENUS_MENU + type);

	// If parent list is disabled, disable everything
	if (object && GADGET(object)->Flags & GFLG_DISABLED)
		disable_all = 1;

	// Otherwise
	else
	{
		short which;
		Att_Node *node;

		// Get selection
		which = GetGadgetValue(data->objlist, GAD_MENUS_MENU + type);
		node = Att_FindNode(data->menu_list[type], which);

		// Nothing selected?
		if (!node)
			disable_change = 1;
	}

	// Clear name field if disabled
	if (disable_all | disable_change)
		SetGadgetValue(data->objlist, GAD_MENUS_MENU_NAME + type, 0);

	// Can't edit menu except for start menus or lister menus
	if (type == MENU_MENU && data->type != MENUTYPE_START && data->type != MENUTYPE_LISTER)
		disable_edit = 1;

	// Disable/enable gadgets
	DisableObject(data->objlist, GAD_MENUS_MENU_ADD + type, (disable_all | disable_add));
	DisableObject(data->objlist, GAD_MENUS_MENU_COPY + type, (disable_all | disable_change | disable_add));
	DisableObject(data->objlist, GAD_MENUS_MENU_DEL + type, (disable_all | disable_change));
	DisableObject(data->objlist, GAD_MENUS_MENU_NAME + type, (disable_all | disable_change));
	DisableObject(data->objlist, GAD_MENUS_MENU_EDIT + type, (disable_all | disable_change | disable_edit));
	DisableObject(data->objlist, GAD_MENUS_MENU_UP + type, (disable_all | disable_change | disable_move));
	DisableObject(data->objlist, GAD_MENUS_MENU_DOWN + type, (disable_all | disable_change | disable_move));
}

// Add a new node
Att_Node *config_menus_new_node(Att_List *list, Cfg_Button *button, Cfg_ButtonFunction *func)
{
	menu_node *node;
	Att_Node *new;

	// Allocate node
	if (!(node = AllocVec(sizeof(menu_node), MEMF_CLEAR)))
		return 0;

	// Fill out node
	node->button = button;
	node->func = func;

	// Add to menu list
	if (!(new = Att_NewNode(list, function_label(func), (ULONG)node, 0)))
	{
		// Failed
		FreeVec(node);
	}

	// Separator?
	else if (new->node.ln_Name && strncmp(new->node.ln_Name, "---", 3) == 0)
		node->flags |= MNF_SEP;

	return new;
}

// New item selected
void config_menus_select_item(config_menus_data *data, short type, BOOL sel)
{
	// Check disable
	config_menus_check_disable(data, type);

	// Menu selected?
	if (type == MENU_MENU)
	{
		// Build item list
		config_menus_build_list(data, MENU_ITEM);

		// Want to select?
		if (sel)
		{
			// Select first item, build sub-item list
			SetGadgetValue(data->objlist, GAD_MENUS_ITEM, 0);
			config_menus_build_list(data, MENU_SUB);
		}
		return;
	}

	// For items, build list of sub-items
	if (type == MENU_ITEM)
	{
		config_menus_build_list(data, MENU_SUB);
	}

	// For sub-items, copy name to field
	else if (type == MENU_SUB)
	{
		Att_Node *sel;

		// Get selection
		if ((sel = Att_FindNode(data->menu_list[MENU_SUB], GetGadgetValue(data->objlist, GAD_MENUS_SUB))))
		{
			// Set field
			SetGadgetValue(data->objlist, GAD_MENUS_SUB_NAME, (ULONG)sel->node.ln_Name);
		}
	}
}

// Add a new menu
void config_menus_add_menu(config_menus_data *data, BOOL copy, unsigned short qual)
{
	Cfg_Button *button;
	Att_Node *node, *copy_sel = 0, *current_sel;
	struct Node *add_after = 0;
	short sel = -1;

	// Get current selection
	current_sel = Att_FindNode(data->menu_list[MENU_MENU], GetGadgetValue(data->objlist, GAD_MENUS_MENU));

	// Get node to copy if needed
	if (copy && !(copy_sel = current_sel))
		return;

	// If we're not inserting, clear selection
	if (!(qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)))
		current_sel = 0;

	// Create a button
	if (!(button =
			  NewButtonWithFunc(data->bank->memory,
								(copy_sel) ? copy_sel->node.ln_Name : (char *)GetString(locale, MSG_USER_MENU_NEW_MENU),
								FTYPE_LEFT_BUTTON)))
	{
		DisplayBeep(data->window->WScreen);
		return;
	}

	// Set title flag
	button->button.flags |= BUTNF_TITLE;

	// Insert?
	if (current_sel)
	{
		// Insert before this node
		Insert(&data->bank->buttons, &button->node, ((menu_node *)current_sel->data)->button->node.ln_Pred);

		// Add after this node
		add_after = &button->node;
	}

	// Add to end of list
	else
		AddTail(&data->bank->buttons, &button->node);

	// Increment button row count
	data->bank->window.rows++;

	// Copying a menu?
	if (copy_sel)
	{
		Cfg_Button *copy_but, *but;
		short count;

		// Go through buttons to copy
		for (copy_but = ((menu_node *)copy_sel->data)->button, count = 0; copy_but->node.ln_Succ;
			 copy_but = (Cfg_Button *)copy_but->node.ln_Succ, count++)
		{
			// Title?
			if (copy_but->button.flags & BUTNF_TITLE)
			{
				// Skip first title
				if (count == 0)
					continue;

				// Break at any other
				break;
			}

			// Copy the button
			if ((but = CopyButton(copy_but, data->bank->memory, 0)))
			{
				// Insert?
				if (add_after)
				{
					// Add after this button
					Insert(&data->bank->buttons, &but->node, add_after);

					// Remember button
					add_after = &but->node;
				}

				// Add to end of button list
				else
					AddTail(&data->bank->buttons, &but->node);

				// Increment button row count
				data->bank->window.rows++;
			}
		}
	}

	// Detach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU, (APTR)~0);

	// Add to menu list
	if ((node = config_menus_new_node(
			 data->menu_list[MENU_MENU],
			 button,
			 (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON))))
	{
		// Insert?
		if (current_sel)
		{
			// Remove from list
			Remove((struct Node *)node);

			// Insert before current selection
			Insert((struct List *)data->menu_list[MENU_MENU], (struct Node *)node, current_sel->node.ln_Pred);
		}

		// Get offset
		sel = Att_FindNodeNumber(data->menu_list[MENU_MENU], node);
	}

	// Attach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU, data->menu_list[MENU_MENU]);

	// Selection?
	if (sel > -1)
	{
		// Select the new node
		SetGadgetValue(data->objlist, GAD_MENUS_MENU, sel);
		config_menus_select_item(data, MENU_MENU, 1);

		// Activate name field
		ActivateStrGad(GADGET(GetObject(data->objlist, GAD_MENUS_MENU_NAME)), data->window);
	}

	// Set change flag
	data->change = 1;
}

// Add a new item or sub-item
void config_menus_add_item(config_menus_data *data, short type, BOOL copy, unsigned short qual, APTR add_item)
{
	Cfg_Button *button;
	Cfg_ButtonFunction *func = 0;
	Att_Node *parent_sel, *node, *copy_sel = 0, *current_sel;

	// Get parent selection
	parent_sel = Att_FindNode(data->menu_list[type - 1], GetGadgetValue(data->objlist, GAD_MENUS_MENU + type - 1));

	// No selection?
	if (!parent_sel)
		return;

	// Get current selection
	current_sel = Att_FindNode(data->menu_list[type], GetGadgetValue(data->objlist, GAD_MENUS_MENU + type));

	// If we're copying, get item to copy
	if (copy && !(copy_sel = current_sel))
		return;

	// If we're not inserting, clear selection
	if (!(qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)))
		current_sel = 0;

	// If we're adding an item, we need a new button
	if (type == MENU_ITEM)
	{
		// Item supplied?
		if (add_item)
			button = (Cfg_Button *)add_item;

		// Copy button?
		else if (copy_sel)
		{
			button = CopyButton(((menu_node *)copy_sel->data)->button, data->bank->memory, 0);
		}

		// Create a new button
		else
		{
			button =
				NewButtonWithFunc(data->bank->memory, GetString(locale, MSG_USER_MENU_NEW_ITEM), FTYPE_LEFT_BUTTON);
		}

		// Failed?
		if (!button)
		{
			DisplayBeep(data->window->WScreen);
			return;
		}

		// Get first function pointer
		if (!(IsListEmpty((struct List *)&button->function_list)))
			func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

		// Detach menu list
		if (!add_item)
			SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

		// Create node
		if ((node = config_menus_new_node(data->menu_list[type], button, func)))
		{
			Att_Node *prev;
			Cfg_Button *prev_but;

			// Insert?
			if (current_sel)
			{
				// Remove node from list
				Remove((struct Node *)node);

				// Insert before current selection
				Insert((struct List *)data->menu_list[type], (struct Node *)node, current_sel->node.ln_Pred);
			}

			// Get previous node
			prev = (Att_Node *)node->node.ln_Pred;

			// Get previous button
			if (prev->node.ln_Pred)
				prev_but = ((menu_node *)prev->data)->button;

			// Otherwise, get menu parent
			else
				prev_but = ((menu_node *)parent_sel->data)->button;

			// Add button to bank
			Insert(&data->bank->buttons, &button->node, (struct Node *)prev_but);

			// Increment button row count
			data->bank->window.rows++;
		}
	}

	// Adding sub-item
	else
	{
		char *name;

		// Get button pointer
		button = ((menu_node *)parent_sel->data)->button;

		// Got function?
		if (add_item)
			func = (Cfg_ButtonFunction *)add_item;

		// Otherwise
		else
		{
			// Create new function
			if (!(func = NewButtonFunction(data->bank->memory, FTYPE_LIST)))
			{
				DisplayBeep(data->window->WScreen);
				return;
			}

			// Copy function?
			if (copy_sel)
			{
				// Copy into new function
				if (!(CopyButtonFunction(((menu_node *)copy_sel->data)->func, data->bank->memory, func)))
				{
					// Failed
					FreeButtonFunction(func);
					DisplayBeep(data->window->WScreen);
					return;
				}
			}

			// Need default name
			else
			{
				// Get name pointer
				name = GetString(locale, MSG_USER_MENU_NEW_ITEM);

				// Fill out label
				if ((func->label = AllocMemH(data->bank->memory, strlen(name) + 1)))
					strcpy(func->label, name);
			}
		}

		// Detach menu list
		if (!add_item)
			SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

		// Create node
		if ((node = config_menus_new_node(data->menu_list[type], button, func)))
		{
			// Insert?
			if (current_sel)
			{
				// Remove node
				Remove((struct Node *)node);

				// Insert before current selection
				Insert((struct List *)data->menu_list[type], (struct Node *)node, current_sel->node.ln_Pred);

				// Insert function in button
				Insert((struct List *)&button->function_list,
					   &func->node,
					   ((menu_node *)current_sel->data)->func->node.ln_Pred);
			}

			// Add function to button
			else
				AddTail((struct List *)&button->function_list, &func->node);
		}
	}

	// Reattach list
	if (!add_item)
		SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// Got a node?
	if (node && !add_item)
	{
		short sel;

		// Get node offset
		if ((sel = Att_FindNodeNumber(data->menu_list[type], node)) > -1)
		{
			// Select the new node
			SetGadgetValue(data->objlist, GAD_MENUS_MENU + type, sel);
			config_menus_select_item(data, type, 1);

			// Activate name field
			ActivateStrGad(GADGET(GetObject(data->objlist, GAD_MENUS_MENU_NAME + type)), data->window);
		}
	}

	// Set change flag
	data->change = 1;
}

// Delete a menu
void config_menus_del_menu(config_menus_data *data)
{
	Att_Node *sel;
	menu_node *node;
	Cfg_Button *button, *next;

	// Get selection
	if (!(sel = Att_FindNode(data->menu_list[MENU_MENU], GetGadgetValue(data->objlist, GAD_MENUS_MENU))))
		return;

	// Get node pointer
	node = (menu_node *)sel->data;

	// Delete all buttons until next menu title
	for (button = node->button; button->node.ln_Succ; button = next)
	{
		// Cache next button
		next = (Cfg_Button *)button->node.ln_Succ;

		// If this is a title, break at this point (unless the first)
		if (button->button.flags & BUTNF_TITLE && button != node->button)
			break;

		// Remove button from bank
		Remove((struct Node *)button);

		// Kill any editors for this button
		config_menu_kill_button_editors(data, button);

		// Free button
		FreeButton(button);

		// Decrement button row count
		data->bank->window.rows--;
	}

	// Detach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU, (APTR)~0);

	// Remove menu node
	Att_RemNode(sel);
	FreeVec(node);

	// Reattach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU, data->menu_list[MENU_MENU]);

	// Select new menu
	config_menus_select_item(data, MENU_MENU, 1);

	// Set change flag
	data->change = 1;
}

// Delete an item
void config_menus_del_item(config_menus_data *data, short type)
{
	Att_Node *sel;
	menu_node *node;

	// Get selection
	if (!(sel = Att_FindNode(data->menu_list[type], GetGadgetValue(data->objlist, GAD_MENUS_MENU + type))))
		return;

	// Get node pointer
	node = (menu_node *)sel->data;

	// Detach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, (APTR)~0);

	// Removing a sub-item?
	if (type == MENU_SUB)
	{
		menu_node *edit;

		// Remove function from button
		Remove((struct Node *)node->func);

		// See if item is being edited
		if ((edit = config_menus_find_editor(data, node->button, node->func)))
		{
			// Kill the editor
			IPC_Command(edit->ipc, IPC_QUIT, 0, 0, 0, 0);
		}

		// Free function
		FreeButtonFunction(node->func);
	}

	// Removing a whole "button"
	else
	{
		// Remove button from bank
		Remove((struct Node *)node->button);

		// Kill any editors for this button
		config_menu_kill_button_editors(data, node->button);

		// Free button
		FreeButton(node->button);

		// Decrement button row count
		data->bank->window.rows--;

		// Detach sub-item list
		SetGadgetChoices(data->objlist, GAD_MENUS_SUB, (APTR)~0);

		// Clear sub-item list
		Att_RemList(data->menu_list[MENU_SUB], REMLIST_SAVELIST | REMLIST_FREEDATA);
	}

	// Free menu node
	Att_RemNode(sel);
	FreeVec(node);

	// Reattach menu list
	SetGadgetChoices(data->objlist, GAD_MENUS_MENU + type, data->menu_list[type]);

	// Do item selection
	config_menus_select_item(data, type, 1);

	// Set change flag
	data->change = 1;
}

// Make a list 'active'
void config_menus_active(config_menus_data *data, short active)
{
	short a;

	// No such concept in menu key editor
	if (data->type == MENUTYPE_KEYS)
		return;

	// Store active value
	data->active = active;

	// Set line pattern
	SetDrPt(data->window->RPort, 0xcccc);

	// Go through the 3 lists
	for (a = 0; a < 3; a++)
	{
		struct Rectangle rect, butrect;

		// Set pen
		SetAPen(data->window->RPort, DRAWINFO(data->window)->dri_Pens[(active == a) ? SHINEPEN : BACKGROUNDPEN]);

		// Get lister rectangle
		GetObjectRect(data->objlist, GAD_MENUS_MENU + a, &rect);

		// Get rectangle of 'move down' button
		GetObjectRect(data->objlist, GAD_MENUS_MENU_DOWN + a, &butrect);

		// Stretch rectangle to cover buttons
		rect.MaxY = butrect.MaxY;

		// Stretch rectangle
		rect.MinX -= 2;
		rect.MinY -= 2;
		rect.MaxX += 2;
		rect.MaxY += 2;

		// Draw rectangle
		Move(data->window->RPort, rect.MinX, rect.MinY);
		Draw(data->window->RPort, rect.MaxX, rect.MinY);
		Draw(data->window->RPort, rect.MaxX, rect.MaxY);
		Draw(data->window->RPort, rect.MinX, rect.MaxY);
		Draw(data->window->RPort, rect.MinX, rect.MinY);
	}

	// Clear line pattern
	SetDrPt(data->window->RPort, 0xffff);
}
