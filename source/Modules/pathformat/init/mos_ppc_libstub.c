#include "module_deps.h"

LIBSTUB(L_Module_Identify, ModuleInfo *, REG(d0, int num))
{
	return L_Module_Identify((int)REG_D0);
}
LIBSTUB(L_Module_Entry,
		int,
		REG(a0, char *args),
		REG(a1, struct Screen *screen),
		REG(a2, IPCData *ipc),
		REG(a3, IPCData *main_ipc),
		REG(d0, IPTR mod_id),
		REG(d1, IPTR mod_data))
{
	return L_Module_Entry((char *)REG_A0,
						  (struct Screen *)REG_A1,
						  (IPCData *)REG_A2,
						  (IPCData *)REG_A3,
						  (IPTR)REG_D0,
						  (IPTR)REG_D1);
}
