/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_SYSINFO_H
#define _PPCINLINE_SYSINFO_H

#ifndef __PPCINLINE_MACROS_H
	#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef SYSINFO_BASE_NAME
	#define SYSINFO_BASE_NAME SysInfoBase
#endif /* !SYSINFO_BASE_NAME */

#define FreeSysInfo(__p0) LP1NR(36, FreeSysInfo, struct SysInfo *, __p0, a0, , SYSINFO_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddNotify(__p0, __p1, __p2) \
	LP3(78,                         \
		struct SI_Notify *,         \
		AddNotify,                  \
		struct SysInfo *,           \
		__p0,                       \
		a0,                         \
		WORD,                       \
		__p1,                       \
		d0,                         \
		LONG,                       \
		__p2,                       \
		d1,                         \
		,                           \
		SYSINFO_BASE_NAME,          \
		0,                          \
		0,                          \
		0,                          \
		0,                          \
		0,                          \
		0)

#define GetPgrp(__p0) LP1(60, LONG, GetPgrp, struct SysInfo *, __p0, a0, , SYSINFO_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RemoveNotify(__p0, __p1) \
	LP2NR(84,                    \
		  RemoveNotify,          \
		  struct SysInfo *,      \
		  __p0,                  \
		  a0,                    \
		  struct SI_Notify *,    \
		  __p1,                  \
		  a1,                    \
		  ,                      \
		  SYSINFO_BASE_NAME,     \
		  0,                     \
		  0,                     \
		  0,                     \
		  0,                     \
		  0,                     \
		  0)

#define GetLoadAverage(__p0, __p1) \
	LP2NR(42,                      \
		  GetLoadAverage,          \
		  struct SysInfo *,        \
		  __p0,                    \
		  a0,                      \
		  struct SI_LoadAverage *, \
		  __p1,                    \
		  a1,                      \
		  ,                        \
		  SYSINFO_BASE_NAME,       \
		  0,                       \
		  0,                       \
		  0,                       \
		  0,                       \
		  0,                       \
		  0)

#define InitSysInfo() LP0(30, struct SysInfo *, InitSysInfo, , SYSINFO_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetNice(__p0, __p1, __p2) \
	LP3(66,                       \
		LONG,                     \
		GetNice,                  \
		struct SysInfo *,         \
		__p0,                     \
		a0,                       \
		LONG,                     \
		__p1,                     \
		d0,                       \
		LONG,                     \
		__p2,                     \
		d1,                       \
		,                         \
		SYSINFO_BASE_NAME,        \
		0,                        \
		0,                        \
		0,                        \
		0,                        \
		0,                        \
		0)

#define GetCpuUsage(__p0, __p1) \
	LP2NR(90,                   \
		  GetCpuUsage,          \
		  struct SysInfo *,     \
		  __p0,                 \
		  a0,                   \
		  struct SI_CpuUsage *, \
		  __p1,                 \
		  a1,                   \
		  ,                     \
		  SYSINFO_BASE_NAME,    \
		  0,                    \
		  0,                    \
		  0,                    \
		  0,                    \
		  0,                    \
		  0)

#define SetNice(__p0, __p1, __p2, __p3) \
	LP4(72,                             \
		LONG,                           \
		SetNice,                        \
		struct SysInfo *,               \
		__p0,                           \
		a0,                             \
		LONG,                           \
		__p1,                           \
		d0,                             \
		LONG,                           \
		__p2,                           \
		d1,                             \
		LONG,                           \
		__p3,                           \
		d2,                             \
		,                               \
		SYSINFO_BASE_NAME,              \
		0,                              \
		0,                              \
		0,                              \
		0,                              \
		0,                              \
		0)

#define GetPid(__p0) LP1(48, LONG, GetPid, struct SysInfo *, __p0, a0, , SYSINFO_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetPpid(__p0) LP1(54, LONG, GetPpid, struct SysInfo *, __p0, a0, , SYSINFO_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetTaskCpuUsage(__p0, __p1, __p2) \
	LP3(96,                               \
		LONG,                             \
		GetTaskCpuUsage,                  \
		struct SysInfo *,                 \
		__p0,                             \
		a0,                               \
		struct SI_TaskCpuUsage *,         \
		__p1,                             \
		a1,                               \
		struct Task *,                    \
		__p2,                             \
		a2,                               \
		,                                 \
		SYSINFO_BASE_NAME,                \
		0,                                \
		0,                                \
		0,                                \
		0,                                \
		0,                                \
		0)

#endif /* !_PPCINLINE_SYSINFO_H */
