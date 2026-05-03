#ifndef PROTO_MULTIUSER_H
#define PROTO_MULTIUSER_H

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

/****************************************************************************/

#ifndef __NOLIBBASE__
	#ifndef __USE_BASETYPE__
extern struct Library *muBase;
	#else
extern struct Library* muBase;
	#endif /* __USE_BASETYPE__ */
#endif	   /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
	#include <interfaces/multiuser.h>
	#ifdef __USE_INLINE__
		#include <inline4/multiuser.h>
	#endif /* __USE_INLINE__ */
	#ifndef CLIB_MULTIUSER_PROTOS_H
		#define CLIB_MULTIUSER_PROTOS_H 1
	#endif /* CLIB_MULTIUSER_PROTOS_H */
	#ifndef __NOGLOBALIFACE__
extern struct muIFace *Imu;
	#endif /* __NOGLOBALIFACE__ */
#else	   /* __amigaos4__ */
	#ifndef CLIB_MULTIUSER_PROTOS_H
		#include <clib/multiuser_protos.h>
	#endif /* CLIB_MULTIUSER_PROTOS_H */
	#if defined(__GNUC__)
		#ifdef __AROS__
			#include <defines/multiuser.h>
		#else /* __AROS__ */
			#ifndef __PPC__
				#include <inline/multiuser.h>
			#else /* __PPC__ */
				#include <ppcinline/multiuser.h>
			#endif /* __PPC__ */
		#endif	   /* __AROS__ */
	#elif defined(__VBCC__)
		#ifndef __PPC__
			#include <inline/multiuser_protos.h>
		#endif /* __PPC__ */
	#else	   /* __GNUC__ */
		#include <pragmas/multiuser_pragmas.h>
	#endif /* __GNUC__ */
#endif	   /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_MULTIUSER_H */
