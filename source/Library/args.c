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

// Allocate and parse arguments
FuncArgs *LIBFUNC L_ParseArgs(REG(a0, char *template), REG(a1, char *args))
{
	short count, len;
	FuncArgs *funcargs;

	// Valid arguments and template?
	if (!template || !args || !*template || !*args)
		return 0;

	// Count the number of arguments in template
	for (len = 0, count = 1; template[len]; len++)
	{
		// Add one for every comma
		if (template[len] == ',')
			++count;
	}

	// Get length of argument string
	len = strlen(args) + 2;

	// Allocate FuncArgs
	if (!(funcargs = AllocVec(sizeof(FuncArgs) + sizeof(LONG *) * count * 2 + len, MEMF_CLEAR)) ||
		!(funcargs->FA_RDArgs = AllocDosObject(DOS_RDARGS, 0)))
	{
		L_DisposeArgs(funcargs);
		return 0;
	}

	// Initialise pointers
	funcargs->FA_ArgArray = (LONG *)(funcargs + 1);
	funcargs->FA_Arguments = funcargs->FA_ArgArray + count;
	funcargs->FA_ArgString = (char *)(funcargs->FA_Arguments + count);

	// Copy arg string and add a newline
	strcpy(funcargs->FA_ArgString, args);
	strcat(funcargs->FA_ArgString, "\n");

	// Initialise RDArgs
	funcargs->FA_RDArgs->RDA_Source.CS_Buffer = funcargs->FA_ArgString;
	funcargs->FA_RDArgs->RDA_Source.CS_Length = strlen(funcargs->FA_ArgString);

	// Call RDArgs
	if (!(ReadArgs(template, funcargs->FA_ArgArray, funcargs->FA_RDArgs)))
	{
		L_DisposeArgs(funcargs);
		return 0;
	}

	// Store argument count
	funcargs->FA_Count = count;

	// Copy argument pointers
	for (len = 0; len < count; len++)
		funcargs->FA_Arguments[len] = funcargs->FA_ArgArray[len];

	return funcargs;
}

// Free FuncArgs
void LIBFUNC L_DisposeArgs(REG(a0, FuncArgs *args))
{
	if (args)
	{
		short arg;

		// Custom arguments?
		if (args->FA_Arguments)
		{
			// Free any custom arguments
			for (arg = 0; arg < args->FA_Count; arg++)
			{
				// Got an argument that's different?
				if (args->FA_Arguments[arg] && args->FA_Arguments[arg] != args->FA_ArgArray[arg])
				{
					// Free it
					FreeVec((APTR)args->FA_Arguments[arg]);
				}
			}
		}

		// Free result of ReadArgs() call
		FreeArgs(args->FA_RDArgs);

		// Free RDArgs structure
		FreeDosObject(DOS_RDARGS, args->FA_RDArgs);

		// Free FuncArgs
		FreeVec(args);
	}
}
