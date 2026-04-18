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

#include "callback_main.h"

STATIC CONST ULONG HookTable[DOPUS_HOOK_COUNT] = {
	GET_DOPUS_CALLBACK(HookCreateFileEntry), GET_DOPUS_CALLBACK(HookFileSet),
	GET_DOPUS_CALLBACK(HookSortFileList),	 GET_DOPUS_CALLBACK(HookAddFileEntry),
	GET_DOPUS_CALLBACK(HookResortLister),	 GET_DOPUS_CALLBACK(HookRefreshLister),
	GET_DOPUS_CALLBACK(HookLockFileList),	 GET_DOPUS_CALLBACK(HookUnlockFileList),
	GET_DOPUS_CALLBACK(HookFindFileEntry),	 GET_DOPUS_CALLBACK(HookSetFileComment),
	GET_DOPUS_CALLBACK(HookRemoveFileEntry), GET_DOPUS_CALLBACK(HookFileQuery),
	GET_DOPUS_CALLBACK(HookShowHelp),		 GET_DOPUS_CALLBACK(HookConvertEntry),
	GET_DOPUS_CALLBACK(HookGetLister),		 GET_DOPUS_CALLBACK(HookGetSource),
	GET_DOPUS_CALLBACK(HookNextSource),		 GET_DOPUS_CALLBACK(HookUnlockSource),
	GET_DOPUS_CALLBACK(HookGetDest),		 GET_DOPUS_CALLBACK(HookEndSource),
	GET_DOPUS_CALLBACK(HookEndDest),		 GET_DOPUS_CALLBACK(HookGetEntry),
	GET_DOPUS_CALLBACK(HookExamineEntry),	 GET_DOPUS_CALLBACK(HookEndEntry),
	GET_DOPUS_CALLBACK(HookRemoveEntry),	 GET_DOPUS_CALLBACK(HookEntryCount),
	GET_DOPUS_CALLBACK(HookReloadEntry),	 GET_DOPUS_CALLBACK(HookAddFile),
	GET_DOPUS_CALLBACK(HookDelFile),		 GET_DOPUS_CALLBACK(HookLoadFile),
	GET_DOPUS_CALLBACK(HookDoChanges),		 GET_DOPUS_CALLBACK(HookCheckAbort),
	GET_DOPUS_CALLBACK(HookGetWindow),		 GET_DOPUS_CALLBACK(HookGetPort),
	GET_DOPUS_CALLBACK(HookGetScreen),		 GET_DOPUS_CALLBACK(HookGetScreenData),
	GET_DOPUS_CALLBACK(HookFreeScreenData),	 GET_DOPUS_CALLBACK(HookOpenProgress),
	GET_DOPUS_CALLBACK(HookUpdateProgress),	 GET_DOPUS_CALLBACK(HookCloseProgress),
	GET_DOPUS_CALLBACK(HookReplaceReq),		 GET_DOPUS_CALLBACK(HookGetPointer),
	GET_DOPUS_CALLBACK(HookFreePointer),	 GET_DOPUS_CALLBACK(HookSendCommand),
	GET_DOPUS_CALLBACK(HookCheckDesktop),	 GET_DOPUS_CALLBACK(HookGetDesktop),
	GET_DOPUS_CALLBACK(HookScript),			 GET_DOPUS_CALLBACK(HookDesktopPopup),
	GET_DOPUS_CALLBACK(HookFirstEntry),		 GET_DOPUS_CALLBACK(HookRexxCommand),
	GET_DOPUS_CALLBACK(HookFileRequest),	 GET_DOPUS_CALLBACK(HookGetThemes),
};

long HookInitHooks(DOpusCallbackInfo *info)
{
	ULONG *srcptr, *dstptr;
	short num;

	// Get pointer to start of function table
	srcptr = (ULONG *)HookTable;

	// Get pointer to storage space in structure
	dstptr = (ULONG *)&info->dc_CreateFileEntry;

	// Fill it out, up to count entries
	for (num = 0; num < info->dc_Count; num++, srcptr++, dstptr++)
	{
		// Copy entry if valid
		if (num < DOPUS_HOOK_COUNT)
			*dstptr = *srcptr;

		// Just clear entry in table
		else
			*dstptr = 0;
	}

	return (num > DOPUS_HOOK_COUNT) ? DOPUS_HOOK_COUNT : num;
}

// Run a script
BOOL ASM SAVEDS HookScript(REG(a0, char *name), REG(a1, char *data))
{
	// Run the script
	return RunScript_Name(name, data);
}

// Send an ARexx command
long ASM SAVEDS HookRexxCommand(REG(a0, char *command),
								REG(a1, char *result),
								REG(d0, long length),
								REG(a2, struct MsgPort *replyport),
								REG(d1, ULONG flags))
{
	struct MsgPort *rexx, *reply;
	struct RexxMsg *msg;
	long rc = -1;

	// Clear result
	if (result)
		*result = 0;

	// Get rexx port
	if (!GUI->rexx_proc || !(rexx = (struct MsgPort *)IPCDATA(GUI->rexx_proc)))
		return -1;

	// Create reply port if needed
	if ((reply = replyport) || (reply = CreateMsgPort()))
	{
		// Create message
		if ((msg = BuildRexxMsgExTags(reply, ".dopus5", rexx->mp_Node.ln_Name, RexxTag_Arg0, (IPTR)command, TAG_END)))
		{
			// Initialise message
			msg->rm_Action |= 1 | RXCOMM;
			if (result)
				msg->rm_Action |= RXFF_RESULT;

			// Send the message and wait for reply
			PutMsg(rexx, (struct Message *)msg);
			WaitPort(reply);
			GetMsg(reply);

			// String reply?
			if (msg->rm_Result2 && result)
				stccpy(result, (char *)msg->rm_Result2, length);
			rc = msg->rm_Result1;

			// Free the result
			if (msg->rm_Result1 == 0 && msg->rm_Result2 != 0)
				DeleteArgstring((char *)msg->rm_Result2);

			// Free message
			FreeRexxMsgEx(msg);
		}

		// Free message port
		if (reply != replyport)
			DeleteMsgPort(reply);
	}

	return rc;
}

// Request a file
long ASM SAVEDS HookFileRequest(REG(a0, struct Window *parent),
								REG(a1, char *title),
								REG(a2, char *initial_path),
								REG(a3, char *pathname),
								REG(d0, ULONG flags),
								REG(d1, char *pattern))
{
	return request_file(parent, title, pathname, initial_path, flags, pattern);
}
