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
/*
 *
 * dopus.library header file
 *
 */

#ifndef PROTO_DOPUS5_H
#define PROTO_DOPUS5_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif
#ifndef PROTO_DOS_H
	#include <proto/dos.h>
#endif
#ifndef PROTO_INTUITION_H
	#include <proto/intuition.h>
#endif
#ifndef PROTO_GRAPHICS_H
	#include <proto/graphics.h>
#endif
//#ifndef PROTO_LAYOUT_H
//# ifndef __MORPHOS__
//#   include <proto/layout.h>
//# endif
//#endif
#ifndef PROTO_WB_H
	#include <proto/wb.h>
#endif
#ifndef PROTO_WB_H
	#include <proto/wb.h>
#endif
#ifndef PROTO_REXXSYSLIB_H
	#include <proto/rexxsyslib.h>
#endif
#ifndef LIBRARIES_DOPUS5_H
	#include <libraries/dopus5.h>
#endif
/****************************************************************************/

#ifndef __NOLIBBASE__
	#ifndef __USE_BASETYPE__
extern struct Library *DOpusBase;
	#else
extern struct Library* DOpusBase;
	#endif /* __USE_BASETYPE__ */
#endif	   /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
	#include <interfaces/dopus5.h>
	#ifdef __USE_INLINE__
		#include <inline4/dopus5.h>
	#endif /* __USE_INLINE__ */
	#ifndef CLIB_DOPUS5_PROTOS_H
		#define CLIB_DOPUS5_PROTOS_H 1
	#endif /* CLIB_DOPUS5_PROTOS_H */
	#ifndef __NOGLOBALIFACE__
extern struct DOpusIFace *IDOpus;
	#endif /* __NOGLOBALIFACE__ */
#else	   /* __amigaos4__ */
	#ifndef CLIB_DOPUS5_PROTOS_H
		#include <clib/dopus5_protos.h>
	#endif /* CLIB_DOPUS5_PROTOS_H */
	#if defined(__GNUC__)
		#ifdef __AROS__
			#include <defines/dopus5.h>
		#else /* __AROS__ */
			#ifndef __PPC__
				#include <inline/dopus5.h>
			#else /* __PPC__ */
				#include <ppcinline/dopus5.h>
			#endif /* __PPC__ */
		#endif	   /* __AROS__ */
	#elif defined(__VBCC__)
		#ifndef __PPC__
			#include <inline/dopus5_protos.h>
		#endif /* __PPC__ */
	#else	   /* __GNUC__ */
		#include <pragmas/dopus5_pragmas.h>
	#endif /* __GNUC__ */
#endif	   /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_DOPUS5_H */
