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

void file_open_with(struct Window *window, char *name, ULONG flags)
{
	char *buffer;
	short len, wb = 0, ok = 0;
	struct DiskObject *icon;

	// Allocate buffer
	if (!(buffer = AllocVec(1400, MEMF_CLEAR)))
		return;

	// App already selected?
	if (flags & (1 << 30))
	{
		short num;
		struct Node *node;

		// Get application number in list
		num = flags & ~(1 << 30);

		// Lock the list
		lock_listlock(&GUI->open_with_list, 0);

		// Find the node
		for (node = GUI->open_with_list.list.lh_Head; node->ln_Succ; node = node->ln_Succ)
		{
			// Matched?
			if (num-- == 0)
				break;
		}

		// Valid node?
		if (node->ln_Succ)
		{
			// Get the application name
			strcpy(buffer + 1, node->ln_Name);
			ok = 1;
		}

		// Unlock the list
		unlock_listlock(&GUI->open_with_list);
	}

	// Need to ask?
	if (!ok)
	{
		// Build title
		lsprintf(buffer + 1024, GetString(&locale, MSG_SELECT_APP), FilePart(name));

		// Ask for file
		if (!(request_file(window, buffer + 1024, buffer + 1, 0, 0, 0)) || !*(buffer + 1))
		{
			FreeVec(buffer);
			return;
		}

		// Add App to OpenWith list
		add_open_with(buffer + 1);
	}

	// See if app has an icon
	if ((icon = GetDiskObject(buffer + 1)))
	{
		// Is it a tool?
		if (icon->do_Type == WBTOOL)
		{
			// Launch as Workbench App
			wb = 1;
		}

		// Free icon
		FreeDiskObject(icon);
	}

	// Fill in quotes
	buffer[0] = '\"';
	buffer[(len = strlen(buffer))] = '\"';

	// Add filename in quotes
	buffer[len + 1] = ' ';
	buffer[len + 2] = '\"';
	strcpy(buffer + len + 3, name);
	buffer[(len = strlen(buffer))] = '\"';
	buffer[len + 1] = 0;

	// Launch the function
	file_launch(buffer, wb, "ram:");

	// Free buffer
	FreeVec(buffer);
}

// Launch a function from a string
BOOL file_launch(char *name, short wb, char *curdir)
{
	char output[180];
	BPTR file;

	// Workbench launch?
	if (wb)
		return WB_LaunchNew(name, (struct Screen *)-1, 0, environment->env->default_stack, 0);

	// Output window
	lsprintf(output,
			 "%s%s/AUTO/CLOSE/WAIT/SCREEN %s",
			 environment->env->output_device,
			 environment->env->output_window,
			 get_our_pubscreen());

	// Open output
	if (!(file = Open(output, MODE_OLDFILE)))
		file = Open("nil:", MODE_OLDFILE);

	// Run command
	return CLI_Launch(name,
					  (struct Screen *)-1,
					  Lock(curdir, ACCESS_READ),
					  file,
					  0,
					  LAUNCHF_USE_STACK,
					  environment->env->default_stack);
}

// Read the OpenWith file
void startup_read_openwith(void)
{
	APTR file;

	// Lock list
	lock_listlock(&GUI->open_with_list, 1);

	// Open file
	if ((file = OpenBuf("DOpus5:System/OpenWith", MODE_OLDFILE, 4000)))
	{
		char line[256];
		short len;

		// Read lines
		while ((len = ReadBufLine(file, line, 255)) > 0)
		{
			struct Node *node;

			// Allocate node
			if ((node = AllocMemH(global_memory_pool, sizeof(struct Node) + len + 1)))
			{
				// Initialise node and copy string
				node->ln_Name = (char *)(node + 1);
				strcpy(node->ln_Name, line);

				// Add to Open With list
				AddTail((struct List *)&GUI->open_with_list, node);
			}
		}

		// Close file
		CloseBuf(file);
	}

	// Unlock list
	unlock_listlock(&GUI->open_with_list);
}

// Add an entry to the OpenWith file
void add_open_with(char *line)
{
	APTR file;
	struct Node *node;

	// Lock list
	lock_listlock(&GUI->open_with_list, 1);

	// Too many nodes?
	if (Att_NodeCount((Att_List *)&GUI->open_with_list) >= environment->env->settings.max_openwith)
	{
		// List is empty?
		if (IsListEmpty((struct List *)&GUI->open_with_list.list))
			return;

		// Get first node
		node = GUI->open_with_list.list.lh_Head;

		// Remove first node and free it
		RemHead((struct List *)&GUI->open_with_list);
		FreeMemH(node);
	}

	// Allocate a new node
	if ((node = AllocMemH(global_memory_pool, sizeof(struct Node) + strlen(line) + 1)))
	{
		// Initialise node and copy string
		node->ln_Name = (char *)(node + 1);
		strcpy(node->ln_Name, line);

		// Add to Open With list
		AddTail((struct List *)&GUI->open_with_list, node);
	}

	// Open file for output
	if ((file = OpenBuf("DOpus5:System/OpenWith", MODE_NEWFILE, 4000)))
	{
		// Write lines
		for (node = GUI->open_with_list.list.lh_Head; node->ln_Succ; node = node->ln_Succ)
		{
			// Write line
			WriteBuf(file, node->ln_Name, strlen(node->ln_Name));
			WriteBuf(file, "\n", 1);
		}

		// Close file
		CloseBuf(file);
	}

	// Unlock list
	unlock_listlock(&GUI->open_with_list);
}
