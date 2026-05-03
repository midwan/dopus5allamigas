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

BOOL desktop_find_hidden(char *, BOOL);
Cfg_Desktop *desktop_snapshot_icon(BackdropObject *icon, short x, short y);
Cfg_Desktop *desktop_find_icon(char *name, ULONG *pos);
BOOL desktop_icon_path(BackdropObject *icon, char *path, short len, BPTR our_lock);
void desktop_refresh_icons(BackdropInfo *info, BOOL);

void desktop_delete(IPCData *, BackdropInfo *, BackdropObject *);

void desktop_drop(BackdropInfo *, struct _DOpusAppMessage *, UWORD);

void notify_disk_name_change(BackdropInfo *, char *, char *);
void backdrop_update_disk(BackdropInfo *info, devname_change *change, BOOL show);

BOOL desktop_add_hidden(char *name);

BOOL desktop_drop_on_object(BackdropInfo *, DOpusAppMessage **, BackdropObject *, UWORD);

short read_desktop_folder(BackdropInfo *, BOOL);
