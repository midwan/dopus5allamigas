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

#define MAX_BUFFER 512

#ifndef __amigaos3__
	#pragma pack(2)
#endif

struct eliza_data
{
	char input_string_buffer[MAX_BUFFER + 4];
	char *input_string;
	char last_input_string[MAX_BUFFER];

	int *first_response;
	int *current_response;
	int *last_response;

	int n1, /* Number of elements in keywords (minus 1) */
		n2, /* Number of elements in wordin and wordout (minus 1) */
		key;

	int keyword_size, wordin_size, wordout_size;
};

#ifndef __amigaos3__
	#pragma pack()
#endif

short eliza_line(struct eliza_data *data, char *line);
BOOL init_eliza(struct eliza_data *);
void free_eliza(struct eliza_data *);
void print_string(char *);
