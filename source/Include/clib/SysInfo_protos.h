#ifndef CLIB_SYSINFO_PROTOS_H
#define CLIB_SYSINFO_PROTOS_H
/*
**      $VER: SysInfo_protos.h 2.00 (20.10.96)
**      SysInfo Release 2.00
**
**      C prototypes.
**
**      This file is public domain.
**
**      Author: Petri Nordlund <petrin@megabaud.fi>
**
**      $Id: SysInfo_protos.h 1.5 1996/10/01 22:57:22 petrin Exp petrin $
**
*/

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

#ifndef LIBRARIES_SYSINFO_H
	#include <libraries/SysInfo.h>
#endif

/* init */
struct SysInfo *InitSysInfo(void);
void FreeSysInfo(struct SysInfo *);

/* load average */
void GetLoadAverage(struct SysInfo *, struct SI_LoadAverage *);

/* id */
LONG GetPid(struct SysInfo *);
LONG GetPpid(struct SysInfo *);
LONG GetPgrp(struct SysInfo *);

/* nice */
LONG GetNice(struct SysInfo *, LONG, LONG);
LONG SetNice(struct SysInfo *, LONG, LONG, LONG);

/* notify */
struct SI_Notify *AddNotify(struct SysInfo *, WORD, LONG);
void RemoveNotify(struct SysInfo *, struct SI_Notify *);

/* cpu usage */
void GetCpuUsage(struct SysInfo *, struct SI_CpuUsage *);
LONG GetTaskCpuUsage(struct SysInfo *, struct SI_TaskCpuUsage *, struct Task *);

#endif /* CLIB_SYSINFO_PROTOS_H */
