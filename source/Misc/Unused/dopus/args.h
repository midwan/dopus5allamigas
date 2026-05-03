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

#ifndef _DOPUS_ARGS
#define _DOPUS_ARGS

/*****************************************************************************

 ReadArgs support

 *****************************************************************************/

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	struct RDArgs *FA_RDArgs;	  // RDArgs structure
	struct RDArgs *FA_RDArgsRes;  // Return from ReadArgs()
	char *FA_ArgString;			  // Copy of argument string (with newline)
	long *FA_ArgArray;			  // Argument array pointer
	long *FA_Arguments;			  // Argument array you should use
	short FA_Count;				  // Number of arguments
	short FA_DoneArgs;			  // DOpus uses this flag for its own purposes
} FuncArgs;

#ifndef __amigaos3__
	#pragma pack()
#endif

FuncArgs *ParseArgs(char *, char *);
void DisposeArgs(FuncArgs *);

#endif
