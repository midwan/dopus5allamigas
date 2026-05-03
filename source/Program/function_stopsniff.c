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

// StopSniff internal function
DOPUS_FUNC(function_stopsniffer)
{
	Lister *lister;

	// Get current lister
	if ((lister = function_lister_current(&handle->source_paths)))
	{
		IPCData *ipc;

		// Lock process list
		lock_listlock(&GUI->process_list, FALSE);

		// See if filetypes process is present
		if ((ipc = IPC_FindProc(&GUI->process_list, "dopus_filetype_sniffer", FALSE, 0)))
		{
			// Send abort command
			IPC_Command(ipc, IPC_ABORT, 0, lister, 0, 0);

			// Send break
			Signal((struct Task *)ipc->proc, SIGBREAKF_CTRL_C);
		}

		// Unlock process list
		unlock_listlock(&GUI->process_list);
	}

	return 1;
}
