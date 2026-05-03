/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_MUSIC_H
#define _PPCINLINE_MUSIC_H

#ifndef __PPCINLINE_MACROS_H
	#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef _BASE_NAME
	#define _BASE_NAME MUSICBase
#endif /* !_BASE_NAME */

#define SetVolume(__p0) LP1NR(60, SetVolume, WORD, __p0, d0, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define IsModule(__p0) LP1(42, WORD, IsModule, char *, __p0, a0, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TempoReset() LP0NR(78, TempoReset, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ContModule() LP0NR(54, ContModule, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PlaySlower() LP0NR(72, PlaySlower, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PlayFaster() LP0NR(66, PlayFaster, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FlushModule() LP0NR(48, FlushModule, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define StopModule() LP0NR(36, StopModule, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#define PlayModule(__p0, __p1) \
	LP2(30, WORD, PlayModule, char *, __p0, a0, BOOL, __p1, d0, , _BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE_MUSIC_H */
