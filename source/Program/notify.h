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

#ifndef _DOPUS_NOTIFY
#define _DOPUS_NOTIFY

#define NOTIFY_FILETYPES_CHANGED (ULONG) - 1
#define NOTIFY_PATTERN_CHANGED (ULONG) - 2
#define NOTIFY_FONT_CHANGED (ULONG) - 3
#define NOTIFY_MODULES_CHANGED (ULONG) - 4
#define NOTIFY_ENV_CHANGED (ULONG) - 5
#define NOTIFY_DESKTOP_CHANGED (ULONG) - 6
#define NOTIFY_COMMANDS_CHANGED (ULONG) - 7

// Prototypes
struct NotifyRequest *start_file_notify(char *filename, ULONG userdata, struct MsgPort *port);
void stop_file_notify(struct NotifyRequest *notify);

#endif
