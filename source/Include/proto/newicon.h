#ifndef PROTO_NEWICON_H
#define PROTO_NEWICON_H

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
#ifndef LIBRARIES_NEWICON_H
	#include <libraries/newicon.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
	#if defined(__MORPHOS__)
extern struct NewIconBase *NewIconBase;
	#elif !defined(__USE_BASETYPE__)
extern struct Library* NewIconBase;
	#else
extern struct Library* NewIconBase;
	#endif /* __USE_BASETYPE__ */
#endif	   /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
	#include <interfaces/newicon.h>
	#ifdef __USE_INLINE__
		#include <inline4/newicon.h>
	#endif /* __USE_INLINE__ */
	#ifndef CLIB_NEWICON_PROTOS_H
		#define CLIB_NEWICON_PROTOS_H 1
	#endif /* CLIB_NEWICON_PROTOS_H */
	#ifndef __NOGLOBALIFACE__
extern struct NewIconIFace *INewIcon;
	#endif /* __NOGLOBALIFACE__ */
#else	   /* __amigaos4__ */
	#ifndef CLIB_NEWICON_PROTOS_H
		#include <clib/newicon_protos.h>
	#endif /* CLIB_NEWICON_PROTOS_H */
	#if defined(__GNUC__)
		#ifdef __AROS__
			#include <defines/newicon.h>
		#else /* __AROS__ */
			#ifndef __PPC__
				#include <inline/newicon.h>
			#else /* __PPC__ */
				#include <ppcinline/newicon.h>
			#endif /* __PPC__ */
		#endif	   /* __AROS__ */
	#elif defined(__VBCC__)
		#ifndef __PPC__
			#include <inline/newicon_protos.h>
		#endif /* __PPC__ */
	#else	   /* __GNUC__ */
		#include <pragmas/newicon_pragmas.h>
	#endif /* __GNUC__ */
#endif	   /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_NEWICON_H */
