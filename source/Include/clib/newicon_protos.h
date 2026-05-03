#ifndef CLIB_NEWICON_PROTOS_H
#define CLIB_NEWICON_PROTOS_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif
#ifndef LIBRARIES_NEWICON_H
	#include <libraries/newicon.h>
#endif
struct NewDiskObject *GetNewDiskObject(UBYTE *name);
BOOL PutNewDiskObject(UBYTE *name, struct NewDiskObject *diskobj);
void FreeNewDiskObject(struct NewDiskObject *diskobj);
BOOL newiconPrivate1(struct NewDiskObject *diskobj);
UBYTE **newiconPrivate2(struct NewDiskObject *diskobj);
VOID newiconPrivate3(struct NewDiskObject *diskobj);
struct Image *RemapChunkyImage(struct ChunkyImage *cim, struct Screen *screen);
VOID FreeRemappedImage(struct Image *image, struct Screen *screen);
/* V40 */
struct NewDiskObject *GetDefNewDiskObject(LONG deftype);
#endif /* CLIB_NEWICON_PROTOS_H */
