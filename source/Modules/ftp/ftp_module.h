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

#ifndef _FTP_MOD_H
#define _FTP_MOD_H

#define CATCOMP_NUMBERS
#include "ftp.strings"

#include <proto/dopus5.h>
#include <proto/module.h>

// This is what 'dopus_ftp' gets sent when we launch it
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct modlaunch_data
{
	IPCData *mld_ftp_ipc;  // dopus_ftp's ipc
	BOOL mld_okay;		   // proc_init() sets this to TRUE so it can be checked after calling IPC_Launch() */

	struct Screen *mld_screen;	// Screen we're on
	IPCData *mld_opus_ipc;		// The main Opus process's ipc
	IPCData *mld_function_ipc;	// dopus_function's ipc, needed for callback hooks
	EXT_FUNC(mld_func_callback);
#ifndef __amigaos3__
	#pragma pack()
#endif

	struct opusftp_globals *mld_og;	 // Points back to global info
};

#endif
