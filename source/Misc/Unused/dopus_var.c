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

// Get environment variables
void dopus_get_vars(void)
{
	if (GetVar("dopus/dopus", GUI->work_buffer, sizeof(GUI->work_buffer), GVF_GLOBAL_ONLY) > 0)
	{
		// Clock?
		if (strstr(GUI->work_buffer, "-clock-"))
			GUI->flags |= GUIF_CLOCK;

		// Create icons?
		if (strstr(GUI->work_buffer, "-icons-"))
			GUI->flags |= GUIF_SAVE_ICONS;

		// File filter?
		if (strstr(GUI->work_buffer, "-filter-"))
			GUI->flags |= GUIF_FILE_FILTER;

		// Def public screen?
		if (strstr(GUI->work_buffer, "-defpub-"))
			GUI->flags |= GUIF_DEFPUBSCR;

		// View icons?
		if (strstr(GUI->work_buffer, "-showicons-"))
			GUI->flags |= GUIF_VIEW_ICONS;

		// Icon action?
		if (strstr(GUI->work_buffer, "-iconaction-"))
			GUI->flags |= GUIF_ICON_ACTION;

		// Show all?
		if (strstr(GUI->work_buffer, "-showall-"))
			GUI->flags |= GUIF_SHOW_ALL;
	}
