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
#ifndef _SEARCH_H
#define _SEARCH_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Data to store search results
typedef struct
{
	char *v_search_found_position;	// Position of match in buffer
	long v_search_found_size;		// Number of bytes searched
	long v_search_found_lines;		// Number of lines searched
	long v_search_last_line_pos;	// Last line position searched
} search_handle;

#ifndef __amigaos3__
	#pragma pack()
#endif

long search_buffer(search_handle *handle,
				   UBYTE *buffer,
				   ULONG buffer_size,
				   UBYTE *match_buf,
				   ULONG match_size,
				   ULONG flags);

#define isonlyword(c) (!c || c == 10 || c == 13 || isspace(c) || ispunct(c))

#endif
