/* Automatically generated header! Do not edit! */

#ifndef _INLINE_MODULE_H
#define _INLINE_MODULE_H

#ifndef __INLINE_MACROS_H
	#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef MODULE_BASE_NAME
	#define MODULE_BASE_NAME ModuleBase
#endif /* !MODULE_BASE_NAME */

#define Module_Expunge() LP0NR(0x12, Module_Expunge, , MODULE_BASE_NAME)

#define Module_Entry(files, screen, ipc, main_ipc, id, data) \
	LP6(0x1e,                                                \
		int,                                                 \
		Module_Entry,                                        \
		struct List *,                                       \
		files,                                               \
		a0,                                                  \
		struct Screen *,                                     \
		screen,                                              \
		a1,                                                  \
		IPCData *,                                           \
		ipc,                                                 \
		a2,                                                  \
		IPCData *,                                           \
		main_ipc,                                            \
		a3,                                                  \
		ULONG,                                               \
		id,                                                  \
		d0,                                                  \
		ULONG,                                               \
		data,                                                \
		d1,                                                  \
		,                                                    \
		MODULE_BASE_NAME)

#define Module_Identify(num) LP1(0x24, ModuleInfo *, Module_Identify, int, num, d0, , MODULE_BASE_NAME)

#endif /* !_INLINE_MODULE_H */
