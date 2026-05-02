#include "config_lib.h"
#include "config_buttons.h"
#include <Program/main_commands.h>

int LIBFUNC L_Config_Buttons(REG(a0, ButtonsStartup *startup),
							 REG(a1, IPCData *ipc),
							 REG(a2, IPCData *owner_ipc),
							 REG(a3, struct Screen *screen),
							 REG(d0, IPTR command_list))
{
	config_buttons_data *data;
	IPCMessage *quit_msg = 0;
	long ret = CONFIG_OK;
	short undo_flag = 0, pending_quit = 0, save_flag = 0;

	// Allocate data
	if (!(data = AllocVec(sizeof(config_buttons_data), MEMF_CLEAR)) || !(data->drag.timer = AllocTimer(UNIT_VBLANK, 0)))
	{
		// Failed
		Forbid();
		FreeVec(data);
		return 0;
	}

	// Initialise data
	NewList(&data->bank_list);
	NewList(&data->clip_list);
	NewList(&data->edit_list);
	data->ipc = ipc;
	data->owner_ipc = owner_ipc;
	data->select_col = -1;
	data->select_row = -1;
	data->command_list = command_list;
	data->palette_data.fgpen = 1;
	data->palette_data.bgpen = 0;

	// Create clipboard port
	if ((data->clip_port = CreateMsgPort()))
	{
		data->clip_handle.cbh_CBport = *data->clip_port;
		data->clip_handle.cbh_Req.io_Message.mn_ReplyPort = &data->clip_handle.cbh_CBport;
	}
	data->clip_count = 1;

	// Read clipboard list
	button_load_clipboard(data);

	// Allocate font requester
	data->font_req = AllocAslRequestTags(ASL_FontRequest,
										 ASLFO_TitleText,
										 GetString(locale, MSG_BUTTONS_SELECT_FONT),
										 ASLFO_Flags,
										 FOF_PRIVATEIDCMP,
										 TAG_END);

	// Open window
	if (!(_config_buttons_open(data, screen)))
	{
		// Failed
		_config_buttons_cleanup(data);
		return 0;
	}

	// Got startup packet?
	if (startup && startup->bank)
	{
		// Edit this bank
		_config_buttons_new_bank(data, startup->bank, startup->ipc);

		// Edit this button
		if (startup->flag)
		{
			_config_buttons_edit_button(data, (short)(startup->button >> 16), (short)(startup->button & 0xffff));
		}
	}

	// Event loop
	FOREVER
	{
		struct IntuiMessage *msg;
		IPCMessage *imsg;
		short quit_flag = 0;

		// Check drag
		if (config_drag_check(&data->drag))
		{
			// End drag
			config_drag_end(&data->drag, 0);
		}

		// Any IPC messages?
		while ((imsg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Look at command
			switch (imsg->command)
			{
			// Quit immediately
			case IPC_QUIT:
				quit_flag = 2;
				quit_msg = imsg;
				imsg = 0;
				ret = -1;
				break;

			// Hide
			case IPC_HIDE:
				if (data->window)
				{
					bank_node *bank;

					// Store window position
					data->window_pos = *((struct IBox *)&data->window->LeftEdge);
					data->pos_valid = 1;

					// Hide clipboard
					config_buttons_hide_clipboard(data);

					// Close main window
					CloseConfigWindow(data->window);
					data->window = 0;
					data->objlist = 0;

					// Go through bank list
					for (bank = (bank_node *)data->bank_list.lh_Head; bank->node.ln_Succ;
						 bank = (bank_node *)bank->node.ln_Succ)
					{
						// Tell children to hide
						IPC_ListCommand(&bank->proc_list, IPC_HIDE, 0, 0, 0);
					}

					// Kill paint box if it's up
					if (data->paint_box)
						IPC_Quit(data->paint_box, 0, 0);
				}
				break;

			// Show
			case IPC_SHOW:
				if (!data->window)
				{
					// Re-open window
					if (!(_config_buttons_open(data, (struct Screen *)imsg->data)))
					{
						quit_flag = 1;
						ret = -1;
					}
					else
					{
						bank_node *bank;

						// Go through bank list
						for (bank = (bank_node *)data->bank_list.lh_Head; bank->node.ln_Succ;
							 bank = (bank_node *)bank->node.ln_Succ)
						{
							// Tell children to appear
							IPC_ListCommand(&bank->proc_list, IPC_SHOW, 0, (IPTR)data->window, 0);
						}
					}
				}
				break;

			// Activate
			case IPC_ACTIVATE:
				if (data->window)
				{
					WindowToFront(data->window);
					ActivateWindow(data->window);
				}
				break;

			// Process a key
			case BUTTONEDIT_PROCESS_KEY:
				if (!pending_quit && data->window &&
			(_config_buttons_handle_key(data, (UWORD)imsg->flags, (UWORD)(IPTR)imsg->data)))
				{
					ret = CONFIG_CHANGE_BUTTONS;
					data->change = 1;
				}
				break;

			// Edit a button
			case BUTTONEDIT_EDIT_BUTTON:
				if (!pending_quit && data->window)
					_config_buttons_edit_button(data, (short)imsg->flags, (short)(IPTR)imsg->data);
				break;

			// Got a button back from the editor
			case BUTTONEDIT_RETURN:
				if (_config_buttons_receive_edit(data, (ButtonReturn *)imsg->data, !pending_quit))
				{
					ret = CONFIG_CHANGE_BUTTONS;
					data->change = 1;
				}
				break;

			// Re-edit a bank
			case BUTTONEDIT_RE_EDIT_ME:

				// Clear bank pointer we had if it's the same
				if (data->bank_node && data->bank_node->bank == (Cfg_ButtonBank *)imsg->flags)
					data->bank_node = 0;

			// Edit a new bank
			case BUTTONEDIT_EDIT_ME:
				_config_buttons_new_bank(data, (Cfg_ButtonBank *)imsg->flags, (IPCData *)imsg->data);
				if (imsg->command == BUTTONEDIT_EDIT_ME)
					data->change = 0;
				break;

			// Stop editing a bank
			case BUTTONEDIT_PISS_OFF:
				if (data->bank_node && data->bank_node->bank == (Cfg_ButtonBank *)imsg->data)
				{
					data->bank_node = 0;
					_config_buttons_new_bank(data, 0, 0);
					data->change = 0;
				}
				break;

			// Change bank pointer
			case BUTTONEDIT_CHANGE_BANK: {
				bank_node *node;

				// See if node is in list
				for (node = (bank_node *)data->bank_list.lh_Head; node->node.ln_Succ;
					 node = (bank_node *)node->node.ln_Succ)
				{
					// Match old pointer?
					if (node->bank == (Cfg_ButtonBank *)imsg->flags)
					{
						// Store new pointer
						node->bank = (Cfg_ButtonBank *)imsg->data;
						break;
					}
				}
			}
			break;

			// New button selected
			case BUTTONEDIT_SELECT_BUTTON:

				// Store selected button
				data->select_col = (short)imsg->flags;
					data->select_row = (short)(IPTR)imsg->data;
				_config_buttons_fix_controls(data);

				// Valid button?
				if (data->select_col != -1)
				{
					// Are we in paint mode?
					if (data->paint_box && !data->bank_node->toolbar)
					{
						Cfg_Button *button;

						// Lock bank
						GetSemaphore(&data->bank_node->bank->lock, SEMF_SHARED, 0);

						// Get button
						if ((button = _config_buttons_get_button(data, data->select_col, data->select_row)))
						{
							// Store new colours
							button->button.fpen = data->palette_data.fgpen;
							button->button.bpen = data->palette_data.bgpen;
						}

						// Unlock bank
						FreeSemaphore(&data->bank_node->bank->lock);

						// Button changed?
						if (button)
						{
							// Redraw button
							_config_buttons_redraw_button(data, data->bank_node, button);

							// Mark change
							ret = CONFIG_CHANGE_BUTTONS;
							data->change = 1;
						}
					}
				}
				break;

			// Copy a button to the clipboard
			case BUTTONEDIT_CLIP_BUTTON:

				// Copy to clipboard
				button_to_clipboard(data, (Cfg_Button *)imsg->data);
				break;

			// Copy a button to a bank
			case BUTTONEDIT_BUTTON_TO_BANK: {
				Cfg_Button *button;

				// Copy the button; this will ensure it is the correct type
				if ((button = CopyButton((Cfg_Button *)imsg->data, 0, data->bank_node->bank->window.flags & BTNWF_GFX)))
				{
					// Send button to bank
					_button_to_bank(
						data, (Cfg_Button *)imsg->data, ((Point *)imsg->data_free)->x, ((Point *)imsg->data_free)->y);

					// Free copy
					FreeButton(button);
				}
				else
					DisplayBeep(data->window->WScreen);

				// Free button
				FreeButton((Cfg_Button *)imsg->data);
			}
			break;

			// Passed an AppMessage
			case CFG_APPMESSAGE_PASS:
				button_receive_appmsg(data, (struct AppMessage *)imsg->data);
				ReplyAppMessage((DOpusAppMessage *)imsg->data);
				break;

			// Button bank saying goodbye
			case BUTTONEDIT_GOODBYE: {
				bank_node *bank;

				// Go through bank list
				for (bank = (bank_node *)data->bank_list.lh_Head; bank->node.ln_Succ;
					 bank = (bank_node *)bank->node.ln_Succ)
				{
					// Is this the bank that just left?
					if (bank->bank == (Cfg_ButtonBank *)imsg->data)
					{
						// Save quit message
						bank->quit_msg = imsg;
						imsg = 0;

						// Does bank have outstanding processes?
						if (!(IPC_ListQuit(&bank->proc_list, 0, 0, FALSE)))
						{
							// Remove this bank from our list
							_config_buttons_remove_bank(data, bank, undo_flag, (quit_flag == 2));
						}

						// Otherwise set pending quit flag in bank
						else
						{
							bank->pending_quit = 1;
						}
						break;
					}
				}
			}
			break;

			// Process saying goodbye
			case IPC_GOODBYE: {
				bank_node *bank;
				IPCData *bye;
				edit_node *node;

				// Get process
				bye = (IPCData *)imsg->data;

				// Paint box?
				if (bye == data->paint_box)
				{
					// Paint box is gone
					data->paint_box = 0;

					// If window is open, clear check
					if (data->window)
					{
						SetGadgetValue(data->objlist, GAD_BUTTONS_PAINT_MODE, 0);
					}
				}

				// See if it's a button editor
				else
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

				// Go through bank list
				for (bank = (bank_node *)data->bank_list.lh_Head; bank->node.ln_Succ;)
				{
					bank_node *next = (bank_node *)bank->node.ln_Succ;

					// All process gone and pending quit?
					if (bank->pending_quit && (IsListLockEmpty(&bank->proc_list)))
					{
						// Remove this bank from our list
						_config_buttons_remove_bank(data, bank, undo_flag, (quit_flag == 2));
					}

					// Get next
					bank = next;
				}

				// All banks gone and pending quit?
				if (pending_quit && (IsListEmpty(&data->bank_list)) && !data->paint_box)
				{
					quit_flag = 1;
					pending_quit = 2;
				}
			}
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
			while ((msg = GetWindowMsg(data->window->UserPort)) ||
				   (data->clip_window && (msg = GetWindowMsg(data->clip_window->UserPort))))
			{
				struct IntuiMessage msg_copy;
				struct Gadget *gadget;

				// Copy message and reply
				msg_copy = *msg;
				ReplyWindowMsg(msg);
				gadget = (struct Gadget *)msg_copy.IAddress;

				if (pending_quit)
					continue;

				// Look at message
				switch (msg_copy.Class)
				{
				// Close window
				case IDCMP_CLOSEWINDOW:

					// Clipboard?
					if (msg_copy.IDCMPWindow == data->clip_window)
					{
						// Hide clipboard
						config_buttons_hide_clipboard(data);

						// Clear flag
						SetGadgetValue(data->objlist, GAD_BUTTONS_SHOW_CLIPBOARD, FALSE);
						break;
					}

					// Set flags to quit
					undo_flag = 1;
					quit_flag = 1;
					break;

				// Mouse move
				case IDCMP_MOUSEMOVE:

					// Dragging something?
					if (data->drag.drag)
					{
						// Handle drag
						config_drag_move(&data->drag);
						break;
					}

					// Clipboard scroller?
					if (msg_copy.IAddress != (APTR)data->clip_scroller)
						break;

					// Store value of clipboard scroller
					StoreGadgetValue(data->clip_objlist, &msg_copy);

				// Gadget
				case IDCMP_GADGETDOWN:

					// Look at gadget ID
					switch (gadget->GadgetID)
					{
					// Clipboard scroller
					case GAD_BUTTONS_CLIPBOARD_SCROLLER:

						// Show clipboard
						show_button_clipboard(data, 0);
						break;
					}
					break;

				// Gadget
				case IDCMP_GADGETUP:

					// Look at gadget ID
					switch (gadget->GadgetID)
					{
					// Cancel
					case GAD_BUTTONS_CANCEL:
						undo_flag = 1;
						quit_flag = 1;
						break;

					// Save
					case GAD_BUTTONS_SAVE:
						save_flag = 1;

					// Ok
					case GAD_BUTTONS_OK:
						quit_flag = 1;
						break;

					// New name
					case GAD_BUTTONS_NAME:
						if (!data->bank_node)
							break;

						// Lock bank
						GetSemaphore(&data->bank_node->bank->lock, SEMF_EXCLUSIVE, 0);

						// Store name
						strcpy(data->bank_node->bank->window.name,
							   (char *)GetGadgetValue(data->objlist, GAD_BUTTONS_NAME));

						// Unlock bank
						FreeSemaphore(&data->bank_node->bank->lock);

						// Refresh display
						_config_buttons_refresh(data, data->bank_node, 0);

						// Set change flag
						ret = CONFIG_CHANGE_BUTTONS;
						data->change = 1;
						break;

					// Picture
					case GAD_BUTTONS_BACKPIC:
					case GAD_BUTTONS_BACKPIC_POPUP: {
						char *ptr;

						if (!data->bank_node || !(ptr = (char *)GetGadgetValue(data->objlist, GAD_BUTTONS_BACKPIC)) ||
							stricmp(ptr, data->bank_node->bank->backpic) == 0)
							break;

						// Store new picture name
						GetSemaphore(&data->bank_node->bank->lock, SEMF_EXCLUSIVE, 0);
						strcpy(data->bank_node->bank->backpic, ptr);
						FreeSemaphore(&data->bank_node->bank->lock);

						// Re-open window
						IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_REOPEN, 0, 0, 0, REPLY_NO_PORT);
						data->change = 1;
					}
					break;

					// Pick font
					case GAD_BUTTONS_FONT_GLASS:
						if (!data->bank_node)
							break;

						// Make window busy
						SetWindowBusy(data->window);

						// Put up font requester
						if (!(AslRequestTags(data->font_req,
											 ASLFO_Window,
											 data->window,
											 ASLFO_InitialName,
											 data->bank_node->bank->window.font_name,
											 ASLFO_InitialSize,
											 data->bank_node->bank->window.font_size,
											 TAG_END)))
						{
							ClearWindowBusy(data->window);
							break;
						}
						ClearWindowBusy(data->window);

						// Update gadgets
						SetGadgetValue(data->objlist, GAD_BUTTONS_FONT_NAME, (IPTR)data->font_req->fo_Attr.ta_Name);
						SetGadgetValue(data->objlist, GAD_BUTTONS_FONT_SIZE, (ULONG)data->font_req->fo_Attr.ta_YSize);

					// Font name/size
					case GAD_BUTTONS_FONT_NAME:
					case GAD_BUTTONS_FONT_SIZE:
						if (!data->bank_node)
							break;

						// Lock bank
						GetSemaphore(&data->bank_node->bank->lock, SEMF_EXCLUSIVE, 0);

						// Store font name and size
						strcpy(data->bank_node->bank->window.font_name,
							   (char *)GetGadgetValue(data->objlist, GAD_BUTTONS_FONT_NAME));
						data->bank_node->bank->window.font_size = GetGadgetValue(data->objlist, GAD_BUTTONS_FONT_SIZE);

						// Add .font if necessary
						if (!(strstr(data->bank_node->bank->window.font_name, ".font")))
						{
							StrConcat(data->bank_node->bank->window.font_name, ".font", 30);
							SetGadgetValue(
								data->objlist, GAD_BUTTONS_FONT_NAME, (IPTR)data->bank_node->bank->window.font_name);
						}

						// Unlock bank
						FreeSemaphore(&data->bank_node->bank->lock);

						// Refresh display (unless tab)
						if (msg_copy.Code != 0x9)
							_config_buttons_refresh(data, data->bank_node, BUTREFRESH_FONT);

						// Set change flag
						ret = CONFIG_CHANGE_BUTTONS;
						data->change = 1;
						break;

					// Add a row
					case GAD_BUTTONS_ROWS_ADD:
						if (_config_buttons_rows_add(data, -1))
						{
							ret = CONFIG_CHANGE_BUTTONS;
							data->change = 1;
						}
						break;

					// Insert a row
					case GAD_BUTTONS_ROWS_INSERT: {
						short row;

						// Get current row
						if ((_config_buttons_get_colrow(data, 0, &row)) != -1)
						{
							// Insert row
							if (_config_buttons_rows_add(data, row))
							{
								ret = CONFIG_CHANGE_BUTTONS;
								data->change = 1;
							}
						}
					}
					break;

					// Add a column
					case GAD_BUTTONS_COLUMNS_ADD:
						if (_config_buttons_columns_add(data, -1))
						{
							ret = CONFIG_CHANGE_BUTTONS;
							data->change = 1;
						}
						break;

					// Insert a column
					case GAD_BUTTONS_COLUMNS_INSERT: {
						short col;

						// Get current column
						if ((_config_buttons_get_colrow(data, &col, 0)) != -1)
						{
							// Insert column
							if (_config_buttons_columns_add(data, col))
							{
								ret = CONFIG_CHANGE_BUTTONS;
								data->change = 1;
							}
						}
					}
					break;

					// Remove a row
					case GAD_BUTTONS_ROWS_REMOVE:
						if (_config_buttons_rows_remove(data))
						{
							ret = CONFIG_CHANGE_BUTTONS;
							data->change = 1;
						}
						break;

					// Remove a column
					case GAD_BUTTONS_COLUMNS_REMOVE:
						if (_config_buttons_columns_remove(data))
						{
							ret = CONFIG_CHANGE_BUTTONS;
							data->change = 1;
						}
						break;

					// Delete a row
					case GAD_BUTTONS_ROWS_DELETE: {
						short row;

						// Get current row
						if ((_config_buttons_get_colrow(data, 0, &row)) != -1)
						{
							// Delete row
							if (_config_buttons_delete_row(data, row))
							{
								ret = CONFIG_CHANGE_BUTTONS;
								data->change = 1;
							}
						}
					}
					break;

					// Delete a column
					case GAD_BUTTONS_COLUMNS_DELETE: {
						short col;

						// Get current column
						if ((_config_buttons_get_colrow(data, &col, 0)) != -1)
						{
							// Delete row
							if (_config_buttons_delete_column(data, col))
							{
								ret = CONFIG_CHANGE_BUTTONS;
								data->change = 1;
							}
						}
					}
					break;

					// Bank border
					case GAD_BUTTONS_BANK_BORDER:

						// Get state of flag (reversed)
						if (msg_copy.Code)
						{
							data->bank_node->bank->window.flags &= ~BTNWF_NO_BORDER;
							DisableObject(data->objlist, GAD_BUTTONS_DRAGBAR_ORIENTATION, TRUE);
						}
						else
						{
							data->bank_node->bank->window.flags |= BTNWF_NO_BORDER;
							DisableObject(data->objlist, GAD_BUTTONS_DRAGBAR_ORIENTATION, FALSE);
						}

						// Re-open window
						IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_REOPEN, 0, 0, 0, REPLY_NO_PORT);
						data->change = 1;
						break;

					// Dragbar orientation
					case GAD_BUTTONS_DRAGBAR_ORIENTATION:

						// Valid bank?
						if (data->bank_node)
						{
							short val;

							// Clear flags initially
							data->bank_node->bank->window.flags &=
								~(BTNWF_VERT | BTNWF_HORIZ | BTNWF_NONE | BTNWF_RIGHT_BELOW);
							val = GetGadgetValue(data->objlist, GAD_BUTTONS_DRAGBAR_ORIENTATION);

							// Horizontal?
							if (val == 1)
								data->bank_node->bank->window.flags |= BTNWF_HORIZ;

							// Vertical?
							else if (val == 2)
								data->bank_node->bank->window.flags |= BTNWF_VERT;

							// Horizontal below
							else if (val == 3)
								data->bank_node->bank->window.flags |= BTNWF_HORIZ | BTNWF_RIGHT_BELOW;

							// Vertical right?
							else if (val == 4)
								data->bank_node->bank->window.flags |= BTNWF_VERT | BTNWF_RIGHT_BELOW;

							// None?
							else if (val == 5)
								data->bank_node->bank->window.flags |= BTNWF_NONE;

							// Refresh window
							_config_buttons_refresh(data, data->bank_node, BUTREFRESH_RESIZE);
							data->change = 1;
						}
						break;

					// Borderless buttons
					case GAD_BUTTONS_BORDERLESS:

						// Get state of flag
						if (msg_copy.Code)
							data->bank_node->bank->window.flags |= BTNWF_BORDERLESS;
						else
							data->bank_node->bank->window.flags &= ~BTNWF_BORDERLESS;

						// Re-open window
						IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_REOPEN, 0, 0, 0, REPLY_NO_PORT);
						data->change = 1;
						break;

					// Simple-refresh window
					case GAD_BUTTONS_SIMPLE_REFRESH:

						// Get state of flag (reversed)
						if (msg_copy.Code)
							data->bank_node->bank->window.flags &= ~BTNWF_SMART_REFRESH;
						else
							data->bank_node->bank->window.flags |= BTNWF_SMART_REFRESH;

						// Re-open window
						IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_REOPEN, 0, 0, 0, REPLY_NO_PORT);
						data->change = 1;
						break;

					// No dog-ears
					case GAD_BUTTONS_NO_DOGEARS:

						// Get state of flag
						if (msg_copy.Code)
							data->bank_node->bank->window.flags |= BTNWF_NO_DOGEARS;
						else
							data->bank_node->bank->window.flags &= ~BTNWF_NO_DOGEARS;

						// Re-draw bank
						IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_REDRAW, 0, 0, 0, 0);
						data->change = 1;
						break;

					// Auto close
					case GAD_BUTTONS_AUTO_CLOSE:

						// Get state of flag
						if (msg_copy.Code)
						{
							data->bank_node->bank->window.flags |= BTNWF_AUTO_CLOSE;
							if (data->bank_node->bank->window.flags & BTNWF_AUTO_ICONIFY)
							{
								data->bank_node->bank->window.flags &= ~BTNWF_AUTO_ICONIFY;
								SetGadgetValue(data->objlist, GAD_BUTTONS_AUTO_ICONIFY, 0);
							}
						}
						else
							data->bank_node->bank->window.flags &= ~BTNWF_AUTO_CLOSE;
						data->change = 1;
						break;

					// Auto iconify
					case GAD_BUTTONS_AUTO_ICONIFY:

						// Get state of flag
						if (msg_copy.Code)
						{
							data->bank_node->bank->window.flags |= BTNWF_AUTO_ICONIFY;
							if (data->bank_node->bank->window.flags & BTNWF_AUTO_CLOSE)
							{
								data->bank_node->bank->window.flags &= ~BTNWF_AUTO_CLOSE;
								SetGadgetValue(data->objlist, GAD_BUTTONS_AUTO_CLOSE, 0);
							}
						}
						else
							data->bank_node->bank->window.flags &= ~BTNWF_AUTO_ICONIFY;
						data->change = 1;
						break;

					// Active popups
					case GAD_BUTTONS_ACTIVE_POPUP:

						// Get state of flag
						if (msg_copy.Code)
							data->bank_node->bank->window.flags |= BTNWF_ACTIVE_POPUP;
						else
							data->bank_node->bank->window.flags &= ~BTNWF_ACTIVE_POPUP;
						data->change = 1;
						break;

					// Clipboard scroller
					case GAD_BUTTONS_CLIPBOARD_SCROLLER:

						// Show clipboard
						show_button_clipboard(data, 0);
						break;

					// Edit a button
					case GAD_BUTTONS_EDIT:

						if (!pending_quit && data->select_col != -1 && data->select_row != -1)
						{
							_config_buttons_edit_button(data, data->select_col, data->select_row);
						}
						break;

					// Copy/cut/erase a button
					case GAD_BUTTONS_COPY:
					case GAD_BUTTONS_CUT:
					case GAD_BUTTONS_ERASE: {
						Cfg_Button *button;

						// Get button
						if ((button = _config_buttons_get_button(data, data->select_col, data->select_row)))
						{
							// Copy to clipboard (unless erase)
							if (gadget->GadgetID != GAD_BUTTONS_ERASE)
							{
								Cfg_Button *copy;

								// Copy button
								if ((copy = CopyButton(button, 0, -1)))
									button_to_clipboard(data, copy);
							}

							// Erase button (unless copy)
							if (gadget->GadgetID != GAD_BUTTONS_COPY)
							{
								Cfg_Button *blank;

								// Create a blank button
								if ((blank = NewButton(0)))
								{
									// Graphical button?
									if (button->button.flags & BUTNF_GRAPHIC)
										blank->button.flags |= BUTNF_GRAPHIC;

									// Fake button return
									_config_buttons_fake_return(data, button, blank);

									// Free blank button
									FreeButton(blank);
								}
							}
						}
					}
					break;

					// Paint mode
					case GAD_BUTTONS_PAINT_MODE:

						// Turn off?
						if (msg_copy.Code == 0)
						{
							// Paint box up?
							if (data->paint_box)
							{
								// Send quit
								IPC_Quit(data->paint_box, 0, 0);
							}
						}

						// Turn on
						else
						{
							// Is paint box up?
							if (data->paint_box)
								IPC_Command(data->paint_box, IPC_ACTIVATE, 0, 0, 0, 0);

							// Need to launch colour selector
							else
							{
								// Initialise palette startup
								data->palette_data.screen_data = data->screen_data;
								palette_editor_init(&data->palette_data);
								data->palette_data.main_ipc = data->owner_ipc;

								// Fill out some data
								data->palette_data.newwin.parent = data->window;
								data->palette_data.owner_ipc = ipc;

								// Launch selector
								IPC_Launch(0,
										   &data->paint_box,
										   "dopus_paint_box",
										   IPC_NATIVE(PaletteBox),
										   STACK_DEFAULT,
										   (IPTR)&data->palette_data,
										   (struct Library *)DOSBase);
							}
						}
						break;

					// Show clipboard
					case GAD_BUTTONS_SHOW_CLIPBOARD:

						// Turn off?
						if (msg_copy.Code == 0)
						{
							config_buttons_hide_clipboard(data);
						}

						// Turn on
						else
						{
							config_buttons_show_clipboard(data);
						}
						break;

					// Clear clipboard
					case GAD_BUTTONS_CLIPBOARD_CLEAR: {
						Cfg_Button *button;

						// Go through button clip list
						for (button = (Cfg_Button *)data->clip_list.lh_Head; button->node.ln_Succ;)
						{
							Cfg_Button *next = (Cfg_Button *)button->node.ln_Succ;

							// Free this button
							FreeButton(button);

							// Get next button
							button = next;
						}

						// Initialise list
						NewList(&data->clip_list);
						data->clip_count = 1;

						// Refresh display
						data->last_clip_draw = (Cfg_Button *)-1;
						show_button_clipboard(data, 3);
					}
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
						IPC_Command(data->owner_ipc, IPC_HELP, (1 << 31), "Button Bank Editor", 0, REPLY_NO_PORT);

						// Clear busy pointer
						ClearWindowBusy(data->window);
						break;
					}

					// No valid bank?
					if (!data->bank_node)
						break;

					// Pass to key handler
					if (_config_buttons_handle_key(data, msg_copy.Code, msg_copy.Qualifier))
					{
						ret = CONFIG_CHANGE_BUTTONS;
						data->change = 1;
					}
					break;

				// Inactive window does right button down
				case IDCMP_INACTIVEWINDOW:
					msg_copy.Code = MENUUP;

				// Mouse button
				case IDCMP_MOUSEBUTTONS:

					// Left button down?
					if (msg_copy.Code == SELECTDOWN)
					{
						// Is there a valid button shown in the clipboard
						if (data->clip_valid && msg_copy.IDCMPWindow == data->clip_window)
						{
							// Clicked on button image?
							if (msg_copy.MouseX >= data->clip_rect.MinX && msg_copy.MouseY >= data->clip_rect.MinY &&
								msg_copy.MouseX <= data->clip_rect.MaxX && msg_copy.MouseY <= data->clip_rect.MaxY)
							{
								// Start the drag
								config_drag_start_window(
									&data->drag, data->clip_window, &data->clip_rect, msg_copy.MouseX, msg_copy.MouseY);
							}
						}
					}

					// Left up/right down
					else if (msg_copy.Code == SELECTUP || msg_copy.Code == MENUDOWN)
					{
						// Dragging something?
						if (data->drag.drag)
						{
							// End drag
							config_drag_end(&data->drag, 0);

							// Select up = handle drag drop
							if (msg_copy.Code == SELECTUP)
							{
								_config_buttons_drop_button(data, msg_copy.MouseX, msg_copy.MouseY);
							}
						}
					}
					break;

				// Ticks
				case IDCMP_INTUITICKS:
					++data->drag.tick_count;
					break;

				// Menu event
				case IDCMP_MENUPICK: {
					struct MenuItem *item;

					// Get item
					if (!(item = ItemAddress(data->window->MenuStrip, msg_copy.Code)))
						break;

					// Quit?
					if (MENUID(item) == BUTTONEDIT_MENU_CLOSE)
					{
						// Set flags to quit
						undo_flag = 1;
						quit_flag = 1;
						break;
					}

					// Send to main process
					IPC_Command(owner_ipc, MAINCMD_COMMAND, MENUID(item), 0, 0, 0);
				}
				break;
				}

				// Check quit flag
				if (quit_flag)
					break;
			}
		}

		// Check quit flag
		if (quit_flag)
		{
			// Not already quit pending?
			if (!pending_quit)
			{
				bank_node *bank;

				// Set busy pointer
				if (data->window)
					SetWindowBusy(data->window);

				// Go through bank list
				for (bank = (bank_node *)data->bank_list.lh_Head; bank->node.ln_Succ;)
				{
					bank_node *next = (bank_node *)bank->node.ln_Succ;

					// Does bank have outstanding processes?
					if (IPC_ListQuit(&bank->proc_list, 0, !undo_flag, FALSE))
					{
						// Set pending quit flag in bank
						bank->pending_quit = 1;
					}

					// Remove this bank from our list
					else
						_config_buttons_remove_bank(data, bank, undo_flag, (quit_flag == 2));

					// Get next
					bank = next;
				}

				// All banks gone?
				if (IsListEmpty(&data->bank_list) && !data->paint_box)
					break;

				// Kill paint box if it's up
				if (data->paint_box)
					IPC_Quit(data->paint_box, 0, 0);

				// Set pending quit flag
				pending_quit = 1;
			}
		}

		// Wait for an event
		Wait(1 << ipc->command_port->mp_SigBit | ((data->drag.timer) ? (1 << data->drag.timer->port->mp_SigBit) : 0) |
			 ((data->clip_window) ? (1 << data->clip_window->UserPort->mp_SigBit) : 0) |
			 ((data->window) ? (1 << data->window->UserPort->mp_SigBit) : 0));
	}

	// Save clipboard
	button_save_clipboard(data, TRUE);

	// Clean up
	_config_buttons_cleanup(data);

	// Reply quit message
	IPC_Reply(quit_msg);

	// Save flag?
	if (save_flag)
		ret |= CONFIG_SAVE;

	return (undo_flag) ? 0 : ret;
}

// Open button editor display
BOOL _config_buttons_open(config_buttons_data *data, struct Screen *screen)
{
	// Fill in new window
	data->newwin.parent = screen;
	data->newwin.dims = &data->win_dims;
	data->newwin.title = GetString(locale, MSG_BUTTONS_TITLE);
	data->newwin.locale = locale;
	data->newwin.flags = WINDOW_SCREEN_PARENT | WINDOW_VISITOR | WINDOW_REQ_FILL | WINDOW_AUTO_KEYS;

	// Initial position
	data->win_dims = _config_buttons_window;

	// Stored position valid?
	if (data->pos_valid)
	{
		data->win_dims.char_dim.Left = 0;
		data->win_dims.char_dim.Top = 0;
		data->win_dims.fine_dim.Left = data->window_pos.Left;
		data->win_dims.fine_dim.Top = data->window_pos.Top;
	}

	// Open window and add objects
	if (!(data->window = OpenConfigWindow(&data->newwin)) ||
		!(data->objlist = AddObjectList(data->window, _config_buttons_objects)))
	{
		return 0;
	}

	// Initialise screen data
	IPC_Command(data->owner_ipc, MAINCMD_GET_SCREEN_DATA, 0, &data->screen_data, 0, REPLY_NO_PORT);

	// Set window ID
	SetWindowID(data->window, 0, WINDOW_BUTTON_CONFIG, (struct MsgPort *)data->ipc);

	// Initialise gadgets
	_config_buttons_new_bank(data, 0, 0);
	data->change = 0;

	return 1;
}

// Clean up
void _config_buttons_cleanup(config_buttons_data *data)
{
	if (data)
	{
		// Hide the clipboard
		config_buttons_hide_clipboard(data);

		// Close window
		CloseConfigWindow(data->window);

		// Free font requester
		FreeAslRequest(data->font_req);

		// Close clipboard port
		if (data->clip_port)
			DeleteMsgPort(data->clip_port);

		// Close timer
		FreeTimer(data->drag.timer);

		// Free data
		FreeVec(data);
	}

	// Forbid at this point
	Forbid();
}

// Show clipboard
void config_buttons_show_clipboard(config_buttons_data *data)
{
	// Already open?
	if (data->clip_window)
		return;

	// Fill in new window for clipboard
	data->newwin.parent = data->window;
	data->newwin.title = GetString(locale, MSG_BUTTONS_CLIPBOARD);
	data->newwin.flags = WINDOW_VISITOR | WINDOW_REQ_FILL | WINDOW_AUTO_KEYS | WINDOW_NO_ACTIVATE;

	// Initial position
	data->win_dims = _config_buttons_clip_window;

	// Stored position valid?
	if (data->clip_pos_valid)
	{
		data->win_dims.char_dim.Left = 0;
		data->win_dims.char_dim.Top = 0;
		data->win_dims.fine_dim.Left = data->clip_window_pos.Left;
		data->win_dims.fine_dim.Top = data->clip_window_pos.Top;
	}

	// Open clipboard window and add objects
	if ((data->clip_window = OpenConfigWindow(&data->newwin)))
	{
		// Add objects
		if (!(data->clip_objlist = AddObjectList(data->clip_window, _config_buttons_clip_objects)))
		{
			CloseConfigWindow(data->clip_window);
			data->clip_window = 0;
		}
		else
		{
			Cfg_Button *button;

			// Go through buttons
			for (button = (Cfg_Button *)data->clip_list.lh_Head; button->node.ln_Succ;
				 button = (Cfg_Button *)button->node.ln_Succ)
			{
				Cfg_ButtonFunction *func;

				// Get first function
				func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

				// Remap if an image
				if (func->node.ln_Succ && func->image)
					RemapImage(func->image, data->window->WScreen, &data->remap);
			}

			// Get clipboard scroller gadget
			data->clip_scroller = GADGET(GetObject(data->clip_objlist, GAD_BUTTONS_CLIPBOARD_SCROLLER));

			// Set window ID
			SetWindowID(data->clip_window, 0, WINDOW_BUTTON_CONFIG, (struct MsgPort *)data->ipc);

			// Display the clipboard
			data->last_clip_draw = 0;
			show_button_clipboard(data, 1);

			// Store window pointer for dragging
			data->drag.window = data->clip_window;
		}
	}
}

// Hide clipboard
void config_buttons_hide_clipboard(config_buttons_data *data)
{
	// Open?
	if (data->clip_window)
	{
		Cfg_Button *button;

		// Go through buttons
		for (button = (Cfg_Button *)data->clip_list.lh_Head; button->node.ln_Succ;
			 button = (Cfg_Button *)button->node.ln_Succ)
		{
			Cfg_ButtonFunction *func;

			// Get first function
			func = (Cfg_ButtonFunction *)button->function_list.mlh_Head;

			// Free remap if an image
			if (func->node.ln_Succ && func->image)
				FreeRemapImage(func->image, &data->remap);
		}

		// Free image remapping
		FreeImageRemap(&data->remap);

		// Save position
		data->clip_window_pos = *((struct IBox *)&data->clip_window->LeftEdge);
		data->clip_pos_valid = 1;

		// Clear windoww
		CloseConfigWindow(data->clip_window);
		data->clip_window = 0;
		data->clip_objlist = 0;
		data->clip_valid = 0;
	}
}

// Edit a new bank
void _config_buttons_new_bank(config_buttons_data *data, Cfg_ButtonBank *bank, IPCData *button_ipc)
{
	bank_node *node = 0;
	ULONG flags = 0;

	// Are we currently editing a bank?
	if (data->bank_node)
	{
		// Same as this one?
		if (data->bank_node->bank == bank)
			return;

		// Say goodbye to old bank
		IPC_Command(data->bank_node->button_ipc, BUTTONEDIT_GOODBYE, data->change, 0, 0, REPLY_NO_PORT);
		data->bank_node = 0;
	}

	// Valid bank?
	if (bank)
	{
		// See if bank is already known to us
		for (node = (bank_node *)data->bank_list.lh_Head; node->node.ln_Succ; node = (bank_node *)node->node.ln_Succ)
		{
			// Match the bank pointer?
			if (bank == node->bank)
				break;
		}

		// Did we not match?
		if (!node->node.ln_Succ)
		{
			// Create a new node
			if ((node = AllocVec(sizeof(bank_node), MEMF_CLEAR)))
			{
				// Initialise node process list
				InitListLock(&node->proc_list, 0);

				// Store bank pointer
				node->bank = bank;
				node->button_ipc = button_ipc;

				// Toolbar?
				if (node->bank->window.flags & BTNWF_TOOLBAR)
				{
					// Set flag
					node->toolbar = 1;
					node->bank->window.flags &= ~BTNWF_TOOLBAR;
				}

				// Lock bank
				GetSemaphore(&bank->lock, SEMF_SHARED, 0);

				// Make a backup of the bank
				node->bank_backup = CopyButtonBank(bank);

				// Unlock bank
				FreeSemaphore(&bank->lock);

				// Add node to list of banks
				AddTail(&data->bank_list, &node->node);
			}
		}
	}

	// Store this as the current bank node
	data->bank_node = node;

	// Window open?
	if (data->window)
	{
		// Disable/enable font and picture gadgets
		DisableObject(
			data->objlist, GAD_BUTTONS_FONT_GLASS, (!bank || (bank->window.flags & BTNWF_GFX) || !data->font_req));
		DisableObject(data->objlist, GAD_BUTTONS_FONT_NAME, (!bank || bank->window.flags & BTNWF_GFX));
		DisableObject(data->objlist, GAD_BUTTONS_FONT_SIZE, (!bank || bank->window.flags & BTNWF_GFX));
		DisableObject(data->objlist, GAD_BUTTONS_BACKPIC_POPUP, !bank);
		DisableObject(data->objlist, GAD_BUTTONS_BACKPIC, !bank);
	}

	// Do we have a valid node?
	if (node)
	{
		// Say hello to new bank
		IPC_Command(node->button_ipc, BUTTONEDIT_HELLO, 0, (APTR)data->ipc, 0, REPLY_NO_PORT);
	}

	// Otherwise, disable the other gadgets
	if (data->window)
	{
		DisableObject(data->objlist, GAD_BUTTONS_NAME, (BOOL)!node);
		DisableObject(data->objlist, GAD_BUTTONS_COLUMNS_ADD, (BOOL)!node);
		DisableObject(data->objlist, GAD_BUTTONS_COLUMNS_REMOVE, (BOOL) !(node && !node->toolbar));
		DisableObject(data->objlist, GAD_BUTTONS_ROWS_ADD, (BOOL) !(node && !node->toolbar));
		DisableObject(data->objlist, GAD_BUTTONS_ROWS_REMOVE, (BOOL) !(node && !node->toolbar));
	}

	// Set menu for normal or toolbar buttons
	AddWindowMenus(data->window, (node && node->toolbar) ? button_toolbar_menu : button_menu);

	// Update display
	_config_buttons_update(data);

	// Nothing currently selected
	data->select_col = -1;
	data->select_row = -1;
	_config_buttons_fix_controls(data);

	// Get flags value
	if (data->bank_node)
		flags = data->bank_node->bank->window.flags;

	// Initialise appearance controls
	SetGadgetValue(data->objlist, GAD_BUTTONS_BANK_BORDER, !(flags & BTNWF_NO_BORDER));
	SetGadgetValue(data->objlist, GAD_BUTTONS_BORDERLESS, flags & BTNWF_BORDERLESS);
	SetGadgetValue(data->objlist, GAD_BUTTONS_SIMPLE_REFRESH, !(flags & BTNWF_SMART_REFRESH));
	SetGadgetValue(data->objlist, GAD_BUTTONS_NO_DOGEARS, flags & BTNWF_NO_DOGEARS);
	SetGadgetValue(data->objlist, GAD_BUTTONS_AUTO_CLOSE, flags & BTNWF_AUTO_CLOSE);
	SetGadgetValue(data->objlist, GAD_BUTTONS_AUTO_ICONIFY, flags & BTNWF_AUTO_ICONIFY);
	SetGadgetValue(data->objlist, GAD_BUTTONS_ACTIVE_POPUP, flags & BTNWF_ACTIVE_POPUP);
	SetGadgetValue(data->objlist,
				   GAD_BUTTONS_DRAGBAR_ORIENTATION,
				   (flags & BTNWF_HORIZ)
					   ? ((flags & BTNWF_RIGHT_BELOW) ? 3 : 1)
					   : (flags & BTNWF_VERT) ? ((flags & BTNWF_RIGHT_BELOW) ? 4 : 2) : (flags & BTNWF_NONE) ? 5 : 0);

	// Disable gadgets
	DisableObject(data->objlist, GAD_BUTTONS_BANK_BORDER, !(data->bank_node && !data->bank_node->toolbar));
	DisableObject(data->objlist, GAD_BUTTONS_BORDERLESS, !data->bank_node);
	DisableObject(data->objlist, GAD_BUTTONS_SIMPLE_REFRESH, !(data->bank_node && !data->bank_node->toolbar));
	DisableObject(data->objlist, GAD_BUTTONS_NO_DOGEARS, !(data->bank_node && !data->bank_node->toolbar));
	DisableObject(data->objlist, GAD_BUTTONS_AUTO_CLOSE, !(data->bank_node && !data->bank_node->toolbar));
	DisableObject(data->objlist, GAD_BUTTONS_AUTO_ICONIFY, !(data->bank_node && !data->bank_node->toolbar));
	DisableObject(data->objlist, GAD_BUTTONS_ACTIVE_POPUP, !(data->bank_node && !data->bank_node->toolbar));
	DisableObject(data->objlist,
				  GAD_BUTTONS_DRAGBAR_ORIENTATION,
				  (!data->bank_node || data->bank_node->toolbar || !(flags & BTNWF_NO_BORDER)));
}

// Remove a bank from the edit list
void _config_buttons_remove_bank(config_buttons_data *data, bank_node *bank, BOOL undo_flag, BOOL quit_flag)
{
	// If there's a quit message, reply it
	if (bank->quit_msg)
		IPC_Reply(bank->quit_msg);

	// Otherwise, say goodbye
	else if (!quit_flag)
	{
		BOOL change;

		// Get change flag state
		change = data->change;

		// Do undo?
		if (undo_flag && bank->bank_backup)
		{
			// Give the main process the new bank pointer
			IPC_Command(bank->button_ipc, BUTTONEDIT_NEW_BANK, 1, (APTR)bank->bank_backup, 0, REPLY_NO_PORT);

			// Replace backup pointer with old bank pointer
			bank->bank_backup = bank->bank;
			change = 0;
		}

		// Say goodbye
		IPC_Command(bank->button_ipc, BUTTONEDIT_GOODBYE, change, 0, 0, 0);
	}

	// Free backup bank
	CloseButtonBank(bank->bank_backup);

	// Remove from bank list
	Remove(&bank->node);

	// Is this our current bank?
	if (data->bank_node == bank)
	{
		data->bank_node = 0;
		_config_buttons_new_bank(data, 0, 0);
		data->change = 0;
	}

	// Free node entry
	FreeVec(bank);
}

// Update display
void _config_buttons_update(config_buttons_data *data)
{
	// Window not open?
	if (!data->window)
		return;

	// Valid current bank?
	if (data->bank_node)
	{
		// Lock bank
		GetSemaphore(&data->bank_node->bank->lock, SEMF_SHARED, 0);

		// Initialise name field
		SetGadgetValue(data->objlist, GAD_BUTTONS_NAME, (IPTR)data->bank_node->bank->window.name);

		// Update column display
		SetGadgetValue(data->objlist, GAD_BUTTONS_COLUMNS, data->bank_node->bank->window.columns);

		// If only 1 column or a toolbar, disable remove button
		DisableObject(data->objlist,
					  GAD_BUTTONS_COLUMNS_REMOVE,
					  (data->bank_node->toolbar || data->bank_node->bank->window.columns < 2));

		// Update row display
		SetGadgetValue(data->objlist, GAD_BUTTONS_ROWS, data->bank_node->bank->window.rows);

		// If only 1 row, disable remove button
		DisableObject(data->objlist,
					  GAD_BUTTONS_ROWS_REMOVE,
					  (data->bank_node->toolbar || data->bank_node->bank->window.rows < 2));

		// Initialise font
		SetGadgetValue(data->objlist, GAD_BUTTONS_FONT_NAME, (IPTR)data->bank_node->bank->window.font_name);
		SetGadgetValue(data->objlist, GAD_BUTTONS_FONT_SIZE, (ULONG)data->bank_node->bank->window.font_size);

		// Initialise picture
		SetGadgetValue(data->objlist, GAD_BUTTONS_BACKPIC, (IPTR)data->bank_node->bank->backpic);

		// Unlock bank
		FreeSemaphore(&data->bank_node->bank->lock);
	}

	// Otherwise clear fields
	else
	{
		SetGadgetValue(data->objlist, GAD_BUTTONS_NAME, 0);
		SetGadgetValue(data->objlist, GAD_BUTTONS_COLUMNS, 0);
		SetGadgetValue(data->objlist, GAD_BUTTONS_ROWS, 0);
		SetGadgetValue(data->objlist, GAD_BUTTONS_FONT_NAME, 0);
		SetGadgetValue(data->objlist, GAD_BUTTONS_FONT_SIZE, 0);
		SetGadgetValue(data->objlist, GAD_BUTTONS_BACKPIC, 0);
	}
}

// Tell button window to refresh
void _config_buttons_refresh(config_buttons_data *data, bank_node *bank, ULONG type)
{
	if (bank)
		IPC_Command(bank->button_ipc, BUTTONEDIT_REFRESH, type, 0, 0, 0);
}

// Reset after adding/removing rows/columns
void _config_buttons_reset(config_buttons_data *data)
{
	// Update local display
	_config_buttons_update(data);

	// Stop flash
	_config_buttons_flash(data, 0);

	// Send refresh to button process
	_config_buttons_refresh(data, data->bank_node, BUTREFRESH_RESIZE | BUTREFRESH_REFRESH);

	// Bounds check selection
	if (data->select_col >= data->bank_node->bank->window.columns ||
		data->select_row >= data->bank_node->bank->window.rows)
	{
		// No selection
		data->select_col = -1;
		data->select_row = -1;
		_config_buttons_flash(data, -1);
	}

	// Restart flash
	else
		_config_buttons_flash(data, 1);

	// Fix button disable
	_config_buttons_fix_controls(data);
}
