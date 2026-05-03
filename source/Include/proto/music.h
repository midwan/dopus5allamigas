#ifndef PROTO_MUSIC_H
#define PROTO_MUSIC_H

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
extern struct Library *MUSICBase;
	#else
extern struct Library* MUSICBase;
	#endif /* __USE_BASETYPE__ */
#endif	   /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
	#include <interfaces/music.h>
	#ifdef __USE_INLINE__
		#include <inline4/music.h>
	#endif /* __USE_INLINE__ */
	#ifndef CLIB_MUSIC_PROTOS_H
		#define CLIB_MUSIC_PROTOS_H 1
	#endif /* CLIB_MUSIC_PROTOS_H */
	#ifndef __NOGLOBALIFACE__
extern struct MUSICIFace *IMUSIC;
	#endif /* __NOGLOBALIFACE__ */
#else	   /* __amigaos4__ */
	#ifndef CLIB_MUSIC_PROTOS_H
		#include <clib/music_protos.h>
	#endif /* CLIB_MUSIC_PROTOS_H */
	#if defined(__GNUC__)
		#ifdef __AROS__
			#include <defines/music.h>
		#else /* __AROS__ */
			#ifndef __PPC__
				#include <inline/music.h>
			#else /* __PPC__ */
				#include <ppcinline/music.h>
			#endif /* __PPC__ */
		#endif	   /* __AROS__ */
	#elif defined(__VBCC__)
		#ifndef __PPC__
			#include <inline/music_protos.h>
		#endif /* __PPC__ */
	#else	   /* __GNUC__ */
		#include <pragmas/music_pragmas.h>
	#endif /* __GNUC__ */
#endif	   /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_MUSIC_H */
