#ifndef LIBRARIES_SYSINFO_H
#define LIBRARIES_SYSINFO_H

/*
**      $VER: SysInfo.h 2.00 (20.10.96)
**      SysInfo Release 2.00
**
**      SysInfo.library definitions
**
**      This file is public domain.
**
**      Author: Petri Nordlund <petrin@megabaud.fi>
**
**      $Id: SysInfo.h 1.5 1996/10/01 23:03:07 petrin Exp petrin $
**
*/

#ifndef EXEC_TYPES_H
	#include "exec/types.h"
#endif /* EXEC_TYPES_H */

#ifndef EXEC_PORTS_H
	#include "exec/ports.h"
#endif /* EXEC_PORTS_H */

#define SYSINFONAME "SysInfo.library"
#define SYSINFOVERSION 2L

#ifndef __amigaos3__
	#pragma pack(2)
#endif

/*
 * This structure is returned by InitSysInfo() and it's READ-ONLY.
 *
 * NOTE!! This structure will grow in the future, so don't make any
 * assumptions about it's length.
 *
 */

struct SysInfo
{
	/* general */
	LONG errno; /* Used to hold error values 		*/
	/* from some functions			*/
	/* load average */
	UWORD loadavg_type;	 /* load average type, see below		*/
	UWORD loadavg_time1; /* Usually 1, 5 and 15 minutes.		*/
	UWORD loadavg_time2; /* These times are in seconds.		*/
	UWORD loadavg_time3; /* 0 = time not implemented		*/
	UWORD fscale;		 /* scale value, if lavgtype = FIXED	*/

	/* id */
	BOOL GetPpid_implemented; /* TRUE if GetPpid is implemented	*/
	BOOL GetPgrp_implemented; /* TRUE if GetPgrp is implemented	*/

	/* get/setnice */
	UWORD which_implemented; /* Search methods for Get/SetNice	*/
	WORD nicemin;			 /* Nice-value giving most cpu time	*/
	WORD nicemax;			 /* Nice-value giving least cpu time	*/

	/* notify */
	BOOL notify_sig_implemented; /* Notify by signal implemented		*/
	BOOL notify_msg_implemented; /* Notify by message implemented	*/

	/* cpu usage */
	UWORD cpu_usage_implemented; /* What cpu usage values are implemented */

	/* task cpu usage */
	UWORD task_cpu_usage_implemented; /* What cpu usage values are implemented */

	LONG reserved[8]; /* Reserved for future use		*/
};

/*
 * general
 *
 */

/* errno values */
#define WHICH_EPERM 1	/* Operation not permitted	*/
#define WHICH_ESRCH 3	/* No such process		*/
#define WHICH_EACCES 13 /* Permission denied		*/
#define WHICH_EINVAL 22 /* Invalid argument		*/

/*
 * load average
 *
 */

/* LoadAverage type */
#define LOADAVG_NONE 0	   /* Load averages not implemented	*/
#define LOADAVG_FIXEDPNT 1 /* Load * SysInfo->fscale		*/

/* GetLoadAverage */
struct SI_LoadAverageFixed
{
	ULONG load1;
	ULONG load2;
	ULONG load3;
};

/* This is needed when calling GetLoadAverage() */
struct SI_LoadAverage
{
	struct SI_LoadAverageFixed lavg_fixed;
	LONG reserved[3]; /* Reserved for future use */
};

/*
 * get/setnice
 *
 */

/* Possible search methods for Get/SetNice */
#define WHICH_PRIO_PROCESS 0
#define WHICH_PRIO_PGRP 1
#define WHICH_PRIO_USER 2
#define WHICH_PRIO_TASK 3

/* These bits are used in which_implemented-field */
#define WHICHB_PRIO_PROCESS 0
#define WHICHB_PRIO_PGRP 1
#define WHICHB_PRIO_USER 2
#define WHICHB_PRIO_TASK 3

#define WHICHF_PRIO_PROCESS (1L << 0)
#define WHICHF_PRIO_PGRP (1L << 1)
#define WHICHF_PRIO_USER (1L << 2)
#define WHICHF_PRIO_TASK (1L << 3)

/*
 * notify
 *
 */

/* This is needed when adding a notify-request. This may grow in future */
struct SI_Notify
{
	struct MsgPort *notify_port; /* Message port for notify-messages	*/
	WORD signal;				 /* Signal NUMBER if you use signals	*/
	LONG reserved[2];			 /* Reserved for future use		*/
};

/* Flags for AddNotify () */
#define AN_USE_MESSAGES (1 << 0)

/*
 * cpu usage
 *
 */

/* This is needed when querying cpu usage */
struct SI_CpuUsage
{
	ULONG total_used_cputime; /* Total used cputime in seconds	*/
	ULONG total_elapsed_time; /* Total used+idle cputime in seconds	*/

	ULONG used_cputime_lastsec;	   /* Used cputime during last second	*/
	ULONG used_cputime_lastsec_hz; /* 100 * lastsec / lastsec_hz = CPU %	*/

	ULONG recent_used_cputime;	  /* Recently used cputime		*/
	ULONG recent_used_cputime_hz; /* 100 * recent / hz = RECENT CPU %	*/
	UWORD recent_seconds;		  /* "recent" means this many seconds	*/

	ULONG involuntary_csw; /* Involuntary context switches 	*/
	ULONG voluntary_csw;   /* Voluntary context switches		*/
	ULONG total_csw;	   /* Total # of context switches		*/

	ULONG involuntary_csw_lastsec; /* Involuntary csws during last second  */
	ULONG voluntary_csw_lastsec;   /* Voluntary csws during last second	*/
	ULONG total_csw_lastsec;	   /* Total # of csws during last second	*/

	LONG reserved[12]; /* Reserved for future use		*/
};

/* These bits are used in cpu_usage_implemented-field */
#define CPU_USAGEB_TOTAL_IMPLEMENTED 0
#define CPU_USAGEB_LASTSEC_IMPLEMENTED 1
#define CPU_USAGEB_RECENT_IMPLEMENTED 2
#define CPU_USAGEB_IVVOCSW_IMPLEMENTED 3
#define CPU_USAGEB_TOTALCSW_IMPLEMENTED 4
#define CPU_USAGEB_IVVOCSW_LASTSEC_IMPLEMENTED 5
#define CPU_USAGEB_TOTALCSW_LASTSEC_IMPLEMENTED 6

#define CPU_USAGEF_TOTAL_IMPLEMENTED (1L << 0)
#define CPU_USAGEF_LASTSEC_IMPLEMENTED (1L << 1)
#define CPU_USAGEF_RECENT_IMPLEMENTED (1L << 2)
#define CPU_USAGEF_IVVOCSW_IMPLEMENTED (1L << 3)
#define CPU_USAGEF_TOTALCSW_IMPLEMENTED (1L << 4)
#define CPU_USAGEF_IVVOCSW_LASTSEC_IMPLEMENTED (1L << 5)
#define CPU_USAGEF_TOTALCSW_LASTSEC_IMPLEMENTED (1L << 6)

/*
 * task cpu usage
 *
 */

/* This is needed when querying cpu usage of a task */
struct SI_TaskCpuUsage
{
	ULONG total_used_cputime; /* Total used cputime  			*/
	ULONG total_used_time_hz; /* used_cputime / hz = cputime in secs	*/
	ULONG total_elapsed_time; /* Total used+idle cputime in seconds   */

	ULONG used_cputime_lastsec;	   /* Used cputime during last second	*/
	ULONG used_cputime_lastsec_hz; /* 100 * lastsec / lastsec_hz = CPU %	*/

	ULONG recent_used_cputime;	  /* Recently used cputime		*/
	ULONG recent_used_cputime_hz; /* 100 * recent / hz = RECENT CPU %	*/
	UWORD recent_seconds;		  /* "recent" means this many seconds	*/

	ULONG involuntary_csw; /* Involuntary context switches 	*/
	ULONG voluntary_csw;   /* Voluntary context switches		*/
	ULONG total_csw;	   /* Total # of context switches		*/

	ULONG involuntary_csw_lastsec; /* Involuntary csws during last second	*/
	ULONG voluntary_csw_lastsec;   /* Voluntary csws during last second	*/
	ULONG total_csw_lastsec;	   /* Total # of csws during last second	*/

	LONG reserved[8]; /* Reserved for future use		*/
};

#ifndef __amigaos3__
	#pragma pack()
#endif

/* These bits are used in cpu_usage_implemented-field */
#define TASK_CPU_USAGEB_TOTAL_IMPLEMENTED 0
#define TASK_CPU_USAGEB_LASTSEC_IMPLEMENTED 1
#define TASK_CPU_USAGEB_RECENT_IMPLEMENTED 2
#define TASK_CPU_USAGEB_IVVOCSW_IMPLEMENTED 3
#define TASK_CPU_USAGEB_TOTALCSW_IMPLEMENTED 4
#define TASK_CPU_USAGEB_IVVOCSW_LASTSEC_IMPLEMENTED 5
#define TASK_CPU_USAGEB_TOTALCSW_LASTSEC_IMPLEMENTED 6

#define TASK_CPU_USAGEF_TOTAL_IMPLEMENTED (1L << 0)
#define TASK_CPU_USAGEF_LASTSEC_IMPLEMENTED (1L << 1)
#define TASK_CPU_USAGEF_RECENT_IMPLEMENTED (1L << 2)
#define TASK_CPU_USAGEF_IVVOCSW_IMPLEMENTED (1L << 3)
#define TASK_CPU_USAGEF_TOTALCSW_IMPLEMENTED (1L << 4)
#define TASK_CPU_USAGEF_IVVOCSW_LASTSEC_IMPLEMENTED (1L << 5)
#define TASK_CPU_USAGEF_TOTALCSW_LASTSEC_IMPLEMENTED (1L << 6)

#endif /* LIBRARIES_SYSINFO_H */
