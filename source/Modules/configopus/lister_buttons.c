#include "config_lib.h"

long LIBFUNC L_Config_ListerButtons(
	REG(a0, char *bank_name),
	REG(a1, IPCData *ipc),
	REG(a2, IPCData *owner_ipc),
	REG(a3, struct Screen *screen),
    REG(d0, Cfg_ButtonBank *def_bank0,
    REG(d1, ULONG command_list0,
    REG(d2, LONG initial))
{
	return 0;
}
