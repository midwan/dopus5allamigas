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

#ifndef _DISKCOPY_H
#define _DISKCOPY_H

#define CATCOMP_NUMBERS
#include "diskcopy.strings"
#include "module_deps.h"

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct Screen *screen;
	struct Window *window;
	IPCData *ipc;

	ConfigWindow win_dims;
	NewConfigWindow new_win;
	ObjectList *list;

	Att_List *source_list;
	Att_List *dest_list;

	struct InfoData info;

	char default_verify;
	char default_bump;

	Point window_pos;
	char pos_valid;

	char status_title[120];

	DiskHandle *source;

	struct MsgPort *dest_port;
	char disk_name[32];

	short abort_bit;
} diskcopy_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

BOOL diskcopy_open(diskcopy_data *data, BOOL);
void diskcopy_close(diskcopy_data *data);
void diskcopy_free(diskcopy_data *data);
Att_List *get_device_list(char *, char *);
void show_device_info(diskcopy_data *data, short);
BOOL start_diskcopy(diskcopy_data *data);
void cleanup_diskcopy(diskcopy_data *data);
// BOOL do_diskcopy(diskcopy_data *data,struct Window *status);
BOOL do_diskcopy(diskcopy_data *data, APTR status);

short diskcopy_error(struct Window *, long, long, char *, long, long);

extern ConfigWindow diskcopy_window;
extern ObjectDef diskcopy_objects[];

enum {
	GAD_BASE,

	GAD_DISKCOPY_LAYOUT,

	GAD_DISKCOPY_SOURCE,
	GAD_DISKCOPY_DEST,
	GAD_DISKCOPY_VERIFY,
	GAD_DISKCOPY_BUMP,

	GAD_DISKCOPY_STATUS,
	GAD_DISKCOPY_DISKCOPY,
	GAD_DISKCOPY_CANCEL,
};

void diskcopy_motor(DiskHandle *handle, short on);

void serialise_disk(ULONG *data, unsigned long offset, struct DateStamp *);

#endif
