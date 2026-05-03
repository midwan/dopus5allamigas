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

#include "dopuslib.h"

// Add a notify request
APTR LIBFUNC L_AddNotifyRequest(REG(d0, ULONG type),
								REG(d1, ULONG userdata),
								REG(a0, struct MsgPort *port),
								REG(a6, struct MyLibrary *libbase))
{
	NotifyNode *node;
	struct LibData *data;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Allocate a new node
	if (!(node = AllocVec(sizeof(NotifyNode), MEMF_CLEAR)))
		return 0;

	// Fill in node
	node->port = port;
	node->userdata = userdata;
	node->type = type;

	// Get data pointer
	data = (struct LibData *)libbase->ml_UserData;

	// Lock notify list
	ObtainSemaphore(&data->notify_lock);

	// Add to list
	AddTail((struct List *)&data->notify_list, (struct Node *)node);

	// Unlock notify list
	ReleaseSemaphore(&data->notify_lock);
	return node;
}

// Remove a notify request
void LIBFUNC L_RemoveNotifyRequest(REG(a0, NotifyNode *node), REG(a6, struct MyLibrary *libbase))
{
	DOpusNotify *msg;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Valid node?
	if (!node)
		return;

	// Lock notify list
	L_GetSemaphore(&((struct LibData *)libbase->ml_UserData)->notify_lock, SEMF_EXCLUSIVE, 0);

	// Remove node
	Remove((struct Node *)node);

	// Forbid to examine port
	Forbid();

	// Go through messages in port
	for (msg = (DOpusNotify *)node->port->mp_MsgList.lh_Head; msg->dn_Msg.mn_Node.ln_Succ;)
	{
		DOpusNotify *next = (DOpusNotify *)msg->dn_Msg.mn_Node.ln_Succ;

		// Notify message?
		if (msg->dn_Msg.mn_Node.ln_Type == NT_DOPUS_NOTIFY && (node->type & msg->dn_Type))
		{
			// Remove and free this message
			Remove((struct Node *)msg);
			FreeVec(msg);
		}

		// Get next
		msg = next;
	}

	// Enable multitasking
	Permit();

	// Unlock notify list
	L_FreeSemaphore(&((struct LibData *)libbase->ml_UserData)->notify_lock);

	// Free node
	FreeVec(node);
}

// Send a notify message
void LIBFUNC L_SendNotifyMsg(REG(d0, ULONG type),
							 REG(d1, ULONG data),
							 REG(d2, ULONG flags),
							 REG(d3, short wait),
							 REG(a0, char *name),
							 REG(a1, struct FileInfoBlock *fib),
							 REG(a6, struct MyLibrary *libbase))
{
	NotifyNode *node;
	struct Task *this_task = 0;
	struct MsgPort reply_port;
	short count = 0;

#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Initialise fake reply port if needed
	if (wait)
	{
		// Get current task
		this_task = FindTask(0);

		// Fill out port
		reply_port.mp_Flags = PA_SIGNAL;
		reply_port.mp_SigBit = SIGB_INTUITION;
		reply_port.mp_SigTask = this_task;
		NewList(&reply_port.mp_MsgList);
	}

	// Lock notify list
	L_GetSemaphore(&((struct LibData *)libbase->ml_UserData)->notify_lock, SEMF_SHARED, 0);

	// Go through notify list
	for (node = (NotifyNode *)((struct LibData *)libbase->ml_UserData)->notify_list.mlh_Head; node->node.mln_Succ;
		 node = (NotifyNode *)node->node.mln_Succ)
	{
		DOpusNotify *msg;

		// Correct type?
		if (node->type & type)
		{
			short size;

			// Calculate size
			size = sizeof(DOpusNotify);
			if (name)
				size += strlen(name);
			if (fib)
				size += sizeof(struct FileInfoBlock) + 1;

			// Allocate message
			if ((msg = AllocVec(size, MEMF_CLEAR)))
			{
				// Initialise message
				msg->dn_Msg.mn_Node.ln_Type = NT_DOPUS_NOTIFY;
				msg->dn_Msg.mn_Length = size;
				msg->dn_Type = type;
				msg->dn_Data = data;
				msg->dn_Flags = flags;
				msg->dn_UserData = node->userdata;
				if (name)
					strcpy(msg->dn_Name, name);

				// Got FileInfoBlock?
				if (fib)
				{
					char *ptr;

					// Get pointer to end of name
					ptr = msg->dn_Name + strlen(msg->dn_Name) + 1;
					if (((ULONG)ptr) & 1)
						++ptr;

					// Set pointer and copy it
					msg->dn_Fib = (struct FileInfoBlock *)ptr;
					CopyMem((char *)fib, ptr, sizeof(struct FileInfoBlock));
				}

				// Wait for reply?
				if (wait)
				{
					// Prevent deadlocks
					if (node->port->mp_SigTask != this_task)
					{
						// Set reply port address
						msg->dn_Msg.mn_ReplyPort = &reply_port;

						// Increment count
						++count;
					}
				}

				// Send message
				PutMsg(node->port, (struct Message *)msg);
			}
		}
	}

	// Unlock notify list
	L_FreeSemaphore(&((struct LibData *)libbase->ml_UserData)->notify_lock);

	// Wait for replies?
	if (wait && count)
	{
		// While outstanding replies
		while (count > 0)
		{
			DOpusNotify *msg;

			// Get replies
			WaitPort(&reply_port);
			while ((msg = (DOpusNotify *)GetMsg(&reply_port)))
			{
				// Free message
				FreeVec(msg);

				// Decrement count
				--count;
			}
		}
	}
}

// Change notification flags
void LIBFUNC L_SetNotifyRequest(REG(a0, NotifyNode *node),
								REG(d0, ULONG new_flags),
								REG(d1, ULONG mask),
								REG(a6, struct MyLibrary *libbase))
{
#ifdef __amigaos4__
	libbase = dopuslibbase_global;
#endif

	// Valid node?
	if (!node)
		return;

	// Set new flag values
	node->type = (node->type & ~mask) | new_flags;
}
