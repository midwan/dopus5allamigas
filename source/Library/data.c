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

#include "dopuslib.h"

struct TextFont *topaz_font;

struct List image_list;
struct SignalSemaphore image_lock;
APTR image_memory;
// APTR					chip_memory;

Class *listview_class, *image_class, *button_class, *palette_class, *string_class, *check_class, *view_class,
	*frame_class, *gauge_class;

FAR WB_Data *wb_data;

char decimal_point = '.';

IPCData *launcher_ipc;
