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
#ifndef _SIMPLEREQUEST_H
#define _SIMPLEREQUEST_H

#include "dopuslib.h"
#include "layout_routines.h"

#ifndef __amigaos3__
	#pragma pack(2)
#endif

#define GAD_POPUP_ID (UWORD) - 7
#define GAD_CHECK_ID (UWORD) - 6
#define GAD_SIGNAL (UWORD) - 5
#define GAD_STRING2_ID (UWORD) - 4
#define GAD_TEXT_ID (UWORD) - 3
#define GAD_STRING_ID (UWORD) - 1

typedef struct
{
	ConfigWindow req_dims;
	NewConfigWindow new_win;
	short gadget_count;
	short max_gadget_width;
	short max_text_width;
	short text_lines;
	short fine_pos, start_pos, width, gad_space, gad_fine_space;
	short object_count;
	struct Window *window;
	ObjectDef *objects;
	ObjectList *objlist;
	unsigned char *keys;
	struct TagItem tags[9], tags2[7], rel_tags[2];
	BOOL strings_2;
	short string_len;
	IPCData *ipc;
	char *string_buffer;
	ULONG waitbits;
	short first_object;
	char **gadgets;
	struct MyLibrary *libbase;
	short signal;
	struct DOpusSimpleRequest *simple;

	APTR parent;
	APTR memory;

	char *string_buffer_2;
	int string_len_2;
} simplereq_data;

#ifndef __amigaos3__
	#pragma pack()
#endif

BOOL _simplereq_open(simplereq_data *data, void *parent);
void _simplereq_close(simplereq_data *data);
BOOL simple_build(simplereq_data *data);

#endif
