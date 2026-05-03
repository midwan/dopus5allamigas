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

short STDARGS rexx_handler_msg_args(char *handler, DirBuffer *buffer, short flags, IPTR *args)
{
	struct RexxMsg *msg;
	char qualbuf[40];
	short res;
	unsigned long type = 0, count = 0;
	RexxDespatch *desp;
	struct MsgPort *port, *reply_port;
	IPTR *argptr;

	// If no handler supplied, use buffer's handler
	if ((!handler || !*handler) && buffer)
		handler = buffer->buf_CustomHandler;

	// If no custom handler or no ARexx, return
	if (!*handler || !RexxSysBase)
		return 0;

	// Allocate despatch packet
	if (!(desp = AllocVec(sizeof(RexxDespatch), MEMF_CLEAR)))
		return 0;

	// Find our ARexx port
	Forbid();
	port = FindPort(GUI->rexx_port_name);
	Permit();

	// Create message packet
	if (!(msg = CreateRexxMsg(port, 0, GUI->rexx_port_name)))
	{
		Permit();
		FreeVec(desp);
		return 0;
	}

	// Go through arguments
	for (argptr = args; argptr && argptr[0] != TAG_END; argptr += 3)
	{
		ULONG ha_type = (ULONG)argptr[0];
		ULONG ha_arg = (ULONG)argptr[1];
		IPTR ha_data = argptr[2];

		// Argument not too large?
		if (ha_arg > 15)
			continue;

		// Value?
		if (ha_type == HA_Value)
		{
			// Set message slot and conversion key bit
			msg->rm_Args[ha_arg] = ha_data;
			type |= 1 << ha_arg;
		}

		// Qualifier?
		else if (ha_type == HA_Qualifier)
		{
			// Get qualifier
			qualbuf[0] = 0;
			if (ha_data & IEQUAL_ANYSHIFT)
				strcat(qualbuf, "shift ");
			if (ha_data & IEQUAL_ANYALT)
				strcat(qualbuf, "alt ");
			if (ha_data & IEQUALIFIER_CONTROL)
				strcat(qualbuf, "control ");
			if (ha_data & IEQUALIFIER_SUBDROP)
				strcat(qualbuf, "subdrop");

			// Set message slot
			msg->rm_Args[ha_arg] = (IPTR)qualbuf;
		}

		// String
		else
			msg->rm_Args[ha_arg] = ha_data;

		// Fix count
		if (ha_arg > count)
			count = ha_arg;
	}

	// Increment count
	++count;

	// Set count
	msg->rm_Action = count | RXFUNC;  //|RXFF_RESULT;

	// Fill in message
	if (!(FillRexxMsg(msg, count, type)))
	{
		// Failed
		DeleteRexxMsg(msg);
		FreeVec(desp);
		return 0;
	}

	// Fill in despatch packet
	stccpy(desp->handler, handler, sizeof(desp->handler));
	desp->rx_msg = msg;

	// Async?
	if (!(flags & RXMF_SYNC) || !(reply_port = CreateMsgPort()))
	{
		// Send to REXX process
		IPC_Command(GUI->rexx_proc, REXXCMD_SEND_RXMSG, flags, 0, desp, 0);
		return 1;
	}

	// Send message directly
	if ((res = rexx_send_rxmsg_args(desp, flags, reply_port)))
	{
		// Wait for reply
		WaitPort(reply_port);
		GetMsg(reply_port);

		// Get result
		res = msg->rm_Result1;

		// Clear message
		ClearRexxMsg(msg, count);

		// Free message
		DeleteRexxMsg(msg);
	}

	// Free the reply port
	DeleteMsgPort(reply_port);

	// Free despatch
	FreeVec(desp);
	return res;
}
