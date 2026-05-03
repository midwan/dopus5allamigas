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

	getusage.c - Conversion of Assem functions to C functions.
*/

#include "dopuslib.h"

static ULONG oldIdle, oldDispatch;

long GetCPUUsage(void)	// Doesn't work on OS4
{
	ULONG newIdle = ((struct ExecBase *)SysBase)->IdleCount;
	ULONG newDispatch = ((struct ExecBase *)SysBase)->DispCount;
	ULONG temp = 0;
	ULONG idleDifference = 0;
	ULONG dispatchDifference = 0;

	temp = oldIdle;
	oldIdle = newIdle;
	idleDifference = newIdle - temp;

	temp = oldDispatch;
	oldDispatch = newDispatch;
	dispatchDifference = newDispatch - temp;

	if (idleDifference <= 0)
		return 100;
	if (dispatchDifference <= 0)
		return 0;

	temp = idleDifference + dispatchDifference;
	return ((dispatchDifference * 100) / temp);
}
