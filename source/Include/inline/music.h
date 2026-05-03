#ifndef _INLINE_MUSIC_H
#define _INLINE_MUSIC_H

#ifndef CLIB_MUSIC_PROTOS_H
	#define CLIB_MUSIC_PROTOS_H
#endif

#ifndef __INLINE_MACROS_H
	#include <inline/macros.h>
#endif

#include <exec/types.h>

#ifndef MUSIC_BASE_NAME
	#define MUSIC_BASE_NAME MUSICBase
#endif

#define PlayModule(par1, last) LP2(0x1e, WORD, PlayModule, char *, par1, a0, BOOL, last, d0, , MUSIC_BASE_NAME)

#define StopModule() LP0NR(0x24, StopModule, , MUSIC_BASE_NAME)

#define IsModule(last) LP1(0x2a, WORD, IsModule, char *, last, a0, , MUSIC_BASE_NAME)

#define FlushModule() LP0NR(0x30, FlushModule, , MUSIC_BASE_NAME)

#define ContModule() LP0NR(0x36, ContModule, , MUSIC_BASE_NAME)

#define SetVolume(last) LP1NR(0x3c, SetVolume, WORD, last, a0, , MUSIC_BASE_NAME)

#define PlayFaster() LP0NR(0x42, PlayFaster, , MUSIC_BASE_NAME)

#define PlaySlower() LP0NR(0x48, PlaySlower, , MUSIC_BASE_NAME)

#define TempoReset() LP0NR(0x4e, TempoReset, , MUSIC_BASE_NAME)

#endif /*  _INLINE_MUSIC_H  */
