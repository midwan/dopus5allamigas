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
#ifndef _IPC_H
#define _IPC_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	struct Message msg;
	ULONG command;
	ULONG flags;
	APTR data;
	APTR data_free;
} IPCMessage;

typedef struct
{
	struct MinNode node;
	struct Process *proc;		   // Process pointer
	IPCMessage startup_msg;		   // Startup message
	struct MsgPort *command_port;  // Port to send commands to
	struct ListLock *list;		   // List we're a member of
} IPCData;

#ifndef __amigaos3__
	#pragma pack()
#endif

enum {
	IPC_COMMAND_BASE = 0x8000000,
	IPC_STARTUP,
	IPC_ABORT,
	IPC_QUIT,
	IPC_ACTIVATE,
	IPC_NEW,
};

// protos
ipc_launch(struct ListLock *, IPCData **, char *, ULONG, ULONG, ULONG);
ipc_startup(IPCData *ipc, APTR data, struct MsgPort *reply);
ULONG ipc_command(IPCData *ipc, ULONG command, ULONG flags, APTR data, APTR data_free, struct MsgPort *reply);
void ipc_reply(IPCMessage *msg);
void ipc_free(IPCData *);
void ipc_free_port(IPCData *ipc);
IPCData *ipc_find_proc(struct ListLock *, char *);
IPCData *ipc_proc_startup(ULONG *, ULONG (*)(IPCData *, APTR));

#endif
