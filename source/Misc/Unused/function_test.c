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

// TEST function
DOPUS_FUNC(function_test)
{
#ifndef __amigaos3__
	struct Library *ConfigOpusBase;
#endif
	Cfg_Filetype *type, *new;

	// No args?
	if (!instruction->funcargs || !instruction->funcargs->FA_Arguments[0])
		return 0;

	// Find filetype
	if (!(type = filetype_find((char *)instruction->funcargs->FA_Arguments[0])))
		return 0;

	// Open config module
	if (!(ConfigOpusBase = OpenLibrary(config_name, 0)))
	{
		return 0;
	}

	// Edit it
	new = EditFiletype(type, 0, handle->ipc, &main_ipc, 1);

// Close config module
#ifdef __amigaos4__
	DropInterface((struct Interface *)IConfigOpus);
#endif
	CloseLibrary(ConfigOpusBase);

	// Got one?
	if (new)
	{
		D(bug("ok!\n"));
		FreeFiletype(new);
	}

	return 0;
}
