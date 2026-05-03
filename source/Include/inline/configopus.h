/* Automatically generated header (sfdc 1.11)! Do not edit! */

#ifndef _INLINE_CONFIGOPUS_H
#define _INLINE_CONFIGOPUS_H

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef CONFIGOPUS_BASE_NAME
#define CONFIGOPUS_BASE_NAME ConfigOpusBase
#endif /* !CONFIGOPUS_BASE_NAME */

#define Config_Settings(___env, ___screen, ___ipc, ___mainipc, ___name) \
      LP5(0x1e, int, Config_Settings , Cfg_Environment *, ___env, a0, struct Screen *, ___screen, a1, IPCData *, ___ipc, a2, IPCData *, ___mainipc, a3, char *, ___name, d0,\
      , CONFIGOPUS_BASE_NAME)

#define Config_Environment(___env, ___screen, ___table, ___ipc, ___mainipc, ___alloc, ___change, ___name, ___scripts) \
      LP9A4(0x24, unsigned long, Config_Environment , Cfg_Environment *, ___env, a0, struct Screen *, ___screen, a1, UWORD *, ___table, a2, IPCData *, ___ipc, a3, IPCData *, ___mainipc, d7, UWORD, ___alloc, d0, ULONG *, ___change, d1, char *, ___name, d2, Att_List *, ___scripts, d3,\
      , CONFIGOPUS_BASE_NAME)

#define Config_removed() \
      LP0(0x2a, ULONG, Config_removed ,\
      , CONFIGOPUS_BASE_NAME)

#define Config_Buttons(___bank, ___ipc, ___owner, ___screen, ___cmdlist) \
      LP5(0x30, int, Config_Buttons , ButtonsStartup *, ___bank, a0, IPCData *, ___ipc, a1, IPCData *, ___owner, a2, struct Screen *, ___screen, a3, ULONG, ___cmdlist, d0,\
      , CONFIGOPUS_BASE_NAME)

#define ConvertConfig(___name, ___screen, ___owner) \
      LP3(0x36, BOOL, ConvertConfig , char *, ___name, a0, struct Screen *, ___screen, a1, IPCData *, ___owner, a2,\
      , CONFIGOPUS_BASE_NAME)

#define Config_Filetypes(___screen, ___ipc, ___owner, ___cmdlist, ___name) \
      LP5(0x3c, short, Config_Filetypes , struct Screen *, ___screen, a0, IPCData *, ___ipc, a1, IPCData *, ___owner, a2, ULONG, ___cmdlist, d0, char *, ___name, a3,\
      , CONFIGOPUS_BASE_NAME)

#define Config_ListerButtons(___name, ___ipc, ___owner, ___screen, ___bank, ___cmdlist, ___init) \
      LP7(0x42, ULONG, Config_ListerButtons , char *, ___name, a0, IPCData *, ___ipc, a1, IPCData *, ___owner, a2, struct Screen *, ___screen, a3, Cfg_ButtonBank *, ___bank, d0, ULONG, ___cmdlist, d1, LONG, ___init, d2,\
      , CONFIGOPUS_BASE_NAME)

#define Config_Menu(___name, ___ipc, ___owner, ___screen, ___bank, ___title, ___cmdlist, ___def, ___hotkeys, ___scripts) \
      LP10(0x48, ULONG, Config_Menu , char *, ___name, a0, IPCData *, ___ipc, a1, IPCData *, ___owner, a2, struct Screen *, ___screen, a3, Cfg_ButtonBank *, ___bank, d0, char *, ___title, d1, ULONG, ___cmdlist, d2, char *, ___def, d3, short, ___hotkeys, d4, Att_List *, ___scripts, d5,\
      , CONFIGOPUS_BASE_NAME)

#define Config_Menus(___ipc, ___owner, ___screen, ___menu, ___cmdlist, ___type, ___name) \
      LP7(0x4e, ULONG, Config_Menus , IPCData *, ___ipc, a0, IPCData *, ___owner, a1, struct Screen *, ___screen, a2, Cfg_ButtonBank *, ___menu, a3, ULONG, ___cmdlist, d0, ULONG, ___type, d1, char *, ___name, d2,\
      , CONFIGOPUS_BASE_NAME)

#define EditFiletype(___type, ___window, ___ipc, ___dopus, ___flags) \
      LP5(0x54, Cfg_Filetype *, EditFiletype , Cfg_Filetype *, ___type, a0, struct Window *, ___window, a1, IPCData *, ___ipc, a2, IPCData *, ___dopus, a3, ULONG, ___flags, d0,\
      , CONFIGOPUS_BASE_NAME)

#define Config_EditFunction(___ipc, ___opus, ___window, ___func, ___memory, ___cmdlist) \
      LP6(0x5a, Cfg_Function *, Config_EditFunction , IPCData *, ___ipc, a0, IPCData *, ___opus, a1, struct Window *, ___window, a2, Cfg_Function *, ___func, a3, APTR, ___memory, d0, ULONG, ___cmdlist, d1,\
      , CONFIGOPUS_BASE_NAME)

#define ShowPaletteBox(___window, ___data, ___fgpen, ___bgpen, ___font, ___spec, ___pen) \
      LP7(0x60, long, ShowPaletteBox , struct Window *, ___window, a0, DOpusScreenData *, ___data, a1, short *, ___fgpen, a2, short *, ___bgpen, a3, struct TextAttr *, ___font, d0, ColourSpec32 *, ___spec, d1, short *, ___pen, d2,\
      , CONFIGOPUS_BASE_NAME)

#define FunctionExportASCII(___name, ___button, ___func, ___type) \
      LP4(0x66, BOOL, FunctionExportASCII , char *, ___name, a0, Cfg_Button *, ___button, a1, Cfg_Function *, ___func, a2, ULONG, ___type, d0,\
      , CONFIGOPUS_BASE_NAME)

#endif /* !_INLINE_CONFIGOPUS_H */
