/* Automatically generated header! Do not edit! */

#ifndef _INLINE_MUSIC_H
#define _INLINE_MUSIC_H

#ifndef AROS_LIBCALL_H
	#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#include <defines/aros_voidcall.h>

#ifndef MUSIC_BASE_NAME
	#define MUSIC_BASE_NAME MUSICBase
#endif /* !MUSIC_BASE_NAME */

#define ContModule() AROS_VOID_LC0( ContModule, struct Library *, MUSIC_BASE_NAME, 9, /* s */)

#define FlushModule() AROS_VOID_LC0( FlushModule, struct Library *, MUSIC_BASE_NAME, 8, /* s */)

#define IsModule(___name) \
	AROS_LC1(WORD, IsModule, AROS_LCA(char *, (___name), A0), struct Library *, MUSIC_BASE_NAME, 7, /* s */)

#define PlayFaster() AROS_VOID_LC0( PlayFaster, struct Library *, MUSIC_BASE_NAME, 11, /* s */)

#define PlayModule(___name, ___foob)          \
	AROS_LC2(WORD,                            \
			 PlayModule,                      \
			 AROS_LCA(char *, (___name), A0), \
			 AROS_LCA(BOOL, (___foob), D0),   \
			 struct Library *,                \
			 MUSIC_BASE_NAME,                 \
			 5,                               \
			 /* s */)

#define PlaySlower() AROS_VOID_LC0( PlaySlower, struct Library *, MUSIC_BASE_NAME, 12, /* s */)

#define SetVolume(___volume) \
	AROS_VOID_LC1( SetVolume, AROS_LCA(WORD, (___volume), D0), struct Library *, MUSIC_BASE_NAME, 10, /* s */)

#define StopModule() AROS_VOID_LC0( StopModule, struct Library *, MUSIC_BASE_NAME, 6, /* s */)

#define TempoReset() AROS_VOID_LC0( TempoReset, struct Library *, MUSIC_BASE_NAME, 13, /* s */)

#endif /* !_INLINE_MUSIC_H */
