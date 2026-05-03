/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_MODULE_H
#define _PPCINLINE_MODULE_H

#ifndef __PPCINLINE_MACROS_H
	#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef MODULE_BASE_NAME
	#define MODULE_BASE_NAME ModuleBase
#endif /* !MODULE_BASE_NAME */

#define Module_Identify(__p0) \
	LP1(36, ModuleInfo *, Module_Identify, int, __p0, d0, , MODULE_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Module_Entry(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(30,                                              \
		int,                                             \
		Module_Entry,                                    \
		struct List *,                                   \
		__p0,                                            \
		a0,                                              \
		struct Screen *,                                 \
		__p1,                                            \
		a1,                                              \
		IPCData *,                                       \
		__p2,                                            \
		a2,                                              \
		IPCData *,                                       \
		__p3,                                            \
		a3,                                              \
		ULONG,                                           \
		__p4,                                            \
		d0,                                              \
		ULONG,                                           \
		__p5,                                            \
		d1,                                              \
		,                                                \
		MODULE_BASE_NAME,                                \
		0,                                               \
		0,                                               \
		0,                                               \
		0,                                               \
		0,                                               \
		0)

#endif /* !_PPCINLINE_MODULE_H */
