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
#ifndef _POOLS_H
#define _POOLS_H

#ifdef __amigaos3__
void *ASM AsmCreatePool(REG(d0, ULONG), REG(d1, ULONG), REG(d2, ULONG), REG(a6, struct Library *));
void ASM AsmDeletePool(REG(a0, void *), REG(a6, struct Library *));
void *ASM AsmAllocPooled(REG(a0, void *), REG(d0, ULONG), REG(a6, struct Library *));
void ASM AsmFreePooled(REG(a0, void *), REG(a1, void *), REG(d0, ULONG), REG(a6, struct Library *));
#else
	#define AsmCreatePool(a, b, c, d) CreatePool(a, b, c)
	#define AsmDeletePool(a, b) DeletePool(a)
	#define AsmAllocPooled(a, b, c) AllocPooled(a, b)
	#define AsmFreePooled(a, b, c, d) FreePooled(a, b, c)
#endif

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	APTR pool_header;			  // Header for pooling
	struct MinList memory_list;	  // List if not pooling
	struct SignalSemaphore lock;  // Lock for task protection
	ULONG puddle_size;			  // Size of puddles for pooling
	ULONG thresh_size;			  // Threshold for pooling
	ULONG type;					  // Type of memory
	ULONG flags;				  // Flags
	ULONG total;				  // Total memory allocated under this handle
	IPTR check_val;			  // Header check value
} MemHandle;

#ifndef __amigaos3__
	#pragma pack()
#endif

#define MEMHF_CLEAR (1 << 0)  // Clear memory
#define MEMHF_LOCK (1 << 1)	  // Want locking

#endif
