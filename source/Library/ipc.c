/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas (dopus5allamigas fork)

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

#include "dopuslib.h"

void ipc_remove_list(IPCData *ipc);

// Launch a generic process
int LIBFUNC L_IPC_Launch(REG(a0, struct ListLock *list),
						 REG(a1, IPCData **storage),
						 REG(a2, char *name),
						 REG(d0, IPTR entry),
						 REG(d1, ULONG stack),
						 REG(d2, IPTR data),
						 REG(a3, struct Library *dos_base),
						 REG(a6, struct MyLibrary *libbase))
{
	struct LibData *libdata;
	IPCData *ipc;
	BOOL path = 0;
	struct TagItem *tags;
	BPTR pathlist = 0;

#if defined(__amigaos4__) || defined(__AROS__)
	libbase = dopuslibbase_global;
#endif

	// Want path?
	if (stack & IPCF_GETPATH)
		path = 1;

	// Get data pointer
	libdata = (struct LibData *)libbase->ml_UserData;

	// Clear storage
	if (storage)
		*storage = 0;

	// Allocate data
	if (!(ipc = AllocVec(sizeof(IPCData), MEMF_CLEAR)) || !((tags = AllocVec(sizeof(struct TagItem) * 8, MEMF_ANY))))
	{
		FreeVec(ipc);
		return 0;
	}

	// Store memory and list pointers
	ipc->memory = libdata->memory;
	ipc->list = list;

	// Fill out process tags
	tags[0].ti_Tag = NP_Entry;
	tags[0].ti_Data = IPC_GET_ENTRY(entry);
	tags[1].ti_Tag = NP_Name;
	tags[1].ti_Data = (IPTR)name;
	tags[2].ti_Tag = NP_WindowPtr;
	tags[2].ti_Data = (IPTR)-1;
	tags[3].ti_Tag = NP_StackSize;
	tags[3].ti_Data = IPCM_STACK(stack);
	tags[4].ti_Tag = NP_Priority;
	tags[4].ti_Data = 0;

#if defined(__MORPHOS__)
	// All Dopus5 code is compiled PPC on MorphOS, and IPC entry points are
	// now native PPC functions (see IPC_EntryCode in libraries/dopus5.h).
	// Force NP_CodeType=CODETYPE_PPC unconditionally; the old IPCF_NATIVE
	// flag is preserved for source compatibility but no longer required.
	tags[3].ti_Tag = NP_CodeType;  // Overwriting NP_StackSize (not required in PPC native code)
	tags[3].ti_Data = CODETYPE_PPC;
#endif

	// Want a path?
	if (path)
	{
#define DOpusBase (libdata->dopus_base)
		// Lock path list
		GetSemaphore(&libdata->path_lock, SEMF_SHARED, 0);

		// Get path list copy
		pathlist = GetDosPathList(libdata->path_list);

		// Unlock path list
		FreeSemaphore(&libdata->path_lock);
#undef DOpusBase

		// Fill out tags
		tags[5].ti_Tag = NP_Cli;
		tags[5].ti_Data = TRUE;
		tags[6].ti_Tag = NP_Path;
		tags[6].ti_Data = (IPTR)pathlist;
		tags[7].ti_Tag = TAG_END;
	}
	else
		tags[5].ti_Tag = TAG_END;

#define DOSBase (dos_base)
	// Launch process
	ipc->proc = CreateNewProc(tags);
#undef DOSBase

	// Free tags now
	FreeVec(tags);

	// Failed to launch?
	if (!ipc->proc)
	{
		if (path)
			L_FreeDosPathList(pathlist);
		FreeVec(ipc);
		return 0;
	}

	// Store pointer
	if (storage)
		*storage = ipc;

	// Send startup message
	return L_IPC_Startup(ipc, (APTR)data, 0);
}

// Send an IPC startup
int LIBFUNC L_IPC_Startup(REG(a0, IPCData *ipc), REG(a1, APTR data), REG(a2, struct MsgPort *reply))
{
	struct MsgPort *port = 0;
	IPCMessage startup;

	// If no message port supplied, create one
	if (!reply)
		port = reply = CreateMsgPort();

	// Fill out startup message
	startup.msg.mn_ReplyPort = reply;
	startup.command = IPC_STARTUP;
	startup.flags = (IPTR)ipc;
	startup.data = data;

	// Send the startup message
	PutMsg(&ipc->proc->pr_MsgPort, (struct Message *)&startup);

	// Wait for reply back
	WaitPort(reply);
	GetMsg(reply);

	// Delete port if we created one
	if (port)
		DeleteMsgPort(port);

	// If there's no command port, report failure
	if (startup.command != IPC_STARTUP)
		return 0;
	return 1;
}

STATIC ULONG CallStartupCode(ULONG (*ASM code)(REG(a0, IPCData *), REG(a1, APTR)), APTR ipc, APTR data)
{
	ULONG rc = 1;

	if (code)
	{
		// IPC_StartupCode() generates a native function on every target
		// (including MorphOS since the trampoline redesign), so we can
		// call it directly. The old MorphOS EmulCallDirect68k() path went
		// through an EmulLibEntry{TRAP_LIB} trampoline that no longer
		// exists.
		rc = code(ipc, data);
	}

	return rc;
}

// Generic IPC startup code
IPCData *LIBFUNC L_IPC_ProcStartup(REG(a0, IPTR *data), REG(a1, ULONG (*ASM code)(REG(a0, IPCData *), REG(a1, APTR))))
{
	IPCData *ipc;
	IPCMessage *msg;
	struct Task *task;
	struct MsgPort *port;
	BOOL success = 1;

	// Get our task
	task = FindTask(0);

	// Get startup message
	port = &((struct Process *)task)->pr_MsgPort;
	WaitPort(port);
	msg = (IPCMessage *)GetMsg(port);

	// Get IPC pointer
	ipc = (IPCData *)msg->flags;

	// Store IPC pointer
	task->tc_UserData = ipc;

	// Save data pointer
	if (data)
		*data = (IPTR)msg->data;

	// Run startup code, create command and reply ports
	if (!CallStartupCode(code, ipc, msg->data) || !(ipc->command_port = CreateMsgPort()) ||
		!(ipc->reply_port = CreateMsgPort()))
	{
		msg->command = 0;
		success = 0;
	}

	// Were we successful?
	if (success)
	{
		// Want to go into a list?
		if (ipc->list)
		{
			// Lock and add to list
			L_GetSemaphore(&ipc->list->lock, SEMF_EXCLUSIVE, 0);
			AddTail(&ipc->list->list, (struct Node *)ipc);
			L_FreeSemaphore(&ipc->list->lock);

			// Set flag to say we're listed
			ipc->flags |= IPCF_LISTED;
		}
	}

	// Reply to startup message
	ReplyMsg((struct Message *)msg);

	// Were we successful?
	if (success)
		return ipc;

	return 0;
}

// Free an IPC
void LIBFUNC L_IPC_Free(REG(a0, IPCData *ipc))
{
	if (ipc)
	{
		// Mark as invalid
		ipc->flags |= IPCF_INVALID;

		// If we were in a list, remove ourselves
		ipc_remove_list(ipc);

		// Free command port
		L_IPC_Flush(ipc);
		DeleteMsgPort(ipc->command_port);

		// Free reply port
		DeleteMsgPort(ipc->reply_port);

		// Free IPC data
		FreeVec(ipc);
	}
}

// Flush an IPC port
void LIBFUNC L_IPC_Flush(REG(a0, IPCData *ipc))
{
	IPCMessage *msg;

	// Flush port
	if (ipc->command_port)
	{
		// Get messages
		while ((msg = (IPCMessage *)GetMsg(ipc->command_port)))
		{
			// Abort message
			msg->command = IPC_ABORT;

			// Reply message
			L_IPC_Reply(msg);
		}
	}
}

// Send an IPC command
IPTR LIBFUNC L_IPC_Command(REG(a0, IPCData *ipc),
						   REG(d0, ULONG command),
						   REG(d1, IPTR flags),
						   REG(a1, APTR data),
						   REG(a2, APTR data_free),
						   REG(a3, struct MsgPort *reply))
{
	struct MsgPort *port = 0;
	IPCData *sender_ipc = 0;
	IPCMessage *msg;
	IPTR result = 1;
	struct Task *task;
	APTR memory = 0;

	// Valid IPC?
	if (!ipc || !ipc->command_port || ipc->flags & IPCF_INVALID)
		return 0;

	// Get this task
	task = FindTask(0);
	if (reply == NO_PORT_IPC)
		reply = 0;
	else if (reply != REPLY_NO_PORT_IPC)
	{
		// Get sender
		if ((sender_ipc = (IPCData *)(task->tc_UserData)))
			memory = sender_ipc->memory;
	}

	// Allocate message
	if (!(msg = L_AllocMemH(memory, sizeof(IPCMessage))))
		return 0;

	// Get reply port from IPC if necessary
	if (reply == REPLY_NO_PORT)
	{
		if (sender_ipc)
			reply = sender_ipc->reply_port;
		else
			reply = REPLY_NO_PORT_IPC;
	}

	// Build message
	msg->command = command;
	msg->flags = flags;
	msg->data = data;
	msg->data_free = data_free;
	msg->sender = sender_ipc;

	// Want reply, no port supplied?
	if (reply == REPLY_NO_PORT_IPC)
	{
		// Create message port
		port = CreateMsgPort();

		// Use as reply port
		msg->msg.mn_ReplyPort = port;
		reply = port;
	}

	// Want reply, port supplied?
	else if (reply)
	{
		// Use supplied port
		msg->msg.mn_ReplyPort = reply;

		// Async?
		if (reply->mp_Node.ln_Pri == PORT_ASYNC_MAGIC)	// original dopus5code: if (reply->mp_Flags&PF_ASYNC)
			reply = 0;
	}

	// Send the message
	PutMsg(ipc->command_port, (struct Message *)msg);

	// Waiting for reply?
	if (reply)
	{
		// Loop for a reply
		FOREVER
		{
			// Got a reply?
			if ((GetMsg(reply)) == (struct Message *)msg)
				break;

			/*
						// Any messages in sender's port?
						if (sender_ipc &&
							!(IsMsgPortEmpty(sender_ipc->command_port)))
						{
							IPCMessage *mes,*next;

							// Go through message list; shouldn't need to Forbid() since
							// this list won't be looked at until we return
							for (mes=(IPCMessage *)sender_ipc->command_port->mp_MsgList.lh_Head;
								mes->msg.mn_Node.ln_Succ;
								mes=next)
							{
								// Cache next message
								next=(IPCMessage *)mes->msg.mn_Node.ln_Succ;

								// Is this message FROM the IPC we just sent a message to?
								if (mes->sender==ipc)
								{
									// Does it require a reply?
									if (mes->msg.mn_ReplyPort)
									{
			D(bug("*** deadlock message from %s (%lx) to %s (%lx)! ***\n"
					"command : %lx\nflags   : %lx\ndata    : %lx\n",
			mes->sender->proc->pr_Task.tc_Node.ln_Name,mes->sender->proc,
			task->tc_Node.ln_Name,task,
			mes->command,mes->flags,mes->data));
										// Remove this message
										Remove((struct Node *)mes);

										// Abort message
										mes->command=IPC_ABORT;

										// Quietly reply to it
										L_IPC_Reply(mes);
									}
								}
							}
						}
			*/

			// Wait for a message
			Wait(1 << reply->mp_SigBit);
			/*
							((sender_ipc)?(1<<sender_ipc->command_port->mp_SigBit):0));
			*/
		}

		// Get result of command
		result = msg->command;

		// Free message
		FreeVec(data_free);
		L_FreeMemH(msg);

		// Free port if we created one
		if (port)
			DeleteMsgPort(port);
	}

	return result;
}

// Send a safe command
IPTR LIBFUNC L_IPC_SafeCommand(REG(a0, IPCData *ipc),
							   REG(d0, ULONG command),
							   REG(d1, IPTR flags),
							   REG(a1, APTR data),
							   REG(a2, APTR data_free),
							   REG(a3, struct MsgPort *reply),
							   REG(a4, struct ListLock *list))
{
	IPCData *look;
	IPTR res = -1;

	// Lock list
	L_GetSemaphore(&list->lock, SEMF_SHARED, 0);

	// See if locker is still valid
	for (look = (IPCData *)list->list.lh_Head; look->node.mln_Succ; look = (IPCData *)look->node.mln_Succ)
	{
		// Match pointer?
		if (ipc == look)
		{
			// Send command
			res = L_IPC_Command(ipc, command, flags, data, data_free, reply);
			break;
		}
	}

	// Unlock list
	L_FreeSemaphore(&list->lock);
	return res;
}

// Reply to an IPC command
void LIBFUNC L_IPC_Reply(REG(a0, IPCMessage *msg))
{
	// Valid message?
	if (!msg)
		return;

	// Message needs a reply?
	if (msg->msg.mn_ReplyPort && msg->msg.mn_Node.ln_Type != NT_REPLYMSG)
		ReplyMsg((struct Message *)msg);

	// Otherwise, free it
	else
	{
		FreeVec(msg->data_free);
		L_FreeMemH(msg);
	}
}

// Find a process in a list
IPCData *LIBFUNC L_IPC_FindProc(REG(a0, struct ListLock *list),
								REG(a1, char *name),
								REG(d0, BOOL activate),
								REG(d1, IPTR data))
{
	IPCData *ipc;

	// Lock list
	L_GetSemaphore(&list->lock, SEMF_SHARED, 0);

	// Go through list
	for (ipc = (IPCData *)list->list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Is this the one we're after?
		if (ipc->proc && ipc->proc->pr_Task.tc_Node.ln_Name && strcmp(name, ipc->proc->pr_Task.tc_Node.ln_Name) == 0)
			break;
	}

	// Found?
	if (ipc && ipc->node.mln_Succ)
	{
		// Send activate message?
		if (activate)
			L_IPC_Command(ipc, IPC_ACTIVATE, 0, (APTR)data, 0, 0);
	}
	else
		ipc = 0;

	// Release semaphore
	L_FreeSemaphore(&list->lock);

	return ipc;
}

// Kill a process by name
void LIBFUNC L_IPC_QuitName(REG(a0, struct ListLock *list), REG(a1, char *name), REG(d0, ULONG quit_flags))
{
	IPCData *ipc;

	// Lock list
	L_GetSemaphore(&list->lock, SEMF_SHARED, 0);

	// Go through list
	for (ipc = (IPCData *)list->list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Is this the one we're after?
		if (ipc->proc && ipc->proc->pr_Task.tc_Node.ln_Name && strcmp(name, ipc->proc->pr_Task.tc_Node.ln_Name) == 0)
		{
			// Send quit message
			L_IPC_Quit(ipc, quit_flags, 0);
			break;
		}
	}

	// Release semaphore
	L_FreeSemaphore(&list->lock);
}

// Send a quit message to an IPC process
void LIBFUNC L_IPC_Quit(REG(a0, IPCData *ipc), REG(d0, ULONG quit_flags), REG(d1, BOOL wait))
{
	// Valid process?
	if (ipc && ipc->proc)
	{
		// Signal the process with control-F
		Signal((struct Task *)ipc->proc, IPCSIG_QUIT);

		// Send quit message
		L_IPC_Command(ipc, IPC_QUIT, quit_flags, 0, 0, (wait) ? REPLY_NO_PORT : 0);
	}
}

// Send hello to a launching process
void LIBFUNC L_IPC_Hello(REG(a0, IPCData *ipc), REG(a1, IPCData *owner))
{
	L_IPC_Command(owner, IPC_HELLO, 0, ipc, 0, REPLY_NO_PORT);
}

struct Goodbye_Packet
{
	IPCData *ipc;
	struct ListLock *list;
};

// Send goodbye to a launching process
void LIBFUNC L_IPC_Goodbye(REG(a0, IPCData *ipc), REG(a1, IPCData *owner), REG(d0, ULONG goodbye_flags))
{
	struct Goodbye_Packet *pkt = 0;

	// If we were in a list, remove ourselves
	ipc_remove_list(ipc);

	// If we're in a list, need a goodbye packet for ListQuit
	if (ipc->list)
	{
		// Allocate and initialise goodbye packet
		if ((pkt = AllocVec(sizeof(struct Goodbye_Packet), 0)))
		{
			pkt->ipc = ipc;
			pkt->list = ipc->list;
		}
	}

	// Send goodbye message to owner
	L_IPC_Command(owner, IPC_GOODBYE, goodbye_flags, ipc, pkt, 0);
}

// Get a goodbye message from an IPC process
IPTR LIBFUNC L_IPC_GetGoodbye(REG(a0, IPCMessage *msg))
{
	return msg->flags;
}

// Tell a list of processes to quit, and optionally wait for them
ULONG LIBFUNC L_IPC_ListQuit(REG(a0, struct ListLock *list),
							 REG(a1, IPCData *owner),
							 REG(d0, ULONG quit_flags),
							 REG(d1, BOOL wait))
{
	IPCData *ipc;
	IPCMessage *msg;
	long count = 0;

	// Lock list
	L_GetSemaphore(&list->lock, SEMF_SHARED, 0);

	// Tell all processes to quit
	for (ipc = (IPCData *)list->list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Tell process to quit
		L_IPC_Quit(ipc, quit_flags, 0);
		++count;
	}

	// Unlock list
	L_FreeSemaphore(&list->lock);

	// Not waiting?
	if (!wait || !owner)
		return (ULONG)count;

	// Loop while the process list is not empty
	while (!(L_IsListLockEmpty(list)))
	{
		// Go through message list
		Forbid();
		for (msg = (IPCMessage *)owner->command_port->mp_MsgList.lh_TailPred; msg->msg.mn_Node.ln_Pred;)
		{
			IPCMessage *next = (IPCMessage *)msg->msg.mn_Node.ln_Pred;

			// Goodbye message, from this list?
			if (msg->command == IPC_GOODBYE)
			{
				struct Goodbye_Packet *pkt;
				struct ListLock *test_list;

				// Got a packet?
				if ((pkt = (struct Goodbye_Packet *)msg->data_free))
				{
					// Get list from packet
					test_list = pkt->list;
				}

				// Otherwise, get list from IPC (dangerous)
				else
					test_list = ((IPCData *)msg->data)->list;

				// See if list matches
				if (test_list == list)
				{
					// Remove message
					Remove((struct Node *)msg);

					// Reply message
					L_IPC_Reply(msg);
				}
			}

			// Get next message
			msg = next;
		}
		Permit();

		// Wait for a message
		Wait(1 << owner->command_port->mp_SigBit);
	}

	return (ULONG)count;
}

// Send a command to list of processes
void LIBFUNC L_IPC_ListCommand(REG(a0, struct ListLock *list),
							   REG(d0, ULONG command),
							   REG(d1, IPTR flags),
							   REG(d2, IPTR data),
							   REG(d3, BOOL wait))
{
	IPCData *ipc;
	struct MsgPort *port = 0;
	long count = 0;

	// If we need replies, create port
	if (wait && (port = CreateMsgPort()))
		port->mp_Node.ln_Pri = PORT_ASYNC_MAGIC;  // original dopus5code: port->mp_Flags|=PF_ASYNC;

	// Lock list
	L_GetSemaphore(&list->lock, SEMF_SHARED, 0);

	// Send command to processes in list
	for (ipc = (IPCData *)list->list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Send command
		if (L_IPC_Command(ipc, command, flags, (APTR)data, 0, port))
			++count;
	}

	// Unlock list
	L_FreeSemaphore(&list->lock);

	// Wait for replies?
	if (port)
	{
		// Loop while there's outstanding messages
		while (count > 0)
		{
			IPCMessage *msg;

			// Wait for a message
			WaitPort(port);

			// Get it
			if ((msg = (IPCMessage *)GetMsg(port)))
			{
				// Clear reply port and free
				msg->msg.mn_ReplyPort = 0;
				L_IPC_Reply(msg);
			}

			// Decrement count
			--count;
		}

		// Free port
		DeleteMsgPort(port);
	}
}

// Remove ourselves from a list
void ipc_remove_list(IPCData *ipc)
{
	// Valid list?
	if (ipc->list && (ipc->flags & IPCF_LISTED))
	{
		L_GetSemaphore(&ipc->list->lock, SEMF_EXCLUSIVE, 0);
		Remove((struct Node *)ipc);
		L_FreeSemaphore(&ipc->list->lock);
		ipc->flags &= ~IPCF_LISTED;
	}
}
