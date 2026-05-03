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

#ifndef _DOPUS_REQUESTERS
#define _DOPUS_REQUESTERS

#define CHECKERROR_ABORT 3
#define CHECKERROR_SKIP 2
#define CHECKERROR_TRY_AGAIN 1

#define EXISTREPLACE_LEAVE 0
#define EXISTREPLACE_REPLACE 1
#define EXISTREPLACE_ALL 2
#define EXISTREPLACE_ABORT 2
#define EXISTREPLACE_AGAIN 2
#define EXISTREPLACE_ABORT2 3

#ifndef __amigaos3__
	#pragma pack(2)
#endif

struct super_request_data
{
	struct DOpusSimpleRequest request;
	char *gad, *gadgets[11];
	int return_values[10];
	short ret_val, gad_count;
};

#ifndef __amigaos3__
	#pragma pack()
#endif

// Prototypes
#define super_request_args(parent, message, ...)                       \
	({                                                                 \
		IPTR __args[] = {__VA_ARGS__};                                 \
		(short)super_request(parent, message, NULL, (ULONG *)&__args); \
	})
short super_request(APTR, char *, IPCData *, ULONG *);

#define error_request(window, flags, ...)                        \
	({                                                           \
		IPTR __text[] = {__VA_ARGS__};                           \
		(short)error_request_a(window, flags, (char **)&__text); \
	})
short error_request_a(struct Window *window, short, char **text);

#define ERF_ASYNC (1 << 0)
#define ERF_MOUSE (1 << 3)

APTR alloc_filereq(void);

void global_requester(char *text);
short request_font(struct Window *, char *, char *, short *, ULONG, unsigned short, unsigned char *, short *, short *);

#endif
