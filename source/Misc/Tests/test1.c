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

#include "icon.h"

// Get icon flags
ULONG GetIconFlags(struct DiskObject *icon)
{
	ULONG flags;

	// Valid icon?
	if (!icon)
		return 0;

	// Stored in gadget userdata
	flags = (ULONG)icon->do_Gadget.UserData;

	// Mask off flags we're not interested in
	flags &= ICONF_POSITION_OK | ICONF_ICON_VIEW | ICONF_BORDER_OFF | ICONF_NO_LABEL | ICONF_BORDER_ON;
	return flags;
}

// Get Opus icon position
void GetIconPosition(struct DiskObject *icon, short *x, short *y)
{
	// Clear values
	if (x)
		*x = 0;
	if (y)
		*y = 0;

	// Valid icon?
	if (!icon)
		return;

	// Get positions
	if (x)
		*x = ((ULONG)icon->do_Gadget.MutualExclude) >> 16;
	if (y)
		*y = ((ULONG)icon->do_Gadget.MutualExclude) & 0xffff;
}

// Set icon flags
void SetIconFlags(struct DiskObject *icon, ULONG flags)
{
	ULONG oldflags;

	// Valid icon?
	if (!icon)
		return;

	// Get old flags from gadget
	oldflags = (ULONG)icon->do_Gadget.UserData;

	// Clear flags we're interested in
	oldflags &= ~(ICONF_POSITION_OK | ICONF_ICON_VIEW | ICONF_BORDER_OFF | ICONF_NO_LABEL | ICONF_BORDER_ON);

	// Or new flags in
	oldflags |= flags;

	// Store back in gadget
	icon->do_Gadget.UserData = (APTR)oldflags;
}

// Set Opus icon position
void SetIconPosition(struct DiskObject *icon, short x, short y)
{
	// Valid icon?
	if (!icon)
		return;

	// Set position
	*((ULONG *)&icon->do_Gadget.MutualExclude) = (x << 16) | y;
}

// Get the icon type
short GetIconType(struct DiskObject *icon)
{
	if (!icon)
		return 0;

	if (icon->do_Gadget.SpecialInfo == (APTR)icon)
		return ICON_CACHED;
	else if (icon->do_Gadget.SpecialInfo == (APTR)(icon + 1))
		return ICON_NEWICON;

	return ICON_NORMAL;
}

void main(int argc, char **argv)
{
	char *name;

	if (argc == 2)
	{
		name = argv[1];
		printf("icon %s\n", name);
	}
}
