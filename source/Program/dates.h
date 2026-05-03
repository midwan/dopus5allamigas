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

#ifndef _DOPUS_DATE
#define _DOPUS_DATE

// Prototypes
void date_build_string(struct DateStamp *, char *, int);
void date_to_strings(struct DateStamp *, char *, char *, int);
char *date_parse_string(char *, char *, char *, int *);
BOOL date_from_strings(char *, char *, struct DateStamp *);
void date_string(long days, char *string, short format, BOOL paren);
void time_string(long minutes, long tick, char *string);

#endif
