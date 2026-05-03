#include "module_deps.h"
LIBSTUB(L_Module_Identify, ModuleInfo *, REG(d0, int num)){return L_Module_Identify(num); }
LIBSTUB(L_Module_Entry, int, REG(a0, char *args), REG(a1, struct Screen *screen), REG(a2, IPCData *ipc), REG(a3, IPCData *main_ipc), REG(d0, IPTR mod_id), REG(d1, IPTR mod_data)){return L_Module_Entry(args, screen, ipc, main_ipc, mod_id, mod_data); }
