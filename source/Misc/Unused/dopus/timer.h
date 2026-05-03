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

#ifndef _DOPUS_TIMER
#define _DOPUS_TIMER

/*****************************************************************************

 Timer

 *****************************************************************************/
#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct TimerHandle
{
	struct MsgPort *port;	  // Port to wait on
	struct timerequest req;	  // Timer request
	struct MsgPort *my_port;  // Supplied port
	short active;			  // Indicates active request
} TimerHandle;

#ifndef __amigaos3__
	#pragma pack()
#endif

TimerHandle *AllocTimer(ULONG, struct MsgPort *);
BOOL CheckTimer(TimerHandle *);
void FreeTimer(TimerHandle *);
struct Library *GetTimerBase(void);
void StartTimer(TimerHandle *, ULONG, ULONG);
void StopTimer(TimerHandle *);
BOOL TimerActive(TimerHandle *);

#endif
