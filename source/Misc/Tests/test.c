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

#include <proto/dopus5.h>
#include "dopus.h"

struct Library *DOpusBase;

#define SetFlag(v, f) ((v) |= (f))
#define ClearFlag(v, f) ((v) &= ~(f))
#define ToggleFlag(v, f) ((v) ^= (f))
#define FlagIsSet(v, f) (((v) & (f)) != 0)
#define FlagIsClear(v, f) (((v) & (f)) == 0)

void main(int argc, char **argv)
{
	char *name;
	struct DiskObject *icon;
	LONG flags;

	if (DOpusBase = OpenLibrary("dopus5.library", 0))
	{
		if (argc > 1)
		{
			name = argv[1];
			printf("icon %s\n", name);

			if (IconBase->lib_Version >= 44)
				icon = GetIconTags(name, ICONGETA_FailIfUnavailable, TRUE, TAG_DONE);
			else
				icon = GetDiskObject(name);

			if (icon)
			{
				ULONG mx, ud;

				ud = (ULONG)icon->do_Gadget.UserData;
				mx = icon->do_Gadget.MutualExclude;

				printf("UserData %lx MutualExclude %lx\n", ud, mx);

				flags = GetIconFlags(icon);
				printf("%lx\n", flags);

				printf("ICONF_POSITION_OK %lx\n", flags & ICONF_POSITION_OK);
				printf("ICONF_ICON_VIEW %lx\n", flags & ICONF_ICON_VIEW);
				printf("ICONF_BORDER_OFF %lx\n", flags & ICONF_BORDER_OFF);
				printf("ICONF_NO_LABEL %lx\n", flags & ICONF_NO_LABEL);
				printf("ICONF_BORDER_ON %lx\n", flags & ICONF_BORDER_ON);

				if (argc == 3)
				{
					if (!strcmp(argv[2], "on"))
					{
						printf("ON\n");

						ClearFlag(flags, ICONF_BORDER_OFF);
						SetFlag(flags, ICONF_BORDER_ON);
					}

					else
					{
						printf("OFF\n");

						SetFlag(flags, ICONF_BORDER_OFF);
						ClearFlag(flags, ICONF_BORDER_ON);
					}

					ud = (ULONG)icon->do_Gadget.UserData;
					mx = icon->do_Gadget.MutualExclude;

					printf("UserData %lx MutualExclude %lx\n", ud, mx);

					printf("%lx\n", flags);

					printf("ICONF_BORDER_OFF %lx\n", flags & ICONF_BORDER_OFF);
					printf("ICONF_NO_LABEL %lx\n", flags & ICONF_NO_LABEL);
					printf("ICONF_BORDER_ON %lx\n", flags & ICONF_BORDER_ON);

					SetIconFlags(icon, flags);
					PutDiskObject(name, icon);
				}

				FreeDiskObject(icon);
			}
		}

		CloseLibrary(DOpusBase);
	}
}
