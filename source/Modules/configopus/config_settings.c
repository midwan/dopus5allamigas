#include "config_lib.h"
#include <proto/module.h>

int LIBFUNC L_Config_Settings(REG(a0, Cfg_Environment *env),
							  REG(a1, struct Screen *screen),
							  REG(a2, IPCData *ipc),
							  REG(a3, IPCData *main_ipc),
							  REG(d0, char *settings_name))
{
	return 0;
}
