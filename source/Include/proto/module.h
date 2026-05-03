#ifndef PROTO_MODULE_H
#define PROTO_MODULE_H

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
#ifndef LIBRARIES_MODULE_H
	#include <libraries/module.h>
#endif
/****************************************************************************/

#ifndef __NOLIBBASE__
	#ifndef __USE_BASETYPE__
extern struct Library *ModuleBase;
	#else
extern struct Library* ModuleBase;
	#endif /* __USE_BASETYPE__ */
#endif	   /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
	#include <interfaces/module.h>
	#ifdef __USE_INLINE__
		#include <inline4/module.h>
	#endif /* __USE_INLINE__ */
	#ifndef CLIB_MODULE_PROTOS_H
		#define CLIB_MODULE_PROTOS_H 1
	#endif /* CLIB_MODULE_PROTOS_H */
	#ifndef __NOGLOBALIFACE__
extern struct ModuleIFace *IModule;
	#endif /* __NOGLOBALIFACE__ */
#else	   /* __amigaos4__ */
	#ifndef CLIB_MODULE_PROTOS_H
		#include <clib/module_protos.h>
	#endif /* CLIB_MODULE_PROTOS_H */
	#if defined(__GNUC__)
		#ifdef __AROS__
			#include <defines/module.h>
		#else /* __AROS__ */
			#ifndef __PPC__
				#include <inline/module.h>
			#else /* __PPC__ */
				#include <ppcinline/module.h>
			#endif /* __PPC__ */
		#endif	   /* __AROS__ */
	#elif defined(__VBCC__)
		#ifndef __PPC__
			#include <inline/module_protos.h>
		#endif /* __PPC__ */
	#else	   /* __GNUC__ */
		#include <pragmas/module_pragmas.h>
	#endif /* __GNUC__ */
#endif	   /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_MODULE_H */
