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

// Allocate a timer
struct TimerHandle *LIBFUNC L_AllocTimer(REG(d0, ULONG unit), REG(a0, struct MsgPort *port))
{
	TimerHandle *handle;

	// Allocate handle
	if (!(handle = AllocVec(sizeof(TimerHandle), MEMF_CLEAR)))
		return 0;

	// Open message port if we need one
	if (!port)
	{
		if (!(handle->my_port = CreateMsgPort()))
		{
			FreeVec(handle);
			return 0;
		}
		port = handle->my_port;
	}

	// Store port address
	handle->port = port;

	// Open timer
	if (OpenDevice("timer.device", unit, &handle->req.tr_node, 0))
	{
		if (handle->my_port)
			DeleteMsgPort(handle->my_port);
		FreeVec(handle);
		return 0;
	}

	// Initialise timer
	handle->req.tr_node.io_Message.mn_ReplyPort = port;
	handle->req.tr_node.io_Command = TR_ADDREQUEST;
	return handle;
}

// Free a timer
void LIBFUNC L_FreeTimer(REG(a0, TimerHandle *handle))
{
	// Valid handle?
	if (handle)
	{
		// Request active?
		L_StopTimer(handle);

		// Close device
		CloseDevice(&handle->req.tr_node);
		if (handle->my_port)
			DeleteMsgPort(handle->my_port);
		FreeVec(handle);
	}
}

// Start timer
void LIBFUNC L_StartTimer(REG(a0, TimerHandle *handle), REG(d0, ULONG seconds), REG(d1, ULONG micros))
{
	// Valid handle?
	if (handle)
	{
		// Stop existing request
		L_StopTimer(handle);

		// Send new request
		handle->req.tr_time.tv_secs = seconds;
		handle->req.tr_time.tv_micro = micros;
		SendIO(&handle->req.tr_node);
		handle->active = 1;
	}
}

// Check timer
BOOL LIBFUNC L_CheckTimer(REG(a0, TimerHandle *handle))
{
	// Valid handle?
	if (handle)
	{
		// Request active?
		if (handle->active)
		{
			// Check request
			if (CheckIO(&handle->req.tr_node))
			{
				// Yep, done
				WaitIO(&handle->req.tr_node);
				handle->active = 0;
				return 1;
			}
		}
	}

	return 0;
}

// Abort timer
void LIBFUNC L_StopTimer(REG(a0, TimerHandle *handle))
{
	// Valid handle?
	if (handle)
	{
		// Request already active?
		if (handle->active)
		{
			// Abort request
			AbortIO(&handle->req.tr_node);
			WaitIO(&handle->req.tr_node);

			// Clear active flag
			handle->active = 0;

			// Initialise request
			handle->req.tr_node.io_Command = TR_ADDREQUEST;
			handle->req.tr_node.io_Flags = 0;
		}
	}
}

// See if timer is running
BOOL LIBFUNC L_TimerActive(REG(a0, TimerHandle *handle))
{
	// Valid and running?
	return (BOOL)(handle && handle->active);
}

// Get timer base
struct Library *LIBFUNC L_GetTimerBase(REG(a6, struct MyLibrary *lib))
{
#ifdef __amigaos4__
	lib = dopuslibbase_global;
#endif

	return ((struct LibData *)lib->ml_UserData)->TimerBase;
}
