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

// Start notify on a file
struct NotifyRequest *start_file_notify(char *filename, ULONG userdata, struct MsgPort *port)
{
	struct NotifyRequest *notify;

	// Create request
	if (!(notify = AllocMemH(global_memory_pool, sizeof(struct NotifyRequest) + strlen(filename) + 1)))
		return 0;

	// Fill out request structure
	notify->nr_Name = (char *)(notify + 1);
	strcpy(notify->nr_Name, filename);
	notify->nr_UserData = userdata;
	notify->nr_Flags = NRF_SEND_MESSAGE;
	notify->nr_stuff.nr_Msg.nr_Port = port;

	// Start notify
	if (!(StartNotify(notify)))
	{
		// Failed to start
		FreeMemH(notify);
		return 0;
	}

	// No worries
	return notify;
}

// Stop notify on a file
void stop_file_notify(struct NotifyRequest *notify)
{
	// Valid request structure?
	if (notify)
	{
		// End notify
		EndNotify(notify);

		// Free stuff
		FreeMemH(notify);
	}
}
