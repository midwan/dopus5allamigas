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

// Open a backdrop object
void backdrop_object_open(BackdropInfo *info,
						  BackdropObject *object,
						  UWORD qual,
						  BOOL activate,
						  long numargs,
						  struct WBArg *arglist)
{
	Lister *lister;
	IPCData *ipc = 0;
	BPTR old, lock;

	// Valid object?
	if (!object || !object->icon)
		return;

	// Is object a group?
	if (object->type == BDO_GROUP)
	{
		// Open group
		backdrop_open_group(info, object, 1);
		return;
	}

	// Bad disk?
	if (object->type == BDO_BAD_DISK)
	{
		// Error
		DisplayBeep(info->window->WScreen);
		return;
	}

	// Is object an appicon?
	if (object->type == BDO_APP_ICON)
	{
		DOpusAppMessage *msg;
		struct MsgPort *port;

		// Is icon busy?
		if (object->flags & BDOF_BUSY)
		{
			DisplayBeep(info->window->WScreen);
			return;
		}

		// Allocate AppMessage
		if (!(msg = AllocAppMessage(global_memory_pool, GUI->appmsg_port, 0)))
			return;

		// Message type
		msg->da_Msg.am_Type = MTYPE_APPICON;

		// Get AppInfo
		port = WB_AppWindowData((struct AppWindow *)object->misc_data, &msg->da_Msg.am_ID, &msg->da_Msg.am_UserData);

		// Send the message
		PutMsg(port, (struct Message *)msg);
		return;
	}

	// If we're not trying to start a tool, see if there is one selected
	if (object->icon->do_Type != WBTOOL && info->first_sel_tool && info->first_sel_tool != object)
	{
		// Check that it's valid
		if (find_backdrop_object(info, info->first_sel_tool))
		{
			// Use this tool to open the projects
			object = info->first_sel_tool;
		}
	}

	// Get icon lock
	if (!(lock = backdrop_icon_lock(object)))
		return;

	// Change current directory
	old = CurrentDir(lock);

	// Directory to read?
	if (object->icon->do_Type == WBDISK || object->icon->do_Type == WBDRAWER || object->icon->do_Type == WBGARBAGE)
	{
		char path[256];

		// Get path
		DevNameFromLockDopus(lock, path, 256);

		// Drawer to read?
		if (object->icon->do_Type == WBDRAWER || object->icon->do_Type == WBGARBAGE)
		{
			// Add to path
			AddPart(path, object->name, 256);
			AddPart(path, "", 256);
		}

		// If shift isn't down, look for already open
		if (!(qual & IEQUAL_ANYSHIFT))
		{
			// Lock lister list
			lock_listlock(&GUI->lister_list, FALSE);

			// Go through listers
			for (ipc = (IPCData *)GUI->lister_list.list.lh_Head; ipc->node.mln_Succ;
				 ipc = (IPCData *)ipc->node.mln_Succ)
			{
				// Get lister
				lister = IPCDATA(ipc);

				// Is this lister what we're after?
				if (stricmp(lister->cur_buffer->buf_Path, path) == 0 && !(lister->flags & LISTERF_BUSY))
				{
					// Activate this window
					IPC_Command(ipc, IPC_ACTIVATE, 0, (APTR)1, 0, 0);
					break;
				}
			}

			// Unlock lister list
			unlock_listlock(&GUI->lister_list);

			// Get one?
			if (!ipc->node.mln_Succ)
				ipc = 0;
		}

		// Open new lister?
		if (!ipc)
		{
			BPTR lock;

			// Lock path
			if (!(lock = Lock(path, ACCESS_READ)))
			{
				// Can't open
				DisplayBeep(info->window->WScreen);
			}

			// Ok to open
			else
			{
				// Unlock path lock
				UnLock(lock);

				// Read into existing lister in icon action mode if control is down
				if (info->lister && info->lister->flags & LISTERF_ICON_ACTION && qual & IEQUALIFIER_CONTROL)
				{
					// Read into existing lister
					read_directory(info->lister, path, GETDIRF_CANMOVEEMPTY | GETDIRF_CANCHECKBUFS);
				}

				// Open lister for this path
				else if ((lister = lister_open_new(path, object, info->window, info->lister)))
				{
					ULONG init_flags;

					init_flags = (activate) ? LISTERF_ACTIVATE : 0;
					if (environment->env->lister_options & LISTEROPTF_DUAL_DEFAULT)
						init_flags |= LISTERF_DUAL;

					// Initialise lister
					IPC_Command(lister->ipc, LISTER_INIT, init_flags, GUI->screen_pointer, 0, 0);

					// Close parent?
					if (qual & IEQUALIFIER_LALT)
					{
						// Got a lister?
						if (info->lister)
							IPC_Command(info->lister->ipc, IPC_QUIT, 0, 0, 0, 0);
					}
				}
			}
		}
	}

	// Something to run?
	else if (object->icon->do_Type == WBTOOL || object->icon->do_Type == WBPROJECT)
	{
		char *default_tool = 0;
		char path[256];
		BOOL execute = 0, ok = 1;
		struct DiskObject *got_icon = 0, *icon;

		// Get icon
		if (object->icon->do_Type == WBPROJECT && (got_icon = GetDiskObject(object->name)))
			icon = got_icon;
		else
			icon = object->icon;

		// Is object a project?
		if (icon->do_Type == WBPROJECT)
		{
			// Valid tool?
			if (icon->do_DefaultTool && *icon->do_DefaultTool)
			{
				// Get default tool
				default_tool = icon->do_DefaultTool;

				// Trap more?
				if (file_trap_more(object->name, default_tool))
					ok = 0;

				// And ignore execute for fake icons
				else if (!got_icon && stricmp(FilePart(default_tool), "execute") == 0)
					default_tool = 0;
			}

			// No default tool?
			if (!default_tool)
			{
				D_S(struct FileInfoBlock, fib)
				BPTR test;

				// Lock and examine file
				if ((test = Lock(object->name, ACCESS_READ)))
				{
					// Examine object
					Examine(test, fib);
					UnLock(test);

					// Is S bit set?
					if (fib->fib_Protection & FIBF_SCRIPT)
					{
						// Execute it
						default_tool = object->name;
						execute = 1;
					}
				}

				// If S bit isn't set, submit to filetypes
				if (!execute)
				{
					short action;

					// Get source path
					DevNameFromLockDopus(lock, path, 256);

					// Get filetype action
					if (qual & IEQUALIFIER_CONTROL)
						action = FTTYPE_CTRL_DOUBLECLICK;
					else if (qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
						action = FTTYPE_ALT_DOUBLECLICK;
					else
						action = FTTYPE_DOUBLE_CLICK;

					// Do filetype action on file
					function_launch(FUNCTION_FILETYPE,
									0,
									action,
									0,
									0,
									0,
									path,
									0,
									BuildArgArray(object->name, 0),
									0,
									(Buttons *)WBArgArray(arglist, numargs, AAF_ALLOW_DIRS));
				}
			}
		}

		// If it's a tool, see if icon exists at all
		else if (icon->do_Type == WBTOOL)
		{
			char buf[256];
			BPTR test;

			// Build icon name
			StrCombine(buf, object->name, ".info", sizeof(buf));

			// Does icon exist?
			if ((test = Lock(buf, ACCESS_READ)))
			{
				// Yep, it's ok
				UnLock(test);
			}

			// No icon
			else
			{
				// Launch proc to run as AmigaDOS
				misc_startup("dopus_run", MENU_EXECUTE, info->window, (APTR)object->name, 0);
				ok = 0;
			}
		}

		// Ok to run?
		if (ok && (icon->do_Type == WBTOOL || default_tool))
		{
			Att_List *command_list;

			// Allocate command list
			if ((command_list = Att_NewList(LISTF_POOL)))
			{
				short len;
				char *launchprog;
				char *command;
				struct Node *node;

				// Program to actually launch
				launchprog = (icon->do_Type == WBTOOL) ? object->name : (char *)FilePart(default_tool);

				// Set screen title
				lsprintf(info->buffer, GetString(&locale, MSG_LAUNCHING_PROGRAM), launchprog);
				title_error(info->buffer, 0);

				// Arguments supplied?
				if (numargs > 0)
				{
					short arg;

					// Go through arguments
					for (arg = 0; arg < numargs; arg++)
					{
						// Get full name
						DevNameFromLockDopus(arglist[arg].wa_Lock, info->buffer, 256);
						AddPart(info->buffer, arglist[arg].wa_Name, 256);

						// Add to command list
						Att_NewNode(command_list, info->buffer, 0, 0);
					}
				}

				// Selected objects as arguments?
				else if (numargs == 0)
				{
					// Get arguments
					backdrop_run_build_args(command_list, info, object);
				}

				// Get total length of arguments
				for (node = command_list->list.lh_Head, len = 0; node->ln_Succ; node = node->ln_Succ)
					len += strlen(node->ln_Name) + 3;

				// Add length of name
				len += strlen(object->name) + 3;

				// Execute?
				if (execute)
					len += 8;

				// Allocate buffer
				if ((command = AllocVec(len, MEMF_ANY)))
				{
					char *ptr;

					// Build command
					if (execute)
						lsprintf(command, "execute \"%s\"", object->name);
					else
						lsprintf(command, "\"%s\"", object->name);

					// Get pointer to end of string
					ptr = command + strlen(command);

					// Add arguments
					for (node = command_list->list.lh_Head; node->ln_Succ; node = node->ln_Succ)
					{
						// Add space and name in quotes
						*ptr++ = ' ';
						*ptr++ = '\"';
						strcpy(ptr, node->ln_Name);

						// Bump pointer
						ptr += strlen(ptr);
						*ptr++ = '\"';
						*ptr = 0;
					}

					// Execute?
					if (execute)
					{
						char output[140];
						BPTR out;

						// Open output
						lsprintf(output,
								 "%s%s/AUTO/CLOSE/WAIT/SCREEN %s",
								 environment->env->output_device,
								 environment->env->output_window,
								 get_our_pubscreen());
						if (!(out = Open(output, MODE_OLDFILE)))
							out = Open("nil:", MODE_OLDFILE);

						// Run command
						CLI_Launch(command,
								   (struct Screen *)-1,
								   0,
								   out,
								   0,
								   LAUNCHF_USE_STACK,
								   environment->env->default_stack);
					}

					// Workbench launch
					else
					{
						WB_LaunchNew(command, GUI->screen_pointer, 0, environment->env->default_stack, default_tool);
					}

					// Free command buffer
					FreeVec(command);
				}

				// Free arg list
				Att_RemList(command_list, 0);
			}
		}

		// Free icon
		if (got_icon)
			FreeDiskObject(got_icon);
	}

	// Restore directory
	CurrentDir(old);
	UnLock(lock);
}

// Build arguments from icons
void backdrop_run_build_args(Att_List *list, BackdropInfo *info, BackdropObject *exclude)
{
	BackdropObject *object;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// Go through backdrop list backwards (to maintain compatibility with Workbench icon ordering)
	for (object = (BackdropObject *)info->objects.list.lh_TailPred; object->node.ln_Pred;
		 object = (BackdropObject *)object->node.ln_Pred)
	{
		// Is object selected (and not the one we're running) with a valid path?
		if (object->state && object != exclude && (object->path || object->type == BDO_DISK))
		{
			BOOL ok = 0;
			char name[256];

			// Is icon for a disk?
			if (object->type == BDO_DISK)
				ok = 1;

			// Is object a real one?
			else if (object->type == BDO_LEFT_OUT)
			{
				// In a lister, it's ok
				if (info->lister || object->flags & BDOF_DESKTOP_FOLDER)
					ok = 1;
			}

			// Ok to use?
			if (ok)
			{
				// Disk?
				if (object->type == BDO_DISK)
					stccpy(name, object->device_name, sizeof(name));

				// Build name
				else
				{
					stccpy(name, object->path, sizeof(name));
					AddPart(name, object->name, 256);
				}

				// Add to list
				Att_NewNode(list, name, 0, 0);
			}
		}
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);
}

/*
// Add to selection list
void backdrop_selection_list(BackdropInfo *info,BackdropObject *object)
{
	// Clear list?
	if (!object) Att_RemList(GUI->icon_sel_list,REMLIST_SAVELIST);

	// Add to list
	else
	{
		char name[256];

		// Disk?
		if (object->type==BDO_DISK)
			strcpy(name,object->device_name);

		// Build name
		else
		{
			strcpy(name,object->path);
			AddPart(name,object->name,256);
		}

		// Add to list
		Att_NewNode(GUI->icon_sel_list,name,(ULONG)info,ADDNODE_EXCLUSIVE);
	}
}
*/
