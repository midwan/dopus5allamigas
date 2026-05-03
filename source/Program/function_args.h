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

#ifndef _DOPUS_FUNCTION_ARGS
#define _DOPUS_FUNCTION_ARGS

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct _FuncArgs
{
	struct RDArgs *rdargs;		// RDArgs structure
	struct RDArgs *rdargs_res;	// Return from ReadArgs()
	char *arg_string;			// Copy of argument string (with newline)
	LONG *arg_array;			// Argument array pointer
	LONG *arguments;			// Arguments we use
	short count;
	short done_args;
} FuncArgs;

#ifndef __amigaos3__
	#pragma pack()
#endif

FuncArgs *function_parse_args(char *template, char *args);
void function_free_args(FuncArgs *args);
BOOL function_file_args(FunctionHandle *handle, InstructionParsed *instruction, char *args);
void function_file_args_free(FunctionHandle *handle);

#endif
