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

IPC_EntryProto(help_proc, static);

// Someone pressed the help key
void help_get_help(short x, short y, unsigned short qual)
{
	struct Window *window = 0;
	struct Layer *layer;
	IPCData *ipc = 0;

	// Lock screen layer
	LockLayerInfo(&GUI->screen_pointer->LayerInfo);

	// Find which layer the mouse is over
	if ((layer = WhichLayer(&GUI->screen_pointer->LayerInfo, x, y)))
	{
		// Get window pointer
		window = layer->Window;

		// Get window ID
		if (GetWindowID(window) != WINDOW_UNKNOWN)
		{
			// Forbid to get port
			Forbid();

			// Get port
			if (!(ipc = (IPCData *)GetWindowAppPort(window)))
				Permit();
		}
	}

	// Unlock layer
	UnlockLayerInfo(&GUI->screen_pointer->LayerInfo);

	// Got a port?
	if (ipc)
	{
		ULONG coords;

		// Convert coordinates to window-relative
		x -= window->LeftEdge;
		y -= window->TopEdge;

		// Pack into longword
		coords = ((unsigned short)x << 16) | (unsigned short)y;

		// Send help command
		IPC_Command(ipc, IPC_HELP, qual, (APTR)(IPTR)coords, 0, 0);

		// Enable multitasking now that message has been sent
		Permit();
	}

	// Otherwise, show generic help
	else
		help_show_help(HELP_MAIN, 0);
}

// Help for a button
void help_button_help(Cfg_Button *button, Cfg_Function *func, unsigned short qual, short which, char *generic)
{
	Cfg_Function *function;
	Cfg_Instruction *ins;
	char *ptr;
	CommandList *command;
	char *help_node = 0;

	// Function supplied?
	if (func)
		function = func;

	// No function pointer supplied
	else
	{
		// Function number supplied
		if (qual != 0xffff)
		{
			// Right button?
			if (qual & IEQUALIFIER_RBUTTON)
				which = FTYPE_RIGHT_BUTTON;

			// Middle button?
			else if (qual & IEQUALIFIER_MIDBUTTON || (qual & IEQUALIFIER_LEFTBUTTON && qual & IEQUAL_ANYSHIFT))
				which = FTYPE_MID_BUTTON;

			// Default function
			else
				which = button->current;
		}

		// Get function
		if (!(function = button_valid(button, which)) && !(function = button_valid(button, FTYPE_LEFT_BUTTON)))
		{
			// Invalid function
			help_node = generic;
		}
	}

	// Valid function
	if (function)
	{
		// Get first instruction
		ins = (Cfg_Instruction *)function->instructions.mlh_Head;

		// Valid command?
		if (!ins->node.mln_Succ || ins->type != INST_COMMAND || !(ptr = ins->string) ||
			!(command = function_find_internal(&ptr, 0)))
		{
			// Invalid instruction
			help_node = generic;
		}

		// Get command
		else
			help_node = command->name;
	}

	// Show help on this command
	help_show_help(help_node, 0);
}

// Menu item help
void help_menu_help(long id, char *generic)
{
	short a;
	char *header = 0;
	Cfg_Button *button;
	APTR appitem;

	// Go through menu help table
	for (a = 0; menu_help_data[a].name; a++)
	{
		// New header?
		if (menu_help_data[a].id == 0)
			header = menu_help_data[a].name;

		// Match ID?
		else if (menu_help_data[a].id == id)
		{
			char buf[80];

			// Build help string
			if (header)
			{
				strcpy(buf, header);
				strcat(buf, " - ");
			}
			else
				buf[0] = 0;
			strcat(buf, menu_help_data[a].name);

			// Show help
			help_show_help(buf, 0);
			return;
		}
	}

	// Lock user menu
	GetSemaphore(&GUI->user_menu_lock, SEMF_SHARED, 0);

	// Go through menu, look for this function
	for (button = (Cfg_Button *)GUI->user_menu->buttons.lh_Head; button->node.ln_Succ;
		 button = (Cfg_Button *)button->node.ln_Succ)
	{
		Cfg_ButtonFunction *func;

		// Go through functions
		for (func = (Cfg_ButtonFunction *)button->function_list.mlh_Head; func->node.ln_Succ;
			 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
		{
			// Match function
			if (func == (Cfg_ButtonFunction *)id)
			{
				// Show help for user menu function
				help_button_help(button, (Cfg_Function *)func, 0xffff, func->function.func_type, GENERIC_USER_MENU);
				FreeSemaphore(&GUI->user_menu_lock);
				return;
			}
		}
	}

	// Unlock user menu
	FreeSemaphore(&GUI->user_menu_lock);

	// Lock app list
	appitem = LockAppList();

	// Search appmenu list, see if that's what was hit
	while ((appitem = NextAppEntry(appitem, APP_MENU)))
	{
		// Item we want?
		if (appitem == (APTR)id)
		{
			// Show help for AppItems
			generic = HELP_APPMENUITEM;
			break;
		}
	}

	// Unlock list
	UnlockAppList();

	// Show generic help
	help_show_help(generic, 0);
}

// Show help on something
void help_show_help(char *thing, char *file)
{
	IPCData *ipc;
	char *copy, *ptr, helpbuf[256];
	CommandList *cmd;

	// Valid thing?
	if (!thing || !*thing)
		return;

	// Can't do help if we don't have the amigaguide.library
	if (!AmigaGuideBase)
	{
		// Flash error and return
		DisplayBeep(GUI->screen_pointer);
		return;
	}

	// Is the node name a command?
	ptr = thing;
	if (!file && (cmd = function_find_internal(&ptr, 0)))
	{
		// Is this an external module that has help available?
		if (cmd->flags & FUNCF_EXTERNAL_FUNCTION && cmd->help_name)
		{
			// Build full path to help file
			strcpy(helpbuf, "DOpus5:Help/");
			AddPart(helpbuf, FilePart(cmd->help_name), 256);

			// Use this file for help
			file = helpbuf;
		}
	}

	// Lock process list
	lock_listlock(&GUI->process_list, FALSE);

	// Is help process running?
	if (!(ipc = IPC_FindProc(&GUI->process_list, "dopus_help", 0, 0)))
	{
		// Unlock process list
		unlock_listlock(&GUI->process_list);

		// Launch help process
		if (!(IPC_Launch(&GUI->process_list,
						 &ipc,
						 "dopus_help",
						 (IPTR)&help_proc,
						 STACK_DEFAULT,
						 0,
						 (struct Library *)DOSBase)))
		{
			// Couldn't launch it
			return;
		}

		// Lock process list
		lock_listlock(&GUI->process_list, FALSE);
	}

	// No file given?
	if (!file || !*file)
		file = "DOpus5:Help/DOpus5.guide";

	// Copy thing
	if (ipc && (copy = AllocVec(strlen(thing) + 1 + strlen(file) + 1, 0)))
	{
		// Copy thing and file string
		strcpy(copy, thing);
		strcpy(copy + strlen(copy) + 1, file);

		// Send help command
		IPC_Command(ipc, HELPCMD_LINK, 0, 0, copy, 0);
	}

	// Unlock process list
	unlock_listlock(&GUI->process_list);
}

// Help process
IPC_EntryCode(help_proc, static)
{
	struct NewAmigaGuide guide;
	AMIGAGUIDECONTEXT help = 0;
	IPCData *ipc;
	char buf[256], filename[256];
	ULONG waitbits;
	BOOL help_ok = 0;
	struct MinList queue;
	IPCMessage *msg;

	// Do startup
	if ((ipc = IPC_ProcStartup(0, 0)))
	{
		// Get initial database name
		strcpy(filename, "DOpus5:Help/DOpus5.guide");

		// Get wait bits
		waitbits = 1 << ipc->command_port->mp_SigBit;

		// Initialise queue list
		NewList((struct List *)&queue);

		// Event loop
		FOREVER
		{
			struct AmigaGuideMsg *agm;
			BOOL quit_flag = 0;

			// AmigaGuide messages?
			while (help && (agm = GetAmigaGuideMsg(help)))
			{
				// Quit?
				if (agm->agm_Type == ShutdownMsgID)
					quit_flag = 1;

				// Ok to proceed?
				else if (agm->agm_Type == ActiveToolID || agm->agm_Type == ToolStatusID)
				{
					// Ok to send link commands
					help_ok = 1;

					// Go through queue
					for (msg = (IPCMessage *)queue.mlh_Head; msg->msg.mn_Node.ln_Succ;)
					{
						IPCMessage *next = (IPCMessage *)msg->msg.mn_Node.ln_Succ;

						// Build command string
						lsprintf(buf, "link \"%s\"", (char *)msg->data_free);

						// Send command
						SendAmigaGuideCmdA(help, buf, 0);

						// Remove from queue
						Remove((struct Node *)msg);

						// Reply to message
						IPC_Reply(msg);

						// Get next
						msg = next;
					}
				}

				// Reply
				ReplyAmigaGuideMsg(agm);
			}

			// Get messages
			while ((msg = (IPCMessage *)GetMsg(ipc->command_port)))
			{
				// Look at command
				switch (msg->command)
				{
				// Show a link?
				case HELPCMD_LINK: {
					char *name;

					// Get guide name
					name = (char *)msg->data_free;
					name += strlen(name) + 1;

					// New guide file, or don't have a guide at all?
					if ((*name && stricmp(name, filename) != 0) || !help)
					{
						// Shutdown the existing database
						if (help)
							CloseAmigaGuide(help);

						// Copy new filename
						if (*name)
							strcpy(filename, name);

						// Open new database
						if (!(help = help_open_database(&guide, filename, GUI->screen, GUI->rexx_port_name)))
						{
							// Can't get database
							DisplayBeep(GUI->screen_pointer);
							quit_flag = 1;
						}

						// Got database
						else
						{
							// Get waitbits
							waitbits = AmigaGuideSignal(help);
							waitbits |= 1 << ipc->command_port->mp_SigBit;
						}

						// Can no longer take messages (need an ok from AG)
						help_ok = 0;
					}

					// Haven't got ok message yet?
					if (!help_ok)
					{
						// Add message to queue
						AddTail((struct List *)&queue, (struct Node *)msg);

						// Clear pointer so it won't get replied
						msg = 0;
					}

					// Ok to send command
					else
					{
						// Build command string
						lsprintf(buf, "link \"%s\"", (char *)msg->data_free);

						// Send command
						SendAmigaGuideCmdA(help, buf, 0);
					}
				}
				break;

				// Hide/Quit
				case IPC_HIDE:
				case IPC_QUIT:
					quit_flag = 1;
					break;
				}

				// Reply
				IPC_Reply(msg);
			}

			// Check quit flag
			if (quit_flag)
				break;

			// Wait for events
			Wait(waitbits);
		}

		// Close database
		if (help)
			CloseAmigaGuide(help);

		// Anything left in the queue?
		for (msg = (IPCMessage *)queue.mlh_Head; msg->msg.mn_Node.ln_Succ;)
		{
			IPCMessage *next = (IPCMessage *)msg->msg.mn_Node.ln_Succ;

			// Reply to message
			IPC_Reply(msg);

			// Get next
			msg = next;
		}

		// Send goodbye message
		IPC_Goodbye(ipc, &main_ipc, 0);
	}

	// Exit
	IPC_Free(ipc);
}

// Open a database
AMIGAGUIDECONTEXT help_open_database(struct NewAmigaGuide *guide, char *name, struct Screen *screen, char *screen_name)
{
	// Fill in guide structure
	guide->nag_Lock = 0;
	guide->nag_Name = name;
	if (GUI->screen)
	{
		guide->nag_Screen = 0;
		guide->nag_PubScreen = screen_name;
	}
	else
	{
		guide->nag_Screen = screen;
		guide->nag_PubScreen = 0;
	}
	guide->nag_HostPort = screen_name;
	guide->nag_ClientPort = "DOPUS_HELP";
	guide->nag_Flags = HTF_CACHE_NODE | HTF_CACHE_DB | HTF_UNIQUE;
	guide->nag_Context = 0;
	guide->nag_Node = 0;
	guide->nag_Line = 0;
	guide->nag_Extens = 0;
	guide->nag_Client = 0;

	// Open help database
	return OpenAmigaGuideAsyncA(guide, 0);
}
