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

#ifndef _addressformat_h
#define _addressformat_h

struct subproc_data;

BOOL get_listformat(struct window_params *wp);
BOOL configure_format(struct display_globals *dg, struct subproc_data *data, IPCMessage *imsg);
void get_formats(struct opusftp_globals *og, struct ftp_environment *env, ListFormat **def_format);
ListFormat *get_opus_format(struct opusftp_globals *og);

#endif
