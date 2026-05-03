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

/*
 *
 * dopus5.library header files
 *
 * Support library for Directory Opus version 5
 *
 * (c) Copyright 1998 Jonathan Potter
 *
 */

// Include this file to conveniently include all the standard Opus headers

#ifndef _DOPUSBASE
#define _DOPUSBASE

#include <dopus/args.h>
#include <dopus/bufferedio.h>
#include <dopus/gui.h>
#include <dopus/clipboard.h>
#include <dopus/images.h>
#include <dopus/iff.h>
#include <dopus/misc.h>
#include <dopus/progress.h>
#include <dopus/appicon.h>
#include <dopus/drag.h>
#include <dopus/locale.h>
#include <dopus/lists.h>
#include <dopus/timer.h>
#include <dopus/notify.h>
#include <dopus/ipc.h>
#include <dopus/requesters.h>
#include <dopus/layout.h>
#include <dopus/popup.h>
#include <dopus/diskio.h>
#include <dopus/edithook.h>
#include <dopus/icon.h>
#include <dopus/memory.h>
#include <dopus/dos.h>
#include <dopus/rexx.h>
#include <dopus/hooks.h>

#ifndef DOPUS_PRAG
	#include <pragmas/dopus_pragmas.h>
#endif

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// DOpus Public Semaphore
struct DOpusPublicSemaphore
{
	struct SignalSemaphore ds_Sem;
	APTR ds_MainIPC;
	struct ListLock ds_Modules;
};

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
