/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_CONFIGOPUS_H
#define _PPCINLINE_CONFIGOPUS_H

#ifndef __PPCINLINE_MACROS_H
	#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef CONFIGOPUS_BASE_NAME
	#define CONFIGOPUS_BASE_NAME ConfigOpusBase
#endif /* !CONFIGOPUS_BASE_NAME */

#define Config_ListerButtons(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(66,                                                            \
		ULONG,                                                         \
		Config_ListerButtons,                                          \
		char *,                                                        \
		__p0,                                                          \
		a0,                                                            \
		IPCData *,                                                     \
		__p1,                                                          \
		a1,                                                            \
		IPCData *,                                                     \
		__p2,                                                          \
		a2,                                                            \
		struct Screen *,                                               \
		__p3,                                                          \
		a3,                                                            \
		Cfg_ButtonBank *,                                              \
		__p4,                                                          \
		a4,                                                            \
		ULONG,                                                         \
		__p5,                                                          \
		d0,                                                            \
		LONG,                                                          \
		__p6,                                                          \
		d1,                                                            \
		,                                                              \
		CONFIGOPUS_BASE_NAME,                                          \
		0,                                                             \
		0,                                                             \
		0,                                                             \
		0,                                                             \
		0,                                                             \
		0)

#define Config_EditFunction(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(90,                                                     \
		Cfg_Function *,                                         \
		Config_EditFunction,                                    \
		IPCData *,                                              \
		__p0,                                                   \
		a0,                                                     \
		IPCData *,                                              \
		__p1,                                                   \
		a1,                                                     \
		struct Window *,                                        \
		__p2,                                                   \
		a2,                                                     \
		Cfg_Function *,                                         \
		__p3,                                                   \
		a3,                                                     \
		APTR,                                                   \
		__p4,                                                   \
		a4,                                                     \
		ULONG,                                                  \
		__p5,                                                   \
		d0,                                                     \
		,                                                       \
		CONFIGOPUS_BASE_NAME,                                   \
		0,                                                      \
		0,                                                      \
		0,                                                      \
		0,                                                      \
		0,                                                      \
		0)

#define Config_Buttons(__p0, __p1, __p2, __p3, __p4) \
	LP5(48,                                          \
		int,                                         \
		Config_Buttons,                              \
		ButtonsStartup *,                            \
		__p0,                                        \
		a0,                                          \
		IPCData *,                                   \
		__p1,                                        \
		a1,                                          \
		IPCData *,                                   \
		__p2,                                        \
		a2,                                          \
		struct Screen *,                             \
		__p3,                                        \
		a3,                                          \
		ULONG,                                       \
		__p4,                                        \
		d0,                                          \
		,                                            \
		CONFIGOPUS_BASE_NAME,                        \
		0,                                           \
		0,                                           \
		0,                                           \
		0,                                           \
		0,                                           \
		0)

#define FunctionExportASCII(__p0, __p1, __p2, __p3) \
	LP4(102,                                        \
		BOOL,                                       \
		FunctionExportASCII,                        \
		char *,                                     \
		__p0,                                       \
		a0,                                         \
		Cfg_Button *,                               \
		__p1,                                       \
		a1,                                         \
		Cfg_Function *,                             \
		__p2,                                       \
		a2,                                         \
		ULONG,                                      \
		__p3,                                       \
		d0,                                         \
		,                                           \
		CONFIGOPUS_BASE_NAME,                       \
		0,                                          \
		0,                                          \
		0,                                          \
		0,                                          \
		0,                                          \
		0)

#define Config_Settings(__p0, __p1, __p2, __p3, __p4) \
	LP5(30,                                           \
		int,                                          \
		Config_Settings,                              \
		Cfg_Environment *,                            \
		__p0,                                         \
		a0,                                           \
		struct Screen *,                              \
		__p1,                                         \
		a1,                                           \
		IPCData *,                                    \
		__p2,                                         \
		a2,                                           \
		IPCData *,                                    \
		__p3,                                         \
		a3,                                           \
		char *,                                       \
		__p4,                                         \
		d0,                                           \
		,                                             \
		CONFIGOPUS_BASE_NAME,                         \
		0,                                            \
		0,                                            \
		0,                                            \
		0,                                            \
		0,                                            \
		0)

#define ConvertConfig(__p0, __p1, __p2) \
	LP3(54,                             \
		BOOL,                           \
		ConvertConfig,                  \
		char *,                         \
		__p0,                           \
		a0,                             \
		struct Screen *,                \
		__p1,                           \
		a1,                             \
		IPCData *,                      \
		__p2,                           \
		a2,                             \
		,                               \
		CONFIGOPUS_BASE_NAME,           \
		0,                              \
		0,                              \
		0,                              \
		0,                              \
		0,                              \
		0)

#define ShowPaletteBox(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(96,                                                      \
		long,                                                    \
		ShowPaletteBox,                                          \
		struct Window *,                                         \
		__p0,                                                    \
		a0,                                                      \
		DOpusScreenData *,                                       \
		__p1,                                                    \
		a1,                                                      \
		short *,                                                 \
		__p2,                                                    \
		a2,                                                      \
		short *,                                                 \
		__p3,                                                    \
		a3,                                                      \
		struct TextAttr *,                                       \
		__p4,                                                    \
		a4,                                                      \
		ColourSpec32 *,                                          \
		__p5,                                                    \
		a5,                                                      \
		short *,                                                 \
		__p6,                                                    \
		d0,                                                      \
		,                                                        \
		CONFIGOPUS_BASE_NAME,                                    \
		0,                                                       \
		0,                                                       \
		0,                                                       \
		0,                                                       \
		0,                                                       \
		0)

#define Config_Environment(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
	LP9(36,                                                                      \
		unsigned long,                                                           \
		Config_Environment,                                                      \
		Cfg_Environment *,                                                       \
		__p0,                                                                    \
		a0,                                                                      \
		struct Screen *,                                                         \
		__p1,                                                                    \
		a1,                                                                      \
		UWORD *,                                                                 \
		__p2,                                                                    \
		a2,                                                                      \
		IPCData *,                                                               \
		__p3,                                                                    \
		a3,                                                                      \
		IPCData *,                                                               \
		__p4,                                                                    \
		a4,                                                                      \
		UWORD,                                                                   \
		__p5,                                                                    \
		d0,                                                                      \
		ULONG *,                                                                 \
		__p6,                                                                    \
		a5,                                                                      \
		char *,                                                                  \
		__p7,                                                                    \
		d1,                                                                      \
		Att_List *,                                                              \
		__p8,                                                                    \
		d2,                                                                      \
		,                                                                        \
		CONFIGOPUS_BASE_NAME,                                                    \
		0,                                                                       \
		0,                                                                       \
		0,                                                                       \
		0,                                                                       \
		0,                                                                       \
		0)

#define Config_Filetypes(__p0, __p1, __p2, __p3, __p4) \
	LP5(60,                                            \
		short,                                         \
		Config_Filetypes,                              \
		struct Screen *,                               \
		__p0,                                          \
		a0,                                            \
		IPCData *,                                     \
		__p1,                                          \
		a1,                                            \
		IPCData *,                                     \
		__p2,                                          \
		a2,                                            \
		ULONG,                                         \
		__p3,                                          \
		d0,                                            \
		char *,                                        \
		__p4,                                          \
		a3,                                            \
		,                                              \
		CONFIGOPUS_BASE_NAME,                          \
		0,                                             \
		0,                                             \
		0,                                             \
		0,                                             \
		0,                                             \
		0)

#define EditFiletype(__p0, __p1, __p2, __p3, __p4) \
	LP5(84,                                        \
		Cfg_Filetype *,                            \
		EditFiletype,                              \
		Cfg_Filetype *,                            \
		__p0,                                      \
		a0,                                        \
		struct Window *,                           \
		__p1,                                      \
		a1,                                        \
		IPCData *,                                 \
		__p2,                                      \
		a2,                                        \
		IPCData *,                                 \
		__p3,                                      \
		a3,                                        \
		ULONG,                                     \
		__p4,                                      \
		d0,                                        \
		,                                          \
		CONFIGOPUS_BASE_NAME,                      \
		0,                                         \
		0,                                         \
		0,                                         \
		0,                                         \
		0,                                         \
		0)

#define Config_Menus(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(78,                                                    \
		ULONG,                                                 \
		Config_Menus,                                          \
		IPCData *,                                             \
		__p0,                                                  \
		a0,                                                    \
		IPCData *,                                             \
		__p1,                                                  \
		a1,                                                    \
		struct Screen *,                                       \
		__p2,                                                  \
		a2,                                                    \
		Cfg_ButtonBank *,                                      \
		__p3,                                                  \
		a3,                                                    \
		ULONG,                                                 \
		__p4,                                                  \
		d0,                                                    \
		ULONG,                                                 \
		__p5,                                                  \
		d1,                                                    \
		char *,                                                \
		__p6,                                                  \
		d2,                                                    \
		,                                                      \
		CONFIGOPUS_BASE_NAME,                                  \
		0,                                                     \
		0,                                                     \
		0,                                                     \
		0,                                                     \
		0,                                                     \
		0)

#define Config_Menu(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8, __p9) \
	LP10(72,                                                                    \
		 ULONG,                                                                 \
		 Config_Menu,                                                           \
		 char *,                                                                \
		 __p0,                                                                  \
		 a0,                                                                    \
		 IPCData *,                                                             \
		 __p1,                                                                  \
		 a1,                                                                    \
		 IPCData *,                                                             \
		 __p2,                                                                  \
		 a2,                                                                    \
		 struct Screen *,                                                       \
		 __p3,                                                                  \
		 a3,                                                                    \
		 Cfg_ButtonBank *,                                                      \
		 __p4,                                                                  \
		 a4,                                                                    \
		 char *,                                                                \
		 __p5,                                                                  \
		 a5,                                                                    \
		 ULONG,                                                                 \
		 __p6,                                                                  \
		 d0,                                                                    \
		 char *,                                                                \
		 __p7,                                                                  \
		 d1,                                                                    \
		 short,                                                                 \
		 __p8,                                                                  \
		 d2,                                                                    \
		 Att_List *,                                                            \
		 __p9,                                                                  \
		 d3,                                                                    \
		 ,                                                                      \
		 CONFIGOPUS_BASE_NAME,                                                  \
		 0,                                                                     \
		 0,                                                                     \
		 0,                                                                     \
		 0,                                                                     \
		 0,                                                                     \
		 0)

#endif /* !_PPCINLINE_CONFIGOPUS_H */
