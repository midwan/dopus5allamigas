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

/*
	Create a memory handle

	puddle_size = size of puddles to use for pooling (0 for no pools)
	thresh_size = threshold of puddle allocations
	type        = type of memory

				  MEMF_CLEAR  = clear memory
				  MEMF_PUBLIC = use semaphore locking when accessing pool

				  Other MEMF flags as normal
*/

void *LIBFUNC L_NewMemHandle(REG(d0, ULONG puddle_size), REG(d1, ULONG thresh_size), REG(d2, ULONG type))
{
	MemHandle *handle;

	// Allocate handle
	if (!(handle = AllocVec(sizeof(MemHandle), MEMF_CLEAR)))
		return 0;

	// Initialise check value
	handle->check_val = (IPTR)handle;

	// Want pooling?
	if (puddle_size > 0)
	{
		long thresh_count;

		// Calculate number of "thresholds" in the puddle size
		thresh_count = UDivMod32(puddle_size, thresh_size);

		// Increment thresh size by size of allocation headers
		thresh_size += 3 * sizeof(IPTR);

		// Increment puddle size by size of allocation headers
		puddle_size += 3 * sizeof(IPTR) * thresh_count;
	}

	// Initialise handle
	NewList((struct List *)&handle->memory_list);
	InitSemaphore(&handle->lock);
	handle->puddle_size = puddle_size;
	handle->thresh_size = thresh_size;

	// Want memory clearing?
	if (type & MEMF_CLEAR)
		handle->flags = MEMHF_CLEAR;

	// Want locking?
	if (type & MEMF_PUBLIC)
	{
		handle->flags |= MEMHF_LOCK;
		type &= ~MEMF_PUBLIC;
	}

	// Save type of memory
	handle->type = type;

	// If puddle size is >0, try to create a pool
	if (puddle_size > 0)
	{
		handle->pool_header = CreatePool(type, puddle_size, thresh_size);
	}

	// Return handle
	return handle;
}

// Free a memory handle and all associated allocations
void LIBFUNC L_FreeMemHandle(REG(a0, MemHandle *handle))
{
	// Valid handle?
	if (handle)
	{
		// Did the handle have a memory pool?
		if (handle->pool_header)
		{
			DeletePool(handle->pool_header);
		}

		// Otherwise, free all memory allocations
		else
			L_ClearMemHandle(handle);

		// Free the handle
		FreeVec(handle);
	}
}

// Clear all memory allocated with a handle (leave handle intact)
void LIBFUNC L_ClearMemHandle(REG(a0, MemHandle *handle))
{
	BOOL realloc = 1;

	// Valid handle?
	if (handle)
	{
		// Want locking?
		if (handle->flags & MEMHF_LOCK)
			ObtainSemaphore(&handle->lock);

		// Did the handle have a memory pool?
		if (handle->pool_header)
		{
			// Is there any memory allocated?
			if (handle->total > 0)
			{
				DeletePool(handle->pool_header);
				handle->pool_header = 0;
			}

			// Otherwise, don't need to clear
			else
				realloc = 0;
		}

		// Otherwise, we have a list of allocations
		else
		{
			struct MinNode *node, *next;

			// Go through list, free all allocations
			for (node = handle->memory_list.mlh_Head; node->mln_Succ; node = next)
			{
				IPTR *ptr;

				// Get next pointer
				next = node->mln_Succ;

				// Free this entry
				ptr = (IPTR *)node;
				FreeMem(node, (ULONG)ptr[3]);
			}
		}

		// Clear allocation total
		handle->total = 0;

		// Did we need to clear it?
		if (realloc)
		{
			// Initialise memory list
			NewList((struct List *)&handle->memory_list);

			// Did we have pooling before?
			if (handle->puddle_size > 0)
			{
				// Try to create a new pool. If this fails we'll be using allocation lists from now on
				handle->pool_header = CreatePool(handle->type, handle->puddle_size, handle->thresh_size);
			}
		}

		// Unlock if necessary
		if (handle->flags & MEMHF_LOCK)
			ReleaseSemaphore(&handle->lock);
	}
}

/*
	Allocate some memory, with size tracking

	Allocated as :    [<mln_Succ>]		-16
					  [<mln_Pred>]		-12
					  <handle>			-2 * sizeof(IPTR)
					  <size>			-1 * sizeof(IPTR)
					  <memory pointer>	0

	handle = handle to use for allocations
			 can be 0 in which case size will be tracked but memory must be
			 freed manually (ie can't FreeMemHandle())
	size   = size of allocation
*/

void *LIBFUNC L_AllocMemH(REG(a0, MemHandle *handle), REG(d0, ULONG size))
{
	register IPTR *mem;

	// Get size to allocate (size rounded up plus 2 longwords)
	size = (size + sizeof(IPTR) - 1) & ~(sizeof(IPTR) - 1);
	size += 2 * sizeof(IPTR);

	// If no handle supplied, use AllocMem()
	if (!handle)
		mem = AllocMem(size, MEMF_CLEAR);

	// Allocate memory through handle
	else
	{
		// Lock if necessary
		if (handle->flags & MEMHF_LOCK)
			ObtainSemaphore(&handle->lock);

		// If there's a memory pool, use that
		if (handle->pool_header)
		{
			mem = AllocPooled(handle->pool_header, size);

			// If successful
			if (mem)
			{
				// Increment allocation total
				handle->total += size;

				// Need to clear?
				if (handle->flags & MEMHF_CLEAR)
				{
					register UBYTE *clr;
					register ULONG clrsize;

					// Clear memory
					clr = (UBYTE *)mem;
					for (clrsize = 0; clrsize < size; clrsize++)
						clr[clrsize] = 0;
				}
			}
		}

		// No memory pool
		else
		{
			// Need extra storage for a MinNode
			size += sizeof(struct MinNode);

			// Allocate extra, for a node
			if ((mem = AllocMem(size, handle->type)))
			{
				// Add to memory list
				AddTail((struct List *)&handle->memory_list, (struct Node *)mem);

				// Bump memory pointer past node
				mem += 2;

				// Add to allocation total
				handle->total += size;
			}
		}

		// Unlock if necessary
		if (handle->flags & MEMHF_LOCK)
			ReleaseSemaphore(&handle->lock);
	}

	// Did we succeed?
	if (mem)
	{
		// Store header pointer and size
		mem[0] = (IPTR)handle;
		mem[1] = size;

		// Return bumped pointer
		return (void *)(mem + 2);
	}

	return 0;
}

// Free some memory allocated with AllocMemH()
void LIBFUNC L_FreeMemH(REG(a0, void *memory))
{
	// Check valid pointer
	if (memory)
	{
		register IPTR *mem;
		register MemHandle *handle;

		// Decrement pointer
		mem = ((IPTR *)memory) - 2;

		// Do we have a memory handle?
		if ((handle = (MemHandle *)mem[0]))
		{
			// See if handle is valid
			if (handle->check_val != (IPTR)handle)
			{
				/*
								D(bug("** Invalid MemHandle : %lx %lx %ld\n",handle,memory,mem[1]));
				*/
			}

			// Assume it is
			else
			{
				// Lock if necessary
				if (handle->flags & MEMHF_LOCK)
					ObtainSemaphore(&handle->lock);

				// Decrement from allocation total
				handle->total -= (ULONG)mem[1];

				// Is there a memory pool?
				if (handle->pool_header)
				{
					FreePooled(handle->pool_header, mem, (ULONG)mem[1]);
				}

				// Otherwise
				else
				{
					struct MinNode *node;

					// Get node pointer
					node = (struct MinNode *)(mem - 2);

					// Remove from memory list
					Remove((struct Node *)node);

					// Free allocation
					FreeMem(node, (ULONG)mem[1]);
				}

				// Unlock if necessary
				if (handle->flags & MEMHF_LOCK)
					ReleaseSemaphore(&handle->lock);
			}
		}

		// Free with FreeMem()
		else
			FreeMem(mem, (ULONG)mem[1]);
	}
}
