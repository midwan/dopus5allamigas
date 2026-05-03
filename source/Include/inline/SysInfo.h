#ifndef _INLINE_SYSINFO_H
#define _INLINE_SYSINFO_H

#ifndef CLIB_SYSINFO_PROTOS_H
	#define CLIB_SYSINFO_PROTOS_H
#endif

#ifndef __INLINE_MACROS_H
	#include <inline/macros.h>
#endif

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif
#ifndef LIBRARIES_SYSINFO_H
	#include <libraries/SysInfo.h>
#endif

#ifndef SYSINFO_BASE_NAME
	#define SYSINFO_BASE_NAME SysInfoBase
#endif

#define InitSysInfo() \
	LP0(0x1e, struct SysInfo *, InitSysInfo, , SYSINFO_BASE_NAME)

#define FreeSysInfo(si) \
	LP1NR(0x24, FreeSysInfo, struct SysInfo *, si, a0, , SYSINFO_BASE_NAME)

#define GetLoadAverage(si, la) \
	LP2NR(0x2a, GetLoadAverage, struct SysInfo *, si, a0, struct SI_LoadAverage *, la, a1, , SYSINFO_BASE_NAME)

#define GetPid(si) \
	LP1(0x30, LONG, GetPid, struct SysInfo *, si, a0, , SYSINFO_BASE_NAME)

#define GetPpid(si) \
	LP1(0x36, LONG, GetPpid, struct SysInfo *, si, a0, , SYSINFO_BASE_NAME)

#define GetPgrp(si) \
	LP1(0x3c, LONG, GetPgrp, struct SysInfo *, si, a0, , SYSINFO_BASE_NAME)

#define GetNice(si, which, who) \
	LP3(0x42, LONG, GetNice, struct SysInfo *, si, a0, LONG, which, d0, LONG, who, d1, , SYSINFO_BASE_NAME)

#define SetNice(si, which, who, nice) \
	LP4(0x48, LONG, SetNice, struct SysInfo *, si, a0, LONG, which, d0, LONG, who, d1, LONG, nice, d2, , SYSINFO_BASE_NAME)

#define AddNotify(si, flags, safety_limit) \
	LP3(0x4e, struct SI_Notify *, AddNotify, struct SysInfo *, si, a0, WORD, flags, d0, LONG, safety_limit, d1, , SYSINFO_BASE_NAME)

#define RemoveNotify(si, notify) \
	LP2NR(0x54, RemoveNotify, struct SysInfo *, si, a0, struct SI_Notify *, notify, a1, , SYSINFO_BASE_NAME)

#define GetCpuUsage(si, usage) \
	LP2NR(0x5a, GetCpuUsage, struct SysInfo *, si, a0, struct SI_CpuUsage *, usage, a1, , SYSINFO_BASE_NAME)

#define GetTaskCpuUsage(si, usage, task) \
	LP3(0x60, LONG, GetTaskCpuUsage, struct SysInfo *, si, a0, struct SI_TaskCpuUsage *, usage, a1, struct Task *, task, a2, , SYSINFO_BASE_NAME)

#endif /*  _INLINE_SYSINFO_H  */
