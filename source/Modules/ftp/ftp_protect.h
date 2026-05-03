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

#ifndef _FTP_PROTECT_H
#define _FTP_PROTECT_H

// Protect GUI message
#ifndef __amigaos3__
	#pragma pack(2)
#endif
struct protectgui_msg
{
	struct Window *pm_window;
	char *pm_name;
	ULONG pm_current;
	ULONG pm_set_mask;
	ULONG pm_clear_mask;
};
#ifndef __amigaos3__
	#pragma pack()
#endif

// Get protection
int function_change_get_protect(struct opusftp_globals *og, struct protectgui_msg *pm);

#endif
