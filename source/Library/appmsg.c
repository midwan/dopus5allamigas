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

// Allocate an AppMessage
DOpusAppMessage *LIBFUNC L_AllocAppMessage(REG(a0, APTR memory), REG(a1, struct MsgPort *reply), REG(d0, short num))
{
	DOpusAppMessage *msg;

	// Allocate message
	if (!(msg = L_AllocMemH(memory, sizeof(DOpusAppMessage) + ((sizeof(struct WBArg) + sizeof(Point)) * num))))
		return 0;

	// Set check pointer
	msg->da_Custom[0] = (IPTR)msg;

	// Set ArgList pointer
	if (num > 0)
		msg->da_Msg.am_ArgList = (struct WBArg *)(msg + 1);
	msg->da_Msg.am_NumArgs = num;

	// Set drop position array pointer
	if (num > 0)
		msg->da_DropPos = (Point *)(msg->da_Msg.am_ArgList + num);

	// Fill out miscellaneous fields
	msg->da_Msg.am_Message.mn_ReplyPort = reply;
	msg->da_Msg.am_Version = AM_VERSION;
	CurrentTime(&msg->da_Msg.am_Seconds, &msg->da_Msg.am_Micros);

	return msg;
}

// Free an AppMessage
void LIBFUNC L_FreeAppMessage(REG(a0, DOpusAppMessage *msg))
{
	// Valid message?
	if (msg)
	{
		short arg;

		// Free any arguments
		if (msg->da_Msg.am_ArgList)
		{
			for (arg = 0; arg < msg->da_Msg.am_NumArgs; arg++)
			{
				UnLock(msg->da_Msg.am_ArgList[arg].wa_Lock);
				L_FreeMemH(msg->da_Msg.am_ArgList[arg].wa_Name);
			}
		}

		// Free message
		L_FreeMemH(msg);
	}
}

// Reply to an AppMessage
void LIBFUNC L_ReplyAppMessage(REG(a0, DOpusAppMessage *msg))
{
	// Valid message?
	if (msg)
	{
		if (msg->da_Msg.am_Message.mn_ReplyPort)
		{
			ReplyMsg((struct Message *)msg);
		}
		else
		{
			L_FreeAppMessage(msg);
		}
	}
}

// Check if a message is a dopus one
BOOL LIBFUNC L_CheckAppMessage(REG(a0, DOpusAppMessage *msg))
{
	// Check check pointer
	return (BOOL)(msg && msg->da_Custom[0] == (IPTR)msg);
}

// Copy a DOpusAppMessage
DOpusAppMessage *LIBFUNC L_CopyAppMessage(REG(a0, DOpusAppMessage *orig), REG(a1, APTR memory))
{
	DOpusAppMessage *msg;
	short arg;

	// Is the original a DOpus message?
	if (!L_CheckAppMessage(orig))
		return 0;

	// Allocate new message
	if (!(msg = L_AllocAppMessage(memory, 0, orig->da_Msg.am_NumArgs)))
		return 0;

	// Copy AppMessage fields
	msg->da_Msg.am_Type = orig->da_Msg.am_Type;
	msg->da_Msg.am_UserData = orig->da_Msg.am_UserData;
	msg->da_Msg.am_ID = orig->da_Msg.am_ID;
	msg->da_Msg.am_Class = orig->da_Msg.am_Class;
	msg->da_Msg.am_MouseX = orig->da_Msg.am_MouseX;
	msg->da_Msg.am_MouseY = orig->da_Msg.am_MouseY;
	msg->da_Msg.am_Seconds = orig->da_Msg.am_Seconds;
	msg->da_Msg.am_Micros = orig->da_Msg.am_Micros;

	// Copy arguments
	for (arg = 0; arg < orig->da_Msg.am_NumArgs; arg++)
	{
		// Copy argument
		L_SetWBArg(msg, arg, orig->da_Msg.am_ArgList[arg].wa_Lock, orig->da_Msg.am_ArgList[arg].wa_Name, memory);
	}

	// Copy Drop-positions
	if (orig->da_DropPos)
		CopyMem((char *)orig->da_DropPos, (char *)msg->da_DropPos, sizeof(Point) * orig->da_Msg.am_NumArgs);

	// Copy other info
	msg->da_DragOffset = orig->da_DragOffset;
	msg->da_Flags = orig->da_Flags;
	msg->da_Custom[1] = orig->da_Custom[1];
	msg->da_Custom[2] = orig->da_Custom[2];
	msg->da_Custom[3] = orig->da_Custom[3];

	return msg;
}

// Set a WBArg entry
BOOL LIBFUNC L_SetWBArg(REG(a0, DOpusAppMessage *msg),
						REG(d0, short num),
						REG(d1, BPTR lock),
						REG(a1, char *name),
						REG(a2, APTR memory))
{
	// Valid message and number?
	if (!msg || num > msg->da_Msg.am_NumArgs)
		return 0;

	// Null-name?
	if (!name)
		name = "";

	// Copy name
	if ((msg->da_Msg.am_ArgList[num].wa_Name = L_AllocMemH(memory, strlen(name) + 1)))
		strcpy(msg->da_Msg.am_ArgList[num].wa_Name, name);

	// Copy lock
	if (lock)
		msg->da_Msg.am_ArgList[num].wa_Lock = DupLock(lock);

	return 1;
}
