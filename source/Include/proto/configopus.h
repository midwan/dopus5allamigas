#ifndef PROTO_CONFIGOPUS_H
#define PROTO_CONFIGOPUS_H

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
#ifndef LIBRARIES_DOPUS5_H
	#include <libraries/configopus.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
	#ifndef __USE_BASETYPE__
extern struct Library *ConfigOpusBase;
	#else
extern struct Library* ConfigOpusBase;
	#endif /* __USE_BASETYPE__ */
#endif	   /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
	#include <interfaces/configopus.h>
	#ifdef __USE_INLINE__
		#include <inline4/configopus.h>
	#endif /* __USE_INLINE__ */
	#ifndef CLIB_CONFIGOPUS_PROTOS_H
		#define CLIB_CONFIGOPUS_PROTOS_H 1
	#endif /* CLIB_CONFIGOPUS_PROTOS_H */
	#ifndef __NOGLOBALIFACE__
extern struct ConfigOpusIFace *IConfigOpus;
	#endif /* __NOGLOBALIFACE__ */
#else	   /* __amigaos4__ */
	#ifndef CLIB_CONFIGOPUS_PROTOS_H
		#include <clib/configopus_protos.h>
	#endif /* CLIB_CONFIGOPUS_PROTOS_H */
	#if defined(__GNUC__)
		#ifdef __AROS__
			#include <defines/configopus.h>
		#else /* __AROS__ */
			#ifndef __PPC__
				#include <inline/configopus.h>
			#else /* __PPC__ */
				#include <ppcinline/configopus.h>
			#endif /* __PPC__ */
		#endif	   /* __AROS__ */
	#elif defined(__VBCC__)
		#ifndef __PPC__
			#include <inline/configopus_protos.h>
		#endif /* __PPC__ */
	#else	   /* __GNUC__ */
		#include <pragmas/configopus_pragmas.h>
	#endif /* __GNUC__ */
#endif	   /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_CONFIGOPUS_H */
