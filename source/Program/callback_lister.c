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
#include "replace.h"
#include <proto/module.h>

APTR ASM SAVEDS HookConvertEntry(REG(a0, FunctionEntry *entry))
{
	return (entry) ? entry->entry : 0;
}

Lister *ASM SAVEDS HookGetLister(REG(a0, PathNode *path))
{
	return (path) ? path->lister : 0;
}

ULONG ASM SAVEDS HookExamineEntry(REG(a0, FunctionEntry *entry), REG(d0, long type))
{
	if (!entry)
		return 0;

	// Name?
	if (type == EE_NAME)
		return (ULONG)entry->name;

	// Type
	else if (type == EE_TYPE)
		return (ULONG)entry->type;

	return 0;
}

/*************************************************/

PathNode *ASM SAVEDS HookGetSource(REG(a0, FunctionHandle *handle), REG(a1, char *pathbuf))
{
	PathNode *path;

	// Clear buffer
	if (pathbuf)
		*pathbuf = 0;

	// Valid source?
	if (!(path = function_path_current(&handle->source_paths)))
		return 0;

	// Copy path name
	if (pathbuf)
		strcpy(pathbuf, path->path);
	return path;
}

PathNode *ASM SAVEDS HookNextSource(REG(a0, FunctionHandle *handle), REG(a1, char *pathbuf))
{
	PathNode *path;

	// Clear buffer
	if (pathbuf)
		*pathbuf = 0;

	// Finish with current path
	function_path_end(handle, &handle->source_paths, 1);

	// Next valid source?
	if (!(path = function_path_current(&handle->source_paths)))
		return 0;

	// Copy path name
	if (pathbuf)
		strcpy(pathbuf, path->path);

	// Out of files?
	if (!function_current_entry(handle))
	{
		// Build new entry list
		function_build_list(handle, &path, handle->cur_instruction);
	}

	return path;
}

void ASM SAVEDS HookUnlockSource(REG(a0, FunctionHandle *handle))
{
	// Unlock listers
	function_do_lister_changes(handle, &handle->source_paths);
	function_unlock_paths(handle, &handle->source_paths, 1);
}

PathNode *ASM SAVEDS HookGetDest(REG(a0, FunctionHandle *handle), REG(a1, char *pathbuf))
{
	PathNode *path;

	// Clear buffer
	if (pathbuf)
		*pathbuf = 0;

	// Valid source?
	if (!(path = function_path_current(&handle->dest_paths)))
		return 0;

	// Copy path name
	if (pathbuf)
		strcpy(pathbuf, path->path);
	return path;
}

void ASM SAVEDS HookEndSource(REG(a0, FunctionHandle *handle), REG(d0, long complete))
{
	// Done with this path
	function_path_end(handle, &handle->source_paths, complete);
}

void ASM SAVEDS HookEndDest(REG(a0, FunctionHandle *handle), REG(d0, long complete))
{
	// Done with this path
	function_path_end(handle, &handle->dest_paths, complete);
}

APTR ASM SAVEDS HookGetEntry(REG(a0, FunctionHandle *handle))
{
	return (APTR)function_get_entry(handle);
}

void ASM SAVEDS HookFirstEntry(REG(a0, FunctionHandle *handle))
{
	// Initialise current entry pointer
	handle->current_entry = (FunctionEntry *)handle->entry_list.lh_Head;
}

void ASM SAVEDS HookEndEntry(REG(a0, FunctionHandle *handle), REG(a1, APTR entry), REG(d0, BOOL deselect))
{
	// End entry
	function_end_entry(handle, entry, deselect);
}

void ASM SAVEDS HookRemoveEntry(REG(a0, FunctionEntry *entry))
{
	if (entry)
		entry->flags |= FUNCENTF_REMOVE;
}

long ASM SAVEDS HookEntryCount(REG(a0, FunctionHandle *handle))
{
	return handle->entry_count;
}

void ASM SAVEDS HookReloadEntry(REG(a0, FunctionHandle *handle), REG(a1, FunctionEntry *entry))
{
	PathNode *path;

	// Get current path
	if ((path = function_path_current(&handle->source_paths)))
	{
		// Add for reload
		function_filechange_reloadfile(handle, path->path, entry->name, 0);
	}
}

void ASM SAVEDS HookAddFile(REG(a0, FunctionHandle *handle),
							REG(a1, char *path),
							REG(a2, struct FileInfoBlock *fib),
							REG(a3, Lister *lister))
{
	// No date given?
	if (fib->fib_Date.ds_Days == 0 && fib->fib_Date.ds_Minute == 0 && fib->fib_Date.ds_Tick == 0)
		DateStamp(&fib->fib_Date);

#ifdef USE_64BIT
	#warning detect if there is a valid fib_Size64 in the FIB
	((FileInfoBlock64 *)fib)->fib_Size64 = fib->fib_Size;
#endif

	// Add file
	function_filechange_addfile(handle, path, fib, 0, lister);
}

void ASM SAVEDS HookDelFile(REG(a0, FunctionHandle *handle),
							REG(a1, char *path),
							REG(a2, char *name),
							REG(a3, Lister *lister))
{
	// Add file
	function_filechange_delfile(handle, path, name, lister, 1);
}

void ASM SAVEDS HookLoadFile(REG(a0, FunctionHandle *handle),
							 REG(a1, char *path),
							 REG(a2, char *name),
							 REG(d0, long flags),
							 REG(d1, BOOL reload))
{
	// Reload?
	if (reload)
		function_filechange_reloadfile(handle, path, name, flags);

	// Load
	else
		function_filechange_loadfile(handle, path, name, flags);
}

void ASM SAVEDS HookDoChanges(REG(a0, FunctionHandle *handle))
{
	// Do changes
	function_filechange_do(handle, 1);
}

BOOL ASM SAVEDS HookCheckAbort(REG(a0, FunctionHandle *handle))
{
	return function_check_abort(handle);
}

struct Window *ASM SAVEDS HookGetWindow(REG(a0, PathNode *path))
{
	// Valid lister?
	if (path && path->lister)
		return path->lister->window;
	return 0;
}

struct MsgPort *ASM SAVEDS HookGetPort(REG(a0, char *portname))
{
	struct MsgPort *port;

	// Copy port name, return pointer to it
	if (portname)
		strcpy(portname, GUI->rexx_port_name);

	// Look for the port
	Forbid();
	port = FindPort(GUI->rexx_port_name);
	Permit();

	// Return port
	return port;
}

struct Screen *ASM SAVEDS HookGetScreen(REG(a0, char *screenname))
{
	// Copy screen name, return pointer to it
	if (screenname)
		strcpy(screenname, get_our_pubscreen());
	return GUI->screen_pointer;
}

DOpusScreenData *ASM SAVEDS HookGetScreenData(void)
{
	DOpusScreenData *data;

	// Allocate
	if ((data = AllocVec(sizeof(DOpusScreenData), MEMF_CLEAR)))
	{
		// Fill it out
		get_screen_data(data);
	}

	return data;
}

void ASM SAVEDS HookFreeScreenData(REG(a0, APTR data))
{
	FreeVec(data);
}

void ASM SAVEDS HookOpenProgress(REG(a0, PathNode *path), REG(a1, char *operation), REG(d0, long total))
{
	ProgressPacket *prog;

	// No lister?
	if (!path || !path->lister)
		return;

	// Allocate progress packet
	if ((prog = AllocVec(sizeof(ProgressPacket), MEMF_CLEAR)))
	{
		// Fill out packet
		prog->total = total;
		prog->operation = operation;
		prog->flags = PWF_FILENAME | PWF_GRAPH;

		// Open progress indicator in lister
		IPC_Command(path->lister->ipc, LISTER_PROGRESS_ON, 0, 0, prog, 0);
	}
}

void ASM SAVEDS HookUpdateProgress(REG(a0, PathNode *path), REG(a1, char *name), REG(d0, long count))
{
	// Update progress indicator
	if (path && path->lister)
		lister_command(path->lister, LISTER_PROGRESS_UPDATE, count, name, 0, 0);
}

void ASM SAVEDS HookCloseProgress(REG(a0, PathNode *path))
{
	// Close progress indicator
	if (path && path->lister)
		lister_command(path->lister, LISTER_PROGRESS_OFF, 0, 0, 0, 0);
}

long ASM SAVEDS HookReplaceReq(REG(a0, struct Window *window),
							   REG(a1, struct Screen *screen),
							   REG(a2, IPCData *ipc),
							   REG(a3, struct FileInfoBlock *file1),
							   REG(a4, struct FileInfoBlock *file2),
							   REG(d0, long flags))
{
	// Show requester
	return SmartAskReplace(window, screen, ipc, 0, file1, 0, file2, flags);
}

ULONG ASM SAVEDS HookGetPointer(REG(a0, struct pointer_packet *ptr))
{
	// Which type?
	switch (ptr->type)
	{
	// Options
	case MODPTR_OPTIONS:

		// Fill out packet
		ptr->pointer = &environment->env->settings;
		ptr->flags = 0;
		return (ULONG)&environment->env->settings;

	// Filetypes
	case MODPTR_FILETYPES:

		// Lock filetype list
		lock_listlock(&GUI->filetypes, FALSE);

		// Fill out packet
		ptr->pointer = &GUI->filetypes;
		ptr->flags = POINTERF_LOCKED;
		return (ULONG)&GUI->filetypes;

	// Handle
	case MODPTR_HANDLE:

		// Allocate a new handle
		if ((ptr->pointer = function_new_handle((struct MsgPort *)ptr->pointer, 0)))
			ptr->flags = POINTERF_LOCKED;
		return (ULONG)ptr->pointer;

	// Default lister format
	case MODPTR_DEFFORMAT:

		// Allocate a format structure
		if ((ptr->pointer = AllocVec(sizeof(ListFormat), 0)))
		{
			CopyMem((char *)&environment->env->list_format, (char *)ptr->pointer, sizeof(ListFormat));
			ptr->flags = POINTERF_LOCKED;
		}
		return (ULONG)ptr->pointer;

	// Command list
	case MODPTR_COMMANDS: {
		struct List *list;
		APTR mem_handle;
		CommandList *cmd;

		// Create memory handle and list structure
		if (!(mem_handle = NewMemHandle(2048, 512, MEMF_CLEAR | MEMF_PUBLIC)) ||
			!(list = AllocMemH(mem_handle, sizeof(struct List) + sizeof(APTR))))
		{
			FreeMemHandle(mem_handle);
			return 0;
		}

		// Save memory handle pointer in list structure
		*((APTR *)(list + 1)) = mem_handle;
		NewList(list);

		// Lock command list and go through it
		lock_listlock(&GUI->command_list, FALSE);
		for (cmd = (CommandList *)GUI->command_list.list.lh_Head; cmd->node.mln_Succ;
			 cmd = (CommandList *)cmd->node.mln_Succ)
		{
			struct DOpusCommandList *dcl;
			long size;

			// Skip private commands
			if (cmd->flags & FUNCF_PRIVATE)
				continue;

			// Calculate size needed
			size = sizeof(struct DOpusCommandList);
			size += strlen(cmd->name) + 1;
			if (cmd->desc)
				size += strlen((char *)cmd->desc) + 1;
			if (cmd->flags & FUNCF_EXTERNAL_FUNCTION && cmd->stuff.module_name)
				size += strlen((char *)cmd->stuff.module_name) + 1;
			if (cmd->template)
				size += strlen(cmd->template) + 1;
			if (cmd->help_name)
				size += strlen(cmd->help_name) + 1;

			// Allocate command list node
			if ((dcl = AllocMemH(mem_handle, size)))
			{
				char *ptr = (char *)(dcl + 1);

				// Initialise node
				dcl->dcl_Node.ln_Name = ptr;
				strcpy(dcl->dcl_Node.ln_Name, cmd->name);
				ptr += strlen(dcl->dcl_Node.ln_Name) + 1;
				dcl->dcl_Flags = cmd->flags;

				// Store optional strings
				if (cmd->desc)
				{
					dcl->dcl_Description = ptr;
					strcpy(dcl->dcl_Description, (char *)cmd->desc);
					ptr += strlen(dcl->dcl_Description) + 1;
				}
				if (cmd->flags & FUNCF_EXTERNAL_FUNCTION && cmd->stuff.module_name)
				{
					dcl->dcl_Module = ptr;
					strcpy(dcl->dcl_Module, (char *)cmd->stuff.module_name);
					ptr += strlen(dcl->dcl_Module) + 1;
				}
				if (cmd->template)
				{
					dcl->dcl_Template = ptr;
					strcpy(dcl->dcl_Template, (char *)cmd->template);
					ptr += strlen(dcl->dcl_Template) + 1;
				}
				if (cmd->help_name)
				{
					dcl->dcl_Help = ptr;
					strcpy(dcl->dcl_Help, (char *)cmd->help_name);
				}

				// Add to list
				AddTail(list, &dcl->dcl_Node);
			}
		}
		unlock_listlock(&GUI->command_list);

		// Initialise return packet
		ptr->pointer = list;
		ptr->flags = POINTERF_LOCKED;
	}
		return (ULONG)ptr->pointer;

	// Script list
	case MODPTR_SCRIPTS: {
		Att_List *list;
		Att_Node *node;

		// Build script list
		if ((list = Att_NewList(LISTF_POOL)))
		{
			for (node = (Att_Node *)script_list->list.lh_Head; node->node.ln_Succ;
				 node = (Att_Node *)node->node.ln_Succ)
				Att_NewNode(list, node->node.ln_Name, node->data, ADDNODE_SORT);
		}
		ptr->pointer = list;
		ptr->flags = POINTERF_LOCKED;
	}
		return (ULONG)ptr->pointer;
	}

	return 0;
}

void ASM SAVEDS HookFreePointer(REG(a0, struct pointer_packet *ptr))
{
	// Which type?
	switch (ptr->type)
	{
	// Filetypes
	case MODPTR_FILETYPES:

		// Check packet
		if (ptr->pointer == &GUI->filetypes)
		{
			// Unlock filetype list
			unlock_listlock(&GUI->filetypes);
		}
		break;

	// Handle
	case MODPTR_HANDLE:

		// Free handle
		function_free((FunctionHandle *)ptr->pointer);
		break;

	// Default lister format
	case MODPTR_DEFFORMAT:
		FreeVec(ptr->pointer);
		break;

	// Command list
	case MODPTR_COMMANDS: {
		struct List *list;
		struct DOpusCommandList *dcl, *next = 0;
		APTR mem_handle;

		// Get list and memory handle
		if (!(list = (struct List *)ptr->pointer) || !(mem_handle = *((APTR *)(list + 1))))
			break;

		// Free list nodes
		for (dcl = (struct DOpusCommandList *)list->lh_Head; dcl->dcl_Node.ln_Succ; dcl = next)
		{
			// Cache next pointer
			next = (struct DOpusCommandList *)dcl->dcl_Node.ln_Succ;

			// Free this entry
			FreeMemH(dcl);
		}

		// Free list and memory handle
		FreeMemH(list);
		FreeMemHandle(mem_handle);
	}
	break;

	// Script list
	case MODPTR_SCRIPTS:
		Att_RemList(ptr->pointer, 0);
		break;
	}
}

ULONG ASM SAVEDS HookSendCommand(REG(a0, FunctionHandle *handle),
								 REG(a1, char *command),
								 REG(a2, char **result),
								 REG(d0, ULONG flags))
{
	struct RexxMsg *msg;
	struct MsgPort *rexx_port;
	ULONG ret = 0;

	// Clear result pointer
	if (result)
		*result = 0;

	// Got ARexx?
	if (!RexxSysBase)
		return (ULONG)-1;

	// Just run script?
	if (flags & COMMANDF_RUN_SCRIPT)
	{
		rexx_send_command(command, 0);
		return 1;
	}

	// Allocate message
	if (!(msg = BuildRexxMsgExTags(handle->reply_port, 0, 0, RexxTag_Arg0, command, TAG_END)))
		return (ULONG)RXERR_NO_MEMORY;

	// Want reply?
	if (flags & COMMANDF_RESULT)
		msg->rm_Action |= RXFF_RESULT;
	else
		msg->rm_Action &= ~RXFF_RESULT;

	// Find rexx port
	Forbid();
	if ((rexx_port = FindPort(GUI->rexx_port_name)))
	{
		// Send message
		PutMsg(rexx_port, (struct Message *)msg);
		Permit();

		// Get reply
		WaitPort(handle->reply_port);
		GetMsg(handle->reply_port);

		// Wait for reply?
		if (flags & COMMANDF_RESULT)
		{
			// Got result string?
			if (msg->rm_Result2)
			{
				// Want result string?
				if (result)
				{
					// Copy result
					if ((*result = AllocVec(strlen((char *)msg->rm_Result2) + 1, 0)))
						strcpy(*result, (char *)msg->rm_Result2);
				}

				// Free result
				DeleteArgstring((STRPTR)msg->rm_Result2);
			}
		}

		// Store result code
		ret = msg->rm_Result1;
	}
	else
		Permit();

	// Free the message
	FreeRexxMsgEx(msg);

	return ret;
}

void ASM SAVEDS HookCheckDesktop(REG(a0, char *path))
{
	// See if path matches desktop path
	if (HookMatchDesktop(path))
	{
		// Update the desktop folder
		misc_startup("dopus_desktop_update", MENU_UPDATE_DESKTOP, GUI->window, 0, TRUE);
	}
}

BOOL ASM SAVEDS HookMatchDesktop(REG(a0, char *path))
{
	BOOL ret = 0;

	// See if path matches desktop path
	if (path)
	{
		BPTR lock1, lock2;

		// Lock both this path and the desktop
		if ((lock1 = Lock(path, ACCESS_READ)) && (lock2 = Lock(environment->env->desktop_location, ACCESS_READ)))
		{
			// See if they're the same
			if (SameLock(lock1, lock2) == LOCK_SAME)
			{
				ret = 1;
			}

			// Unlock the second lock
			UnLock(lock2);
		}

		// Unlock first lock
		UnLock(lock1);
	}

	return ret;
}

UWORD ASM SAVEDS HookGetDesktop(REG(a0, char *path))
{
	BPTR lock;

	// Get full pathname
	strcpy(path, environment->env->desktop_location);
	if ((lock = Lock(path, ACCESS_READ)))
	{
		DevNameFromLockDopus(lock, path, 256);
		UnLock(lock);
	}

	// Add trailing slash
	AddPart(path, "", 256);

	// Return flags
	if (!(environment->env->env_flags & ENVF_DESKTOP_FOLDER))
		return 0;
	else
		return (UWORD)(environment->env->desktop_popup_default + 1);
}

short ASM SAVEDS HookDesktopPopup(REG(a0, ULONG flags))
{
	UWORD res;
	PopUpItem *item;

	// Main window open?
	if (!GUI->window)
		return 0;

	// Make sure main window is active
	ActivateWindow(GUI->window);

	// Disable items
	if (flags & (1 << 1) && (item = GetPopUpItem(&GUI->desktop_menu->ph_Menu, MENU_DESKTOP_LEFTOUT)))
		item->flags |= POPUPF_DISABLED;
	if (flags & (1 << 2) && (item = GetPopUpItem(&GUI->desktop_menu->ph_Menu, MENU_DESKTOP_COPY)))
		item->flags |= POPUPF_DISABLED;
	if (flags & (1 << 3) && (item = GetPopUpItem(&GUI->desktop_menu->ph_Menu, MENU_DESKTOP_MOVE)))
		item->flags |= POPUPF_DISABLED;

	// Do popup menu
	res = DoPopUpMenu(GUI->window, &GUI->desktop_menu->ph_Menu, 0, SELECTDOWN);

	// Enable items
	if (flags & (1 << 1) && (item = GetPopUpItem(&GUI->desktop_menu->ph_Menu, MENU_DESKTOP_LEFTOUT)))
		item->flags &= ~POPUPF_DISABLED;
	if (flags & (1 << 2) && (item = GetPopUpItem(&GUI->desktop_menu->ph_Menu, MENU_DESKTOP_COPY)))
		item->flags &= ~POPUPF_DISABLED;
	if (flags & (1 << 3) && (item = GetPopUpItem(&GUI->desktop_menu->ph_Menu, MENU_DESKTOP_MOVE)))
		item->flags &= ~POPUPF_DISABLED;

	// Map result
	if (res == MENU_DESKTOP_LEFTOUT)
		return 1;
	if (res == MENU_DESKTOP_COPY)
		return 2;
	if (res == MENU_DESKTOP_MOVE)
		return 3;

	// Cancelled
	return 0;
}

void ASM SAVEDS HookGetThemes(REG(a0, char *path))
{
	BPTR lock;

	// Get full pathname
	strcpy(path, environment->env->themes_location);
	if ((lock = Lock(path, ACCESS_READ)))
	{
		DevNameFromLockDopus(lock, path, 256);
		UnLock(lock);
	}

	// Add trailing slash
	AddPart(path, "", 256);
}
