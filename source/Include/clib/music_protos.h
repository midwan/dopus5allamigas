#ifndef CLIB_MUSIC_PROTOS_H
#define CLIB_MUSIC_PROTOS_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

WORD PlayModule(char *, BOOL);
VOID StopModule(void);
WORD IsModule(char *);
VOID FlushModule(void);
VOID ContModule(void);
VOID SetVolume(WORD);
VOID PlayFaster(void);
VOID PlaySlower(void);
VOID TempoReset(void);

#endif /* CLIB_MUSIC_PROTOS_H */
