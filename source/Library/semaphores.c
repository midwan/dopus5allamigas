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

#undef TRAP_SEMAPHORE

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct SemaphoreOwner
{
	struct MinNode node;
	struct Task *task;
	long exclusive;
	char *data;
} SemaphoreOwner;

typedef struct SemaphoreNode
{
	struct MinNode node;
	struct SignalSemaphore *sem;
	struct MinList owners;
	struct MinList waiters;
} SemaphoreNode;

#ifndef __amigaos3__
	#pragma pack()
#endif

long LIBFUNC L_GetSemaphore(REG(a0, struct SignalSemaphore *sem), REG(d0, long exclusive), REG(a1, char *name))
{
#ifdef TRAP_SEMAPHORE
	SemaphoreNode *node;
	SemaphoreOwner *owner = 0;
	struct MyLibrary *libbase;
	struct LibData *data;
#endif

	// Attempt only?
	if (exclusive & SEMF_ATTEMPT)
	{
		long res;

		// Want exclusive lock?
		if (exclusive & SEMF_EXCLUSIVE)
			res = AttemptSemaphore(sem);

		// Shared lock
		else
		{
			// Try for shared semaphore
			if (!(res = AttemptSemaphoreShared(sem)))
			{
				// Fix <v39 bug
				if (((struct Library *)SysBase)->lib_Version < 39)

				{
					// Try for exclusive instead
					res = AttemptSemaphore(sem);
				}
			}
		}

		return res;
	}

#ifdef TRAP_SEMAPHORE
	// Get our library data
	libbase = GET_DOPUSLIB;
	data = (struct LibData *)libbase->ml_UserData;

	// Lock semaphore list
	ObtainSemaphore(&data->semaphores.lock);

	// Look for semaphore in list
	for (node = (SemaphoreNode *)data->semaphores.list.lh_Head; node->node.mln_Succ;
		 node = (SemaphoreNode *)node->node.mln_Succ)
	{
		// Try to match semaphore
		if (node->sem == sem)
			break;
	}

	// Not matched?
	if (!node->node.mln_Succ)
	{
		// Allocate node
		if (node = AllocVec(sizeof(SemaphoreNode), MEMF_CLEAR))
		{
			// Add to semaphore list
			AddTail(&data->semaphores.list, (struct Node *)node);

			// Set semaphore pointer
			node->sem = sem;

			// Initialise lists
			NewList((struct List *)&node->owners);
			NewList((struct List *)&node->waiters);
		}
	}

	// Got a valid node?
	if (node)
	{
		// Allocate owner node
		if (owner = AllocVec(sizeof(SemaphoreOwner), MEMF_CLEAR))
		{
			// Fill out owner
			owner->task = FindTask(0);
			owner->exclusive = exclusive;
			owner->data = name;

			// Add to waiter list
			AddTail((struct List *)&node->waiters, (struct Node *)owner);
		}
	}

	// Unlock semaphore list
	ReleaseSemaphore(&data->semaphores.lock);

#endif

	// Try to get the actual semaphore
	if (exclusive & SEMF_EXCLUSIVE)
		ObtainSemaphore(sem);
	else
	{
		// Fix <v39 bug
		if (((struct Library *)SysBase)->lib_Version < 39)

		{
			// Try to get shared semaphore
			if (!AttemptSemaphoreShared(sem))
			{
				// Try for exclusive semaphore
				if (!AttemptSemaphore(sem))
				{
					// Wait for shared lock
					ObtainSemaphoreShared(sem);
				}
			}
		}

		// Under 39, just ask for shared lock
		else
			ObtainSemaphoreShared(sem);
	}

#ifdef TRAP_SEMAPHORE

	// Valid owner?
	if (owner)
	{
		// Lock semaphore list
		ObtainSemaphore(&data->semaphores.lock);

		// Remove us from the waiter queue
		Remove((struct Node *)owner);

		// Add us to the owner queue
		AddTail((struct List *)&node->owners, (struct Node *)owner);

		// Unlock semaphore list
		ReleaseSemaphore(&data->semaphores.lock);
	}

#endif

	return TRUE;
}

void LIBFUNC L_FreeSemaphore(REG(a0, struct SignalSemaphore *sem))
{
#ifdef TRAP_SEMAPHORE
	SemaphoreNode *node;
	SemaphoreOwner *owner;
	struct Task *task;
	struct MyLibrary *libbase;
	struct LibData *data;

	// Get our library data
	libbase = GET_DOPUSLIB;
	data = (struct LibData *)libbase->ml_UserData;

	// Find this task
	task = FindTask(0);

	// Lock semaphore list
	ObtainSemaphore(&data->semaphores.lock);

	// Go through Semaphore list
	for (node = (SemaphoreNode *)data->semaphores.list.lh_Head; node->node.mln_Succ;
		 node = (SemaphoreNode *)node->node.mln_Succ)
	{
		// Try to match semaphore
		if (node->sem == sem)
			break;
	}

	// Found semaphore?
	if (node->node.mln_Succ)
	{
		// Go through semaphore owners, backwards
		for (owner = (SemaphoreOwner *)node->owners.mlh_TailPred; owner->node.mln_Pred;
			 owner = (SemaphoreOwner *)owner->node.mln_Pred)
		{
			// Match task
			if (owner->task == task)
			{
				// Remove from owner list and free entry
				Remove((struct Node *)owner);
				FreeVec(owner);
				break;
			}
		}

		// If semaphore is now unused, remove from list
		if (IsListEmpty((struct List *)&node->owners) && IsListEmpty((struct List *)&node->waiters))
		{
			// Remove and free
			Remove((struct Node *)node);
			FreeVec(node);
		}
	}

	// Unlock semaphore list
	ReleaseSemaphore(&data->semaphores.lock);

#endif

	// Unlock actual semaphore
	ReleaseSemaphore(sem);
}

void LIBFUNC L_ShowSemaphore(REG(a0, struct SignalSemaphore *sem))
{
#ifdef TRAP_SEMAPHORE
	SemaphoreNode *node;
	SemaphoreOwner *owner;
	struct MyLibrary *libbase;
	struct LibData *data;

	// Get our library data
	libbase = GET_DOPUSLIB;
	data = (struct LibData *)libbase->ml_UserData;

	// Lock semaphore list
	ObtainSemaphore(&data->semaphores.lock);

	// Go through Semaphore list
	for (node = (SemaphoreNode *)data->semaphores.list.lh_Head; node->node.mln_Succ;
		 node = (SemaphoreNode *)node->node.mln_Succ)
	{
		// Match semaphore
		if (node->sem == sem)
			break;
	}

	// Matched?
	if (node->node.mln_Succ)
	{
		D(bug("Semaphore : %s\n", sem->ss_Link.ln_Name));
		D(bug("Owners:\n"));
		for (owner = (SemaphoreOwner *)node->owners.mlh_Head; owner->node.mln_Succ;
			 owner = (SemaphoreOwner *)owner->node.mln_Succ)
		{
			D(bug("%lc   %s (%lx) %s\n",
				  (owner->exclusive) ? '*' : ' ',
				  owner->task->tc_Node.ln_Name,
				  owner->task,
				  owner->data));
		}
		D(bug("Waiters:\n"));
		for (owner = (SemaphoreOwner *)node->waiters.mlh_Head; owner->node.mln_Succ;
			 owner = (SemaphoreOwner *)owner->node.mln_Succ)
		{
			D(bug("%lc   %s (%lx) %s\n",
				  (owner->exclusive) ? '*' : ' ',
				  owner->task->tc_Node.ln_Name,
				  owner->task,
				  owner->data));
		}
	}

	// Unlock semaphore list
	ReleaseSemaphore(&data->semaphores.lock);
#endif
}

// Initialise a ListLock
void LIBFUNC L_InitListLock(REG(a0, struct ListLock *ll), REG(a1, char *name))
{
	NewList(&ll->list);
	InitSemaphore(&ll->lock);
	ll->lock.ss_Link.ln_Name = name;
}
