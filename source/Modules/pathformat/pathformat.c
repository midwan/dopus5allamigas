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

#include "pathformat.h"
#include <Program/position.h>
#include <dopus/lib_macros.h>

/*#ifdef __amigaos4__
struct CommoditiesIFace *ICommodities;
#endif*/

int LIBFUNC L_Module_Entry(REG(a0, struct List *files),
						   REG(a1, struct Screen *screen),
						   REG(a2, IPCData *ipc),
						   REG(a3, IPCData *main_ipc),
						   REG(d0, IPTR mod_id),
						   REG(d1, IPTR mod_data))
{
	config_path_data *data;
	short success = 1;

	// Allocate data
	if (!(data = AllocVec(sizeof(config_path_data), MEMF_CLEAR)) || !(data->path_list = Att_NewList(0)))
	{
		FreeVec(data);
		return 0;
	}

	// Store data pointers
	data->paths = files;
	data->ipc = ipc;
	data->main_ipc = main_ipc;
	data->memory = (APTR)(IPTR)mod_id;
	data->def_format = (ListFormat *)(IPTR)mod_data;

	// AppPort
	data->appport = CreateMsgPort();

	// Open commodities library
	/*data->cxbase=OpenLibrary("commodities.library",0);
	#ifdef __amigaos4__
	ICommodities=(struct CommoditiesIFace *)GetInterface(data->cxbase,"main",1,NULL);
	#endif	*/

	// Build path list
	config_paths_build_list(data);

	// Open window
	if (!(config_paths_open(data, screen)))
		return 0;

	// Event loop
	FOREVER
	{
		IPCMessage *imsg;
		struct IntuiMessage *msg;
		int quit_flag = 0;

		// IPC messages?
		while ((imsg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Quit?
			if (imsg->command == IPC_QUIT)
			{
				quit_flag = 1;
				success = 0;
			}

			// Hide?
			else if (imsg->command == IPC_HIDE)
			{
				RemoveAppWindow(data->appwindow);
				data->appwindow = 0;
				CloseConfigWindow(data->window);
				data->window = 0;
			}

			// Show?
			else if (imsg->command == IPC_SHOW)
			{
				if (!(config_paths_open(data, imsg->data)))
				{
					quit_flag = 1;
					success = 0;
				}
			}

			// Activate
			else if (imsg->command == IPC_ACTIVATE)
			{
				if (data->window)
				{
					WindowToFront(data->window);
					ActivateWindow(data->window);
				}
			}

			// Reply
			IPC_Reply(imsg);
		}

		// Any messages to the window?
		if (data->window)
		{
			while ((msg = GetWindowMsg(data->window->UserPort)))
			{
				struct IntuiMessage msg_copy;
				UWORD gadgetid;

				// Copy message and reply
				msg_copy = *msg;
				ReplyWindowMsg(msg);

				// Look at message
				switch (msg_copy.Class)
				{
				// Close window
				case IDCMP_CLOSEWINDOW:
					quit_flag = 1;
					success = 0;
					break;

				// Gadget
				case IDCMP_GADGETUP:
				case IDCMP_GADGETDOWN:

					// Get GadgetID
					gadgetid = ((struct Gadget *)msg_copy.IAddress)->GadgetID;

					// Look at gadget
					switch (gadgetid)
					{
					// Cancel
					case GAD_PATHFORMAT_CANCEL:
						success = 0;

					// Use
					case GAD_PATHFORMAT_SAVE:
						quit_flag = 1;
						break;

					// Add an entry
					case GAD_PATHFORMAT_ADD:
						config_paths_add(data, TRUE);
						break;

					// Change path name
					case GAD_PATHFORMAT_PATH:
						config_paths_change(data);
						break;

					// Select an entry
					case GAD_PATHFORMAT_PATHS: {
						Att_Node *node;

						// Find selected entry
						node = Att_FindNode(data->path_list, msg_copy.Code);

						// Double-click?
						if (DoubleClick(data->seconds, data->micros, msg_copy.Seconds, msg_copy.Micros) &&
							node == data->path_sel)
						{
							// Edit it
							config_paths_edit(data);
							break;
						}

						// Store double-click time
						data->seconds = msg_copy.Seconds;
						data->micros = msg_copy.Micros;

						// Not already selected?
						if (node != data->path_sel)
						{
							// Select it
							config_paths_select(data, node, FALSE);
						}
					}
					break;

					// Key
					case GAD_PATHFORMAT_KEY:
						config_paths_get_key(data);
						break;

					// Open new lister
					case GAD_PATHFORMAT_NEW_LISTER:

						// Valid selection?
						if (data->path_sel)
						{
							position_rec *pos = (position_rec *)data->path_sel->data;

							// Open new lister?
							if (msg_copy.Code)
							{
								// Set flag
								pos->flags |= POSITIONF_OPEN_NEW;

								// Clear type flags
								pos->flags &= ~(POSITIONF_ICON | POSITIONF_ICON_ACTION);

								// Icon action?
								if (msg_copy.Code == MODE_ACTION)
									pos->flags |= POSITIONF_ICON | POSITIONF_ICON_ACTION;

								// Icon?
								else if (msg_copy.Code == MODE_ICON)
									pos->flags |= POSITIONF_ICON;
							}

							// No lister
							else
								pos->flags &= ~POSITIONF_OPEN_NEW;
						}
						break;

					// Remove
					case GAD_PATHFORMAT_REMOVE:
						config_paths_remove(data, TRUE);
						break;

					// Edit
					case GAD_PATHFORMAT_EDIT:
						config_paths_edit(data);
						break;
					}
					break;

				// Key press
				case IDCMP_RAWKEY:

					// Help?
					if (msg_copy.Code == 0x5f && !(msg_copy.Qualifier & VALID_QUALIFIERS))
					{
						// Valid main IPC?
						if (main_ipc)
						{
							// Set busy pointer
							SetWindowBusy(data->window);

							// Send help request
							IPC_Command(main_ipc, IPC_HELP, (1 << 31), "Paths", 0, (struct MsgPort *)-1);

							// Clear busy pointer
							ClearWindowBusy(data->window);
						}
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
			break;

		// App messages?
		if (data->appport)
		{
			struct AppMessage *msg;

			// Get messages
			while ((msg = (struct AppMessage *)GetMsg(data->appport)))
			{
				// Get first argument
				if (msg->am_NumArgs > 0)
				{
					// Is it a directory?
					if (!msg->am_ArgList[0].wa_Name || !*msg->am_ArgList[0].wa_Name)
					{
						char buf[256];

						// Expand path name
						if (NameFromLock(msg->am_ArgList[0].wa_Lock, buf, 256))
						{
							Att_Node *node;

							// Fix trailing /
							AddPart(buf, "", 256);

							// Check it's not already in the list
							if (!(node = (Att_Node *)FindNameI((struct List *)data->path_list, buf)))
							{
								// Add a new entry
								config_paths_add(data, FALSE);

								// Copy path to path field
								SetGadgetValue(data->objlist, GAD_PATHFORMAT_PATH, (IPTR)buf);

								// Accept the new path
								config_paths_change(data);
							}

							// It is, select this entry
							else
								config_paths_select(data, node, FALSE);
						}
					}
				}

				// Reply message
				ReplyMsg((struct Message *)msg);
			}
		}

		// Wait for input
		Wait(((data->window) ? (1 << data->window->UserPort->mp_SigBit) : 0) |
			 ((data->appport) ? (1 << data->appport->mp_SigBit) : 0) | 1 << ipc->command_port->mp_SigBit);
	}

	// Free stuff
	config_paths_cleanup(data);
	return success;
}

// Open window
BOOL config_paths_open(config_path_data *data, struct Screen *screen)
{
	// Already open?
	if (data->window)
		return 1;

	// Fill out and open window
	data->newwin.parent = screen;
	data->newwin.dims = &pathformat_window;
	data->newwin.title = GetString(locale, MSG_PATHFORMAT_TITLE);
	data->newwin.locale = locale;
	data->newwin.flags = WINDOW_VISITOR | WINDOW_AUTO_KEYS | WINDOW_REQ_FILL | WINDOW_SCREEN_PARENT;
	if (!(data->window = OpenConfigWindow(&data->newwin)) ||
		!(data->objlist = AddObjectList(data->window, pathformat_objects)))
	{
		config_paths_cleanup(data);
		return 0;
	}

	// Add AppWindow
	if (data->appport)
		data->appwindow = AddAppWindowA(0, 0, data->window, data->appport, 0);

	// Attach path list
	SetGadgetChoices(data->objlist, GAD_PATHFORMAT_PATHS, data->path_list);

	// Initially disable everything
	config_paths_disable(data, TRUE);
	return 1;
}

// Clean up
void config_paths_cleanup(config_path_data *data)
{
	if (data)
	{
		// AppWindow
		RemoveAppWindow(data->appwindow);

		// Close window
		CloseConfigWindow(data->window);

		// Free list
		Att_RemList(data->path_list, 0);

		// Close cx library
		/*#ifdef __amigaos4__
		DropInterface((struct Interface *)ICommodities);
		#endif
		CloseLibrary(data->cxbase);*/

		// Free port
		if (data->appport)
		{
			struct Message *msg;

			// Flush it
			while ((msg = GetMsg(data->appport)))
				ReplyMsg(msg);

			// Delete it
			DeleteMsgPort(data->appport);
		}

		// Free data
		FreeVec(data);
	}
}

// Build path list
void config_paths_build_list(config_path_data *data)
{
	position_rec *pos;

	// Go through position list
	for (pos = (position_rec *)data->paths->lh_Head; pos->node.ln_Succ; pos = (position_rec *)pos->node.ln_Succ)
	{
		// User-edited path?
		if (pos->node.ln_Type == PTYPE_POSITION && pos->flags & POSITIONF_USER)
		{
			// Add to our list
			Att_NewNode(data->path_list, pos->node.ln_Name, (IPTR)pos, ADDNODE_SORT);
		}
	}
}

// Disable gadgets
void config_paths_disable(config_path_data *data, BOOL state)
{
	// Disable editing gadgets
	DisableObject(data->objlist, GAD_PATHFORMAT_KEY, state);
	DisableObject(data->objlist, GAD_PATHFORMAT_REMOVE, state);
	DisableObject(data->objlist, GAD_PATHFORMAT_EDIT, state);
	DisableObject(data->objlist, GAD_PATHFORMAT_PATH_FOLDER, state);
	DisableObject(data->objlist, GAD_PATHFORMAT_PATH, state);
	DisableObject(data->objlist, GAD_PATHFORMAT_NEW_LISTER, state);

	// Disable lister if it's empty
	DisableObject(data->objlist, GAD_PATHFORMAT_PATHS, (IsListEmpty((struct List *)data->path_list)));
}

// Add an entry
void config_paths_add(config_path_data *data, BOOL activate)
{
	position_rec *pos;
	Att_Node *node;

	// Create a new entry
	if ((pos = (position_rec *)AllocMemH(data->memory, sizeof(position_rec) + 256)))
	{
		// Initialise entry
		pos->node.ln_Type = PTYPE_POSITION;
		pos->node.ln_Name = pos->name;
		pos->flags |= POSITIONF_USER | POSITIONF_NEW;
		if (data->def_format)
			CopyMem((char *)data->def_format, (char *)&pos->format, sizeof(ListFormatStorage));

		// Initialise key
		pos->code = 0xffff;

		// Add to list path list
		AddTail(data->paths, (struct Node *)pos);

		// Remove list from gadget
		SetGadgetChoices(data->objlist, GAD_PATHFORMAT_PATHS, (APTR)~0);

		// Add to lister
		node = Att_NewNode(data->path_list, 0, (IPTR)pos, ADDNODE_SORT);

		// Attach list to gadget
		SetGadgetChoices(data->objlist, GAD_PATHFORMAT_PATHS, data->path_list);

		// Select the new node
		config_paths_select(data, node, TRUE);

		// Activate path field
		if (activate)
			ActivateStrGad(GADGET(GetObject(data->objlist, GAD_PATHFORMAT_PATH)), data->window);
	}
}

// Change entry's path
void config_paths_change(config_path_data *data)
{
	char *str;
	BPTR lock;
	char path[256];
	position_rec *pos;
	Att_Node *node;
	struct List *search;

	// No path selected?
	if (!data->path_sel)
		return;

	// Get path string
	str = (char *)GetGadgetValue(data->objlist, GAD_PATHFORMAT_PATH);

	// Invalid?
	if (!str || !*str)
	{
		// Remove this path
		config_paths_remove(data, TRUE);
		return;
	}

	// Try to lock
	if ((lock = Lock(str, ACCESS_READ)))
	{
		// Get full pathname
		NameFromLock(lock, path, 256);
		UnLock(lock);
	}

	// Otherwise, use entered path
	else
		strcpy(path, str);

	// Fix path with trailing /
	AddPart(path, "", 256);

	// See if path is already in edit list
	if ((node = (Att_Node *)FindNameI((struct List *)data->path_list, path)))
	{
		// Not the currently selected node?
		if (node != data->path_sel)
		{
			// Error
			DisplayBeep(data->window->WScreen);
			ActivateStrGad(GADGET(GetObject(data->objlist, GAD_PATHFORMAT_PATH)), data->window);
			return;
		}
	}

	// Is path already in snapshot list?
	search = data->paths;
	while ((pos = (position_rec *)FindNameI(search, path)))
	{
		// Position entry?
		if (pos->node.ln_Type == PTYPE_POSITION)
			break;
		search = (struct List *)pos;
	}

	// Found it?
	if (pos)
	{
		// Different entry?
		if (pos != (position_rec *)data->path_sel->data)
		{
			// Remove current entry from list
			Remove((struct Node *)data->path_sel->data);

			// Free current data
			FreeMemH((void *)data->path_sel->data);
			data->path_sel->data = (IPTR)pos;
		}

		// If path exists, clear "new" flag
		if (lock)
			pos->flags &= ~POSITIONF_NEW;
	}

	// It is now
	else
		pos = (position_rec *)data->path_sel->data;

	// Set its user flag
	pos->flags |= POSITIONF_USER;

	// Store new name
	strcpy(pos->name, path);

	// Remove list from gadget
	SetGadgetChoices(data->objlist, GAD_PATHFORMAT_PATHS, (APTR)~0);

	// Remove existing node and add new one
	Att_RemNode(data->path_sel);

	// Add to list
	data->path_sel = Att_NewNode(data->path_list, pos->node.ln_Name, (IPTR)pos, ADDNODE_SORT);

	// Attach list to gadget
	SetGadgetChoices(data->objlist, GAD_PATHFORMAT_PATHS, data->path_list);

	// Select the new node
	config_paths_select(data, data->path_sel, TRUE);
}

// Select a node
void config_paths_select(config_path_data *data, Att_Node *node, BOOL new)
{
	long num;
	char buf[80];

	// Path selected?
	if (data->path_sel)
	{
		// Is current path empty?
		if (!((position_rec *)data->path_sel->data)->name[0])
		{
			// Remove it
			config_paths_remove(data, FALSE);
		}
	}

	// Find number
	num = Att_FindNodeNumber(data->path_list, node);

	// Fix selection if necessary
	if (new || GetGadgetValue(data->objlist, GAD_PATHFORMAT_PATHS) != num)
		SetGadgetValue(data->objlist, GAD_PATHFORMAT_PATHS, num);

	// Valid node?
	if (num > -1)
	{
		position_rec *pos = (position_rec *)node->data;
		short mode;

		// Get key string
		BuildKeyString(pos->code, pos->qual, pos->qual_mask, pos->qual_same, buf);

		// Fill out gadgets
		SetGadgetValue(data->objlist, GAD_PATHFORMAT_PATH, (IPTR)node->node.ln_Name);
		SetGadgetValue(data->objlist, GAD_PATHFORMAT_KEY, (IPTR)buf);

		// Get mode
		if (!(pos->flags & POSITIONF_OPEN_NEW))
			mode = MODE_NONE;
		else if (pos->flags & POSITIONF_ICON_ACTION)
			mode = MODE_ACTION;
		else if (pos->flags & POSITIONF_ICON)
			mode = MODE_ICON;
		else
			mode = MODE_NAME;

		// Set mode
		SetGadgetValue(data->objlist, GAD_PATHFORMAT_NEW_LISTER, mode);

		// Enable gadgets
		config_paths_disable(data, FALSE);
	}

	// Clear and disable gadgets
	else
	{
		SetGadgetValue(data->objlist, GAD_PATHFORMAT_PATH, 0);
		SetGadgetValue(data->objlist, GAD_PATHFORMAT_KEY, 0);
		SetGadgetValue(data->objlist, GAD_PATHFORMAT_NEW_LISTER, MODE_NONE);
		config_paths_disable(data, TRUE);
	}

	// Store selection
	data->path_sel = node;
}

// Get key code
void config_paths_get_key(config_path_data *data)
{
	// Valid selection?
	if (data->path_sel && CxBase)
	{
		char *key;
		position_rec *pos;
		IX ix;

		// Current position
		pos = (position_rec *)data->path_sel->data;

		// Get key string
		key = (char *)GetGadgetValue(data->objlist, GAD_PATHFORMAT_KEY);

		// Empty?
		if (!key || !*key)
			pos->code = 0xffff;

		// Parse IX
		else if (!(ParseIX(key, &ix)))
		{
			// Store key codes
			pos->code = ix.ix_Code;
			pos->qual = QualValid(ix.ix_Qualifier);
			pos->qual_mask = ix.ix_QualMask;
			pos->qual_same = ix.ix_QualSame;
		}

		// Invalid
		else
		{
			DisplayBeep(data->window->WScreen);
			ActivateStrGad(GADGET(GetObject(data->objlist, GAD_PATHFORMAT_KEY)), data->window);
			return;
		}
	}
}

// Remove a path
void config_paths_remove(config_path_data *data, BOOL disable)
{
	// Valid selection?
	if (data->path_sel)
	{
		position_rec *pos = (position_rec *)data->path_sel->data;

		// Remove list from gadget
		SetGadgetChoices(data->objlist, GAD_PATHFORMAT_PATHS, (APTR)~0);

		// Remove node from list
		Att_RemNode(data->path_sel);

		// Attach list to gadget
		SetGadgetChoices(data->objlist, GAD_PATHFORMAT_PATHS, data->path_list);

		// Was it completely user-defined?
		if (pos->flags & POSITIONF_NEW)
		{
			// Remove from path list
			Remove((struct Node *)pos);

			// Free entry
			FreeMemH(pos);
		}

		// Otherwise, clear user flag
		else
			pos->flags &= ~(POSITIONF_USER | POSITIONF_FORMAT);

		// No node selected
		data->path_sel = 0;

		// Select no node
		if (disable)
			config_paths_select(data, 0, FALSE);
	}
}

// Edit path format
void config_paths_edit(config_path_data *data)
{
	// Valid selection?
	if (data->path_sel)
	{
		position_rec *pos = (position_rec *)data->path_sel->data;
		struct Library *ModuleBase = NULL;
#ifdef __amigaos4__
		struct ModuleIFace *IModule = NULL;
#endif
		ListFormat format;
		short ret = 0;

		// Make window busy
		SetWindowBusy(data->window);

		// Get lister format module
		if ((ModuleBase = OpenLibrary("dopus5:modules/listerformat.module", LIB_VERSION)) &&
			GETINTERFACE(IModule, ModuleBase))
		{
			// Convert ListFormatStorage to ListFormat
			CopyMem((char *)&pos->format, (char *)&format, sizeof(ListFormatStorage));

			// Edit format
			ret = Module_Entry((struct List *)&format, (struct Screen *)data->window, data->ipc, data->main_ipc, 0, 0);

#ifdef __amigaos4__
			DropInterface((struct Interface *)IModule);
#endif
			CloseLibrary(ModuleBase);
		}
		else
			CloseLibrary(ModuleBase);
		DisplayBeep(data->window->WScreen);

		// Convert back if successful
		if (ret)
		{
			CopyMem((char *)&format, (char *)&pos->format, sizeof(ListFormatStorage));
			pos->flags |= POSITIONF_FORMAT;
		}

		// Make window unbusy
		ClearWindowBusy(data->window);
	}
}
