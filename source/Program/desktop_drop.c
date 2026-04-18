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

void desktop_drop(BackdropInfo *info, DOpusAppMessage *msg, UWORD qual)
{
	BackdropObject *drop_obj;
	short ok = 0, x, y;
	char *name, *source_path;
	short arg, action = DESKTOP_POPUP_LEFTOUT;
	struct ArgArray *array = 0;
	struct ArgArrayEntry *aae;
	BOOL check;

	// Lock backdrop list
	lock_listlock(&info->objects, 0);

	// See if we dropped stuff on an object
	if ((drop_obj = backdrop_get_object(info, msg->da_Msg.am_MouseX, msg->da_Msg.am_MouseY, 0)))
	{
		// Is shift/alt down?
		if ((qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT)) == (IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT))
		{
			char pathname[256];

			// Get path of first file
			GetWBArgPath(&msg->da_Msg.am_ArgList[0], pathname, 256);

			// Replace the image
			backdrop_replace_icon_image(info, pathname, drop_obj);

			// Reply the message
			ReplyAppMessage(msg);
		}

		// Handle drop on an object
		else if (desktop_drop_on_object(info, &msg, drop_obj, qual))
		{
			// Reply the message
			ReplyAppMessage(msg);
		}

		// Not dropped
		else
			drop_obj = 0;
	}

	// Unlock backdrop list
	unlock_listlock(&info->objects);

	// Dropped on an object?
	if (drop_obj)
		return;

	// Get buffer
	if (!(name = AllocVec(1024, MEMF_CLEAR)))
		return;
	source_path = name + 512;

	// Popup menu?
	if (environment->env->env_flags & ENVF_DESKTOP_FOLDER)
	{
		UWORD res;

		// Activate the underlying menu
		ActivateWindow(info->window);

		// Default action set?
		if (environment->env->desktop_popup_default > DESKTOP_POPUP_NONE &&
			!(qual & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)))
		{
			// Use default action
			action = environment->env->desktop_popup_default;
		}

		// Ask user what they want to do
		else if ((res = DoPopUpMenu(info->window, &GUI->desktop_menu->ph_Menu, 0, SELECTDOWN)) == (UWORD)-1 ||
				 res == MENU_DESKTOP_CANCEL)
		{
			// Cancelled
			FreeVec(name);
			return;
		}

		// Help?
		else if (res & POPUP_HELPFLAG)
		{
			// Do help
			help_menu_help(res & ~POPUP_HELPFLAG, 0);

			// Cancelled
			FreeVec(name);
			return;
		}

		// Get action
		else if (res == MENU_DESKTOP_COPY)
			action = DESKTOP_POPUP_COPY;
		else if (res == MENU_DESKTOP_MOVE)
			action = DESKTOP_POPUP_MOVE;
	}

	// Set busy pointer
	SetBusyPointer(info->window);

	// DOpus message?
	check = CheckAppMessage(msg);

	// Go through arguments
	for (arg = 0; arg < msg->da_Msg.am_NumArgs; arg++)
	{
		// What operation?
		switch (action)
		{
		// Leave out?
		case DESKTOP_POPUP_LEFTOUT:

			// Get path name
			GetWBArgPath(&msg->da_Msg.am_ArgList[arg], name, 512);

			// Ignore if it's an icon or a disk
			if (!(isicon(name)) && name[strlen(name) - 1] != ':')
			{
				short len, perm = 0;
				BackdropObject *icon;

				// Permanent?
				if (GUI->flags2 & GUIF2_BENIFY)
					perm = 1;

				// Get position
				x = msg->da_Msg.am_MouseX;
				y = msg->da_Msg.am_MouseY;

				// Drop from DOpus?
				if (check)
				{
					// Adjust position for icon offset
					x += msg->da_DragOffset.x + msg->da_DropPos[arg].x;
					y += msg->da_DragOffset.y + msg->da_DropPos[arg].y;
				}

				// Strip trailing /
				len = strlen(name) - 1;
				if (name[len] == '/')
					name[len] = 0;

				// Leave this out
				if ((icon = backdrop_leave_out(info, name, (perm) ? BLOF_PERMANENT : 0, x, y)))
				{
					// Position it
					backdrop_place_object(info, icon);

					// Save for permanent leftout
					if (perm)
						backdrop_save_leftouts(info);
					ok = 1;
				}
			}
			break;

		// Copy/Move
		case DESKTOP_POPUP_COPY:
		case DESKTOP_POPUP_MOVE: {
			BOOL dir = 0;

			// Create ArgArray if needed
			if (!array && !(array = NewArgArray()))
				break;

			// Get path name
			GetWBArgPath(&msg->da_Msg.am_ArgList[arg], name, 512);

			// Set flag if a directory
			if (!msg->da_Msg.am_ArgList[arg].wa_Name || !*msg->da_Msg.am_ArgList[arg].wa_Name)
				dir = 1;

			// Get source path
			if (!*source_path)
			{
				char *ptr;

				// Copy from name
				strcpy(source_path, name);

				// Strip last part
				if ((ptr = FilePart(source_path)))
					*ptr = 0;
			}

			// Create argument
			if ((aae = NewArgArrayEntry(array, FilePart(name))))
			{
				// Set directory flag
				if (dir)
					aae->ae_Flags |= AEF_DIR;

				// Set OK flag
				ok = 1;
			}
		}
		break;
		}
	}

	// Successful?
	if (ok)
	{
		// Left-outs?
		if (action == DESKTOP_POPUP_LEFTOUT)
		{
			// Refresh
			backdrop_show_objects(info, BDSF_RECALC);
		}

		// Otherwise, launch function
		else
		{
			// Launch the function
			function_launch(FUNCTION_RUN_FUNCTION_EXTERNAL,
							(action == DESKTOP_POPUP_COPY) ? def_function_copy : def_function_move,
							0,
							FUNCF_ICONS | FUNCF_RESCAN_DESKTOP | FUNCF_DRAG_DROP | FUNCF_COPY_NO_MOVE,
							0,
							0,
							source_path,
							environment->env->desktop_location,
							array,
							0,
							(Buttons *)CopyAppMessage(msg, global_memory_pool));
		}
	}

	// Otherwise, free array
	else
		FreeArgArray(array);

	// Free buffer
	FreeVec(name);

	// Clear busy pointer
	ClearPointer(info->window);

	// Reply the message
	ReplyAppMessage(msg);
}

// Handle drop on an object
BOOL desktop_drop_on_object(BackdropInfo *info, DOpusAppMessage **msg, BackdropObject *drop_obj, UWORD qual)
{
	char *name;
	short ret = 1;

	// Allocate buffer
	if (!(name = AllocVec(1024, 0)))
		return 1;

	// Was it an AppIcon?
	if (drop_obj->type == BDO_APP_ICON)
	{
		// Is the icon busy?
		if (drop_obj->flags & BDOF_BUSY)
		{
			// Flash error
			DisplayBeep(info->window->WScreen);
		}

		// Otherwise, pass message on
		else
		{
			struct MsgPort *port;

			// Turn message into an AppIcon one
			(*msg)->da_Msg.am_Type = MTYPE_APPICON;

			// Get port and info
			port = WB_AppWindowData(
				(struct AppWindow *)drop_obj->misc_data, &(*msg)->da_Msg.am_ID, &(*msg)->da_Msg.am_UserData);

			// Fix reply port
			(*msg)->da_Msg.am_Message.mn_ReplyPort = GUI->appmsg_port;

			// Send the message on
			PutMsg(port, (struct Message *)*msg);

			// Zero message pointer
			*msg = 0;
		}
	}

	// Was it a group?
	else if (drop_obj->type == BDO_GROUP)
	{
		short arg;
		GroupData *group;
		BOOL ok = 0;

		// Find group if it's open
		lock_listlock(&GUI->group_list, 0);
		if (!(group = backdrop_find_group(drop_obj)))
			unlock_listlock(&GUI->group_list);

		// Go through arguments
		for (arg = 0; arg < (*msg)->da_Msg.am_NumArgs; arg++)
		{
			// Valid file?
			if ((*msg)->da_Msg.am_ArgList[arg].wa_Name && *(*msg)->da_Msg.am_ArgList[arg].wa_Name)
			{
				// Get filename
				GetWBArgPath(&(*msg)->da_Msg.am_ArgList[arg], name, 512);

				// Send add message to group if it's open
				if (group)
				{
					char *copy;

					// Copy name
					if ((copy = AllocVec(strlen(name) + 1, 0)))
					{
						strcpy(copy, name);
						IPC_Command(group->ipc, GROUP_ADD_ICON, 0, 0, copy, 0);
						ok = 1;
					}
				}

				// Otherwise add object to group
				else
				{
					backdrop_group_add_object(drop_obj->name, 0, name, -1, -1);
					ok = 1;
				}
			}
		}

		// Show error if no ok
		if (!ok)
			DisplayBeep(GUI->screen_pointer);

		// Unlock process list if it's locked
		if (group)
			unlock_listlock(&GUI->group_list);
	}

	// Or a project?
	else if (drop_obj->icon->do_Type == WBPROJECT)
	{
		// Is it an Opus command?
		if (command_filetype)
		{
			BPTR lock, old;

			// Set failure initially
			ret = 0;

			// Get lock on directory
			if ((lock = backdrop_icon_lock(drop_obj)))
			{
				// Go there
				old = CurrentDir(lock);

				// See if it's a command
				if (filetype_match_type(drop_obj->name, command_filetype))
				{
					// Run command with args
					backdrop_object_open(info, drop_obj, 0, 0, (*msg)->da_Msg.am_NumArgs, (*msg)->da_Msg.am_ArgList);
					ret = 1;
				}

				// Restore CD
				UnLock(CurrentDir(old));
			}
		}

		// Beep if not a command
		if (!ret)
			DisplayBeep(GUI->screen_pointer);
	}

	// Or a tool?
	else if (drop_obj->icon->do_Type == WBTOOL)
	{
		// Run program with args
		backdrop_object_open(info, drop_obj, 0, 0, (*msg)->da_Msg.am_NumArgs, (*msg)->da_Msg.am_ArgList);
	}

	// Or a disk/directory?
	else if (drop_obj->icon->do_Type == WBDISK || drop_obj->icon->do_Type == WBDRAWER ||
			 drop_obj->icon->do_Type == WBGARBAGE)
	{
		struct ArgArray *arg_array;

		// Get arg array
		if ((arg_array = AppArgArray(*msg, AAF_ALLOW_DIRS)))
		{
			BPTR lock;

			// Get pathname of first file
			DevNameFromLockDopus((*msg)->da_Msg.am_ArgList[0].wa_Lock, name, 512);

			// Need source directory; if no name, get parent
			if ((!(*msg)->da_Msg.am_ArgList[0].wa_Name || !*(*msg)->da_Msg.am_ArgList[0].wa_Name) &&
				(lock = ParentDir((*msg)->da_Msg.am_ArgList[0].wa_Lock)))
			{
				// Get pathname of parent
				DevNameFromLockDopus(lock, name, 512);
				UnLock(lock);
			}

			// Get destination path
			if ((lock = backdrop_icon_lock(drop_obj)))
			{
				short action;

				// Get path
				DevNameFromLockDopus(lock, name + 512, 512);
				UnLock(lock);

				// Is object a left-out?
				if (drop_obj->type == BDO_LEFT_OUT)
				{
					// Add left-out name
					AddPart(name + 512, drop_obj->name, 512);
				}

				// Get filetype action
				if (qual & IEQUALIFIER_CONTROL)
					action = FTTYPE_CTRL_DRAGDROP;
				else if (qual & (IEQUALIFIER_LALT | IEQUALIFIER_RALT))
					action = FTTYPE_ALT_DRAGDROP;
				else
					action = FTTYPE_DRAG_DROP;

				// Do filetype action on files
				function_launch(FUNCTION_FILETYPE,
								0,
								action,
								FUNCF_DRAG_DROP | FUNCF_ICONS,
								0,
								0,
								name,
								name + 512,
								arg_array,
								0,
								(Buttons *)CopyAppMessage(*msg, global_memory_pool));
			}
		}
	}
	else
		ret = 0;

	FreeVec(name);
	return ret;
}
