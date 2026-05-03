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

Att_List *script_list = 0;

static char *script_names[] = {"Startup",
							   "Shutdown",
							   "Hide",
							   "Reveal",
							   "Disk inserted",
							   "Disk removed",
							   "Double-click",
							   "Open lister",
							   "Open buttons",
							   "Open group",
							   "Close lister",
							   "Close buttons",
							   "Close group",
							   "Bad disk inserted",
							   "Right double-click",
							   "Middle double-click",
							   "Pre-Startup",
							   "AnsiRead",
							   "HexRead",
							   "IconInfo",
							   "Play",
							   "Print",
							   "Read",
							   "Show",
							   "SmartRead",
							   0};

static UWORD script_flags[] = {SCRIPTF_STARTUP,					 // Startup
							   SCRIPTF_STARTUP | SCRIPTF_SYNC,	 // Shutdown
							   0,								 // Hide
							   0,								 // Reveal
							   0,								 // Disk inserted
							   0,								 // Disk removed
							   SCRIPTF_NO_SOUND,				 // Double-click
							   SCRIPTF_NO_LOCK,					 // Open lister
							   0,								 // Open buttons
							   0,								 // Open group
							   0,								 // Close lister
							   0,								 // Close buttons
							   0,								 // Close group
							   0,								 // Bad disk inserted
							   SCRIPTF_NO_SOUND,				 // Right double-click
							   SCRIPTF_NO_SOUND,				 // Middle double-click
							   SCRIPTF_NO_SOUND | SCRIPTF_SYNC,	 // Pre-Startup
							   SCRIPTF_NO_SOUND,				 // AnsiRead
							   SCRIPTF_NO_SOUND,				 // HexRead
							   SCRIPTF_NO_SOUND,				 // IconInfo
							   SCRIPTF_NO_SOUND,				 // Play
							   SCRIPTF_NO_SOUND,				 // Print
							   SCRIPTF_NO_SOUND,				 // Read
							   SCRIPTF_NO_SOUND,				 // Show
							   SCRIPTF_NO_SOUND};				 // SmartRead

// Initialise scripts
void InitScripts(void)
{
	short num, error;
	struct AnchorPath *anchor;

	// Create list
	if (!(script_list = Att_NewList(LISTF_POOL)))
		return;

	// Add internal scripts
	for (num = 0; script_names[num]; num++)
		Att_NewNode(script_list, script_names[num], script_flags[num], ADDNODE_EXCLUSIVE);

	// Allocate anchor path
	if (!(anchor = AllocVec(sizeof(struct AnchorPath) + 256, MEMF_CLEAR)))
		return;

	// Initialise anchor path
	anchor->ap_Strlen = 256;
	anchor->ap_Flags = APF_DOWILD;

	// Search for scripts
	error = MatchFirst("dopus5:system/scripts/#?.scp", anchor);

	// Continue while there's files
	while (!error)
	{
		char *buffer;

		// Allocate buffer
		if ((buffer = AllocVec(anchor->ap_Info.fib_Size + 1, MEMF_CLEAR)))
		{
			BPTR file;

			// Read file
			if ((file = Open(anchor->ap_Buf, MODE_OLDFILE)) && Read(file, buffer, anchor->ap_Info.fib_Size) > 0)
			{
				long pos, start, size = anchor->ap_Info.fib_Size;

				// Go through buffer
				for (pos = 0, start = 0; pos < size; pos++)
				{
					// End of line?
					if (buffer[pos] == '\n' || pos == size - 1)
					{
						char *ptr;
						short flags = 0;

						// Valid line?
						if (pos > start)
						{
							// Null-terminate
							if (buffer[pos] == '\n')
								buffer[pos] = 0;

							// Find comma
							if ((ptr = strchr(buffer + start, ',')))
							{
								// Strip comma
								*ptr = 0;

								// Get flags
								flags = atoi(ptr + 1);
							}

							// Create node for script
							Att_NewNode(script_list, buffer + start, flags, ADDNODE_EXCLUSIVE);
						}

						// Bump start position
						start = pos + 1;
					}
				}
			}

			// Close file
			if (file)
				Close(file);

			// Free buffer
			FreeVec(buffer);
		}

		// Find next file in directory
		anchor->ap_Flags &= ~APF_DODIR;
		error = MatchNext(anchor);
	}

	// Clean up match stuff
	MatchEnd(anchor);
	FreeVec(anchor);
}

// Free scripts
void FreeScripts(void)
{
	Att_RemList(script_list, 0);
	script_list = 0;
}

BOOL RunScript(short id, char *data)
{
	short num;
	Att_Node *node;

	// No scripts?
	if (!script_list)
		return 0;

	// Find internal script
	for (num = 0, node = (Att_Node *)script_list->list.lh_Head; num < id && node->node.ln_Succ;
		 num++, node = (Att_Node *)node->node.ln_Succ)
		;

	// Found it?
	if (node->node.ln_Succ)
	{
		// Run it
		return RunScript_Node(node, data);
	}

	// Not found
	return 0;
}

BOOL RunScript_Name(char *name, char *data)
{
	Att_Node *node;

	// No scripts?
	if (!script_list)
		return 0;

	// Find script
	if ((node = (Att_Node *)FindNameI((struct List *)script_list, name)))
		return RunScript_Node(node, data);

	return 0;
}

BOOL RunScript_Node(Att_Node *node, char *data)
{
	Cfg_ButtonFunction *func;
	struct Message reply;
	struct MsgPort *reply_port = 0;
	ULONG flags = 0;
	Cfg_SoundEntry *sound;

	// Lock sound list
	GetSemaphore(&environment->sound_lock, SEMF_SHARED, 0);

	// See if there's a sound configured for this event
	if ((sound = (Cfg_SoundEntry *)FindNameI((struct List *)&environment->sound_list, node->node.ln_Name)))
	{
		if (sound->dse_Sound[0])
		{
			// Don't play other sounds before the startup script
			if (!(environment->env->settings.general_flags & GENERALF_EXCLUSIVE_STARTUP_SND) ||
				(GUI->flags & GUIF_DONE_STARTUP) || (node->data & SCRIPTF_STARTUP))
			{
				Cfg_Function *func;
				char command[400], *ptr;
				short vol;

				// Get sound pointer
				ptr = (sound->dse_Random[0]) ? sound->dse_Random : sound->dse_Sound;
				vol = (sound->dse_Random[0] && sound->dse_RandomVolume) ? sound->dse_RandomVolume : sound->dse_Volume;

				// Build command
				lsprintf(command, "play \"%s\" quiet vol %ld", ptr, vol);
				if ((func = new_default_function(command, 0)))
				{
					func->function.flags2 |= FUNCF2_FREE_FUNCTION;
					function_launch(FUNCTION_RUN_FUNCTION, func, 0, FUNCF_NO_SOURCE, 0, 0, 0, 0, 0, 0, 0);
				}
			}
		}
	}

	// Unlock sound list
	FreeSemaphore(&environment->sound_lock);

	// Find script function
	if (!(func = FindScript_Function(node->node.ln_Name)))
		return 0;

	// If this is a synchronous script, we need to wait for reply
	if (node->data & SCRIPTF_SYNC)
	{
		// Create reply port
		if ((reply_port = CreateMsgPort()))
		{
			// Fill out reply message
			reply.mn_ReplyPort = reply_port;
		}
	}

	// Get flags
	if (node->data & SCRIPTF_NO_LOCK)
		flags = FUNCF_NO_SOURCE;
	if (reply_port)
		flags |= FUNCF_SYNC;

	// Run function
	function_launch(FUNCTION_RUN_FUNCTION,
					(Cfg_Function *)func,
					0,
					flags,
					0,
					0,
					0,
					0,
					BuildArgArray(data, 0),
					(reply_port) ? &reply : 0,
					0);

	// Wait for reply if needed
	if (reply_port)
	{
		WaitPort(reply_port);
		DeleteMsgPort(reply_port);
	}

	// Unlock script lock
	FreeSemaphore(&GUI->scripts_lock);
	return 1;
}

Cfg_ButtonFunction *FindScript_Function(char *name)
{
	Cfg_Button *button;
	Cfg_ButtonFunction *func;

	// Got a script bank?
	if (!GUI->scripts)
		return 0;

	// Lock script lock
	GetSemaphore(&GUI->scripts_lock, SEMF_SHARED, 0);

	// Go through buttons
	for (button = (Cfg_Button *)GUI->scripts->buttons.lh_Head; button->node.ln_Succ;
		 button = (Cfg_Button *)button->node.ln_Succ)
	{
		// Get function
		if ((func = (Cfg_ButtonFunction *)FindFunctionType((struct List *)&button->function_list, FTYPE_LEFT_BUTTON)))
		{
			// Empty function?
			if (IsListEmpty((struct List *)&func->instructions))
				continue;

			// Match name
			if ((func->label && stricmp(func->label, name) == 0) ||
				(func->node.ln_Name && stricmp(func->node.ln_Name, name) == 0))
			{
				return func;
			}
		}
	}

	// Unlock script lock
	FreeSemaphore(&GUI->scripts_lock);
	return 0;
}

void InitSoundEvents(BOOL force)
{
	Cfg_SoundEntry *sound;
	char buf[512];

	// Lock sound list
	GetSemaphore(&environment->sound_lock, SEMF_SHARED, 0);

	// Go through sounds
	for (sound = (Cfg_SoundEntry *)environment->sound_list.mlh_Head; sound->dse_Node.ln_Succ;
		 sound = (Cfg_SoundEntry *)sound->dse_Node.ln_Succ)
	{
		// If sound is empty, make sure random buffer is too
		if (!sound->dse_Sound[0])
		{
			sound->dse_Random[0] = 0;
			sound->dse_RandomVolume = 0;
		}

		// Otherwise, see if sound has changed
		else if (sound->dse_Flags & CFGSEF_CHANGED || force)
		{
			D_S(struct FileInfoBlock, fib)
			D_S(struct AnchorPath, anchor)
			Att_List *list;
			Att_Node *node = 0;
			short count = 0, num, wild = 0, err;
			char *ptr;
			APTR file;

			// Clear change flag and initialise random buffer
			sound->dse_Flags &= ~CFGSEF_CHANGED;
			sound->dse_Random[0] = 0;
			sound->dse_RandomVolume = 0;

			// Check for wildcard characters
			if ((ptr = FilePart(sound->dse_Sound)) && (strchr(ptr, '*') || strchr(ptr, '?') || strchr(ptr, '#')))
				wild = 1;

			// If supplied picture is a file (or doesn't exist), use the name as given
			else if (!GetFileInfo(sound->dse_Sound, fib) || fib->fib_DirEntryType < 0)
				continue;

			// Build name to search directory
			strcpy(buf, sound->dse_Sound);
			if (!wild)
				AddPart(buf, "*", 300);

			// Build list of matching files
			anchor->ap_BreakBits = 0;
			anchor->ap_Flags = 0;
			anchor->ap_Strlen = 0;
			if ((err = MatchFirst(buf, anchor)) || !(list = Att_NewList(LISTF_POOL)))
			{
				if (!err)
					MatchEnd(anchor);
				continue;
			}
			while (!err)
			{
				if (anchor->ap_Info.fib_DirEntryType < 0)
				{
					if (Att_NewNode(list, anchor->ap_Info.fib_FileName, atoi(anchor->ap_Info.fib_Comment), 0))
						++count;
				}
				else
					anchor->ap_Flags &= ~APF_DODIR;
				err = MatchNext(anchor);
			}
			MatchEnd(anchor);

			// No sounds?
			if (count < 1)
			{
				Att_RemList(list, 0);
				continue;
			}

			// See what the last random sound was
			lsprintf(buf, "dopus5:system/rnd.%s", sound->dse_Node.ln_Name);
			if ((file = OpenBuf(buf, MODE_OLDFILE, 512)))
			{
				ReadBufLine(file, buf + 200, 256);
				CloseBuf(file);
			}
			else
				buf[200] = 0;

			// Pick random sound
			while (!node)
			{
				num = RandomDopus(8193);
				num %= count;
				if (!(node = Att_FindNode(list, num)))
					break;

				// Check it's not the one used last time
				if (stricmp(node->node.ln_Name, FilePart(buf + 200)) != 0 || count == 1)
					break;
				node = 0;
			}

			// Got sound?
			if (node)
			{
				// Save name of sound so it won't be used next time
				strcpy(buf + 200, sound->dse_Sound);
				if (wild && (ptr = FilePart(buf + 200)))
					*ptr = 0;
				AddPart(buf + 200, node->node.ln_Name, 256);
				if ((file = OpenBuf(buf, MODE_NEWFILE, 512)))
				{
					WriteBuf(file, buf + 200, strlen(buf + 200) + 1);
					CloseBuf(file);
				}

				// Store random name and volume
				strcpy(sound->dse_Random, buf + 200);
				if ((sound->dse_RandomVolume = node->data) < 0 || sound->dse_RandomVolume > 64)
					sound->dse_RandomVolume = 0;
			}

			// Free list
			Att_RemList(list, 0);
		}
	}

	// Unlock sound list
	FreeSemaphore(&environment->sound_lock);
}
