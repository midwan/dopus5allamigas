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

#include "dopus.h"

#define ARGBUFFER 514

// Allocate and parse arguments
FuncArgs *function_parse_args(char *template, char *args)
{
	short count, len;
	FuncArgs *funcargs;

	// Valid arguments and template?
	if (!template || !args || !template[0] || !args[0])
		return 0;

	// Count the number of arguments in template
	for (len = 0, count = 1; template[len]; len++)
	{
		// Add one for every comma
		if (template[len] == ',')
			++count;
	}

	// Allocate FuncArgs
	if (!(funcargs = AllocVec(sizeof(FuncArgs) + ARGBUFFER + sizeof(LONG *) * count * 2, MEMF_CLEAR)) ||
		!(funcargs->rdargs = AllocDosObject(DOS_RDARGS, 0)))
	{
		function_free_args(funcargs);
		return 0;
	}

	// Initialise pointers
	funcargs->arg_string = ((char *)funcargs) + sizeof(FuncArgs);
	funcargs->arg_array = (LONG *)(funcargs->arg_string + ARGBUFFER);
	funcargs->arguments = funcargs->arg_array + count;

	// Initialise RDArgs
	funcargs->rdargs->RDA_Source.CS_Buffer = funcargs->arg_string;
	funcargs->rdargs->RDA_Source.CS_Length = strlen(args) + 1;

	// Copy arg string and add a newline
	strcpy(funcargs->arg_string, args);
	strcat(funcargs->arg_string, "\n");

	// Call RDArgs
	if (!(funcargs->rdargs_res = ReadArgs(template, funcargs->arg_array, funcargs->rdargs)))
	{
		function_free_args(funcargs);
		return 0;
	}

	// Store argument count
	funcargs->count = count;

	// Copy argument pointers
	for (len = 0; len < count; len++)
		funcargs->arguments[len] = funcargs->arg_array[len];

	return funcargs;
}

// Free FuncArgs
void function_free_args(FuncArgs *args)
{
	if (args)
	{
		short arg;

		// Custom arguments?
		if (args->arguments)
		{
			// Free any custom arguments
			for (arg = 0; arg < args->count; arg++)
			{
				// Got an argument that's different?
				if (args->arguments[arg] && args->arguments[arg] != args->arg_array[arg])
				{
					// Free it
					FreeVec((APTR)args->arguments[arg]);
				}
			}
		}

		// Free result of ReadArgs() call
		FreeArgs(args->rdargs_res);

		// Free RDArgs structure
		FreeDosObject(DOS_RDARGS, args->rdargs);

		// Free FuncArgs
		FreeVec(args);
	}
}
