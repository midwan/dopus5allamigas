#ifndef PROTO_SYSINFO_H
#define PROTO_SYSINFO_H

/*
**	$Id$
**	Includes Release 50.1
**
**	Prototype/inline/pragma header file combo
**
**	Copyright (c) 2010 Hyperion Entertainment CVBA.
**	All Rights Reserved.
*/

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif
#ifndef LIBRARIES_SYSINFO_H
	#include <libraries/SysInfo.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
	#ifndef __USE_BASETYPE__
extern struct Library *SysInfoBase;
	#else
extern struct Library* SysInfoBase;
	#endif /* __USE_BASETYPE__ */
#endif	   /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
	#include <interfaces/SysInfo.h>
	#ifdef __USE_INLINE__
		#include <inline4/SysInfo.h>
	#endif /* __USE_INLINE__ */
	#ifndef CLIB_SYSINFO_PROTOS_H
		#define CLIB_SYSINFO_PROTOS_H 1
	#endif /* CLIB_SYSINFO_PROTOS_H */
	#ifndef __NOGLOBALIFACE__
extern struct SysInfoIFace *ISysInfo;
	#endif /* __NOGLOBALIFACE__ */
#else	   /* __amigaos4__ */
	#ifndef CLIB_SYSINFO_PROTOS_H
		#include <clib/SysInfo_protos.h>
	#endif /* CLIB_SYSINFO_PROTOS_H */
	#if defined(__GNUC__)
		#ifdef __AROS__
			#include <defines/SysInfo.h>
		#else /* __AROS__ */
			#ifndef __PPC__
				#include <inline/SysInfo.h>
			#else /* __PPC__ */
				#include <ppcinline/SysInfo.h>
			#endif /* __PPC__ */
		#endif	   /* __AROS__ */
	#elif defined(__VBCC__)
		#ifndef __PPC__
			#include <inline/sysinfo_protos.h>
		#endif /* __PPC__ */
	#else	   /* __GNUC__ */
		#include <pragmas/sysinfo_pragmas.h>
	#endif /* __GNUC__ */
#endif	   /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_SYSINFO_H */
