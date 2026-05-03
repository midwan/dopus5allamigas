#ifndef CLIB_MODULE_PROTOS_H
#define CLIB_MODULE_PROTOS_H

#ifndef EXEC_TYPES_H
	#include <exec/types.h>
#endif

int Module_Entry(struct List* files, struct Screen* screen, IPCData* ipc, IPCData* main_ipc, IPTR id, IPTR data);
ModuleInfo* Module_Identify(int num);

#endif /* CLIB_MODULE_PROTOS_H */
