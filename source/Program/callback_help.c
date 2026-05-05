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

void ASM SAVEDS HookShowHelp(REG(a0, char *file_name), REG(a1, char *node_name))
{
	char filename[256];

	// Filename supplied?
	if (file_name && *file_name)
	{
		// Make string into "DOpus5:Help/" followed by filename
		stccpy(filename, "DOpus5:Help", 256);
		AddPart(filename, FilePart(file_name), 256);

		// Get new pointer
		file_name = filename;
	}

	// Show help
	help_show_help(node_name, file_name);
}
