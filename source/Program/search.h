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

#ifndef _DOPUS_SEARCH
#define _DOPUS_SEARCH

enum {
	GAD_SEARCH_LAYOUT,
	GAD_SEARCH_CANCEL,
	GAD_SEARCH_OKAY,
	GAD_SEARCH_TEXT,
	GAD_SEARCH_CASE,
	GAD_SEARCH_WILD,
	GAD_SEARCH_ONLYWORD,
	GAD_SEARCH_RESULT
};

enum { RESULT_PROMPT, RESULT_LEAVE, RESULT_OUTPUT };

extern ConfigWindow search_window;
extern ObjectDef search_objects[];

// Data to store search results
typedef struct
{
	char *v_search_found_position;	// Position of match in buffer
	long v_search_found_size;		// Number of bytes searched
	long v_search_found_lines;		// Number of lines searched
	long v_search_last_line_pos;	// Last line position searched
} search_handle;

// Prototypes
int search_file(APTR, UBYTE *, ULONG, UBYTE *, ULONG);
int search_buffer(search_handle *, UBYTE *, ULONG, UBYTE *, ULONG, ULONG);

#endif
