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

#ifndef _MACROS_H
#define _MACROS_H

/****************************************************************************/
/*
** Some useful macros
*/

#include <SDI/SDI_compiler.h>

#if defined(__amigaos4__)
	#define GETINTERFACE(iface, base) (iface = (APTR)GetInterface((struct Library *)(base), "main", 1L, NULL))
	#define DROPINTERFACE(iface) (DropInterface((struct Interface *)iface), iface = NULL)
#else
	#define GETINTERFACE(iface, base) TRUE
	#define DROPINTERFACE(iface)
#endif

// special flagging macros
#define isFlagSet(v, f) (((v) & (f)) == (f))  // return TRUE if the flag is set
#define hasFlag(v, f) (((v) & (f)) != 0)	  // return TRUE if one of the flags in f is set in v
#define isFlagClear(v, f) (((v) & (f)) == 0)  // return TRUE if flag f is not set in v
#define SET_FLAG(v, f) ((v) |= (f))			  // set the flag f in v
#define CLEAR_FLAG(v, f) ((v) &= ~(f))		  // clear the flag f in v
#define MASK_FLAG(v, f) ((v) &= (f))		  // mask the variable v with flag f bitwise

// transforms a define into a string
#define STR(x) STR2(x)
#define STR2(x) #x

/****************************************************************************/

#endif /* _MACROS_H */
