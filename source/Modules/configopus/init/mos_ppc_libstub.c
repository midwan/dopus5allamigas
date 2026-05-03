#include "module_deps.h"

LIBSTUB(L_Config_Settings,
		int,
		REG(a0, Cfg_Environment *env),
		REG(a1, struct Screen *screen),
		REG(a2, IPCData *ipc),
		REG(a3, IPCData *main_ipc),
		REG(d0, char *settings_name))
{
	return L_Config_Settings(
		(Cfg_Environment *)REG_A0, (struct Screen *)REG_A1, (IPCData *)REG_A2, (IPCData *)REG_A3, (char *)REG_D0);
}
LIBSTUB(L_Config_Environment,
		unsigned long,
		REG(a0, Cfg_Environment *env),
		REG(a1, struct Screen *screen),
		REG(a2, UWORD *pen_table),
		REG(a3, IPCData *ipc),
		REG(a4, IPCData *main_ipc),
		REG(d0, UWORD pen_alloc),
		REG(a5, ULONG *change_flags),
		REG(d1, char *settings_name),
		REG(d2, Att_List *script_list))
{
	return L_Config_Environment((Cfg_Environment *)REG_A0,
								(struct Screen *)REG_A1,
								(UWORD *)REG_A2,
								(IPCData *)REG_A3,
								(IPCData *)REG_A4,
								(UWORD)REG_D0,
								(ULONG *)REG_A5,
								(char *)REG_D1,
								(Att_List *)REG_D2);
}
LIBSTUB(L_Config_Buttons,
		int,
		REG(a0, ButtonsStartup *startup),
		REG(a1, IPCData *ipc),
		REG(a2, IPCData *owner_ipc),
		REG(a3, struct Screen *screen),
		REG(d0, ULONG command_list))
{
	return L_Config_Buttons(
		(ButtonsStartup *)REG_A0, (IPCData *)REG_A1, (IPCData *)REG_A2, (struct Screen *)REG_A3, (ULONG)REG_D0);
}
LIBSTUB(L_ConvertConfig, BOOL, REG(a0, char *name), REG(a1, struct Screen *parent), REG(a2, IPCData *owner_ipc))
{
	return L_ConvertConfig((char *)REG_A0, (struct Screen *)REG_A1, (IPCData *)REG_A2);
}
LIBSTUB(L_Config_Filetypes,
		short,
		REG(a0, struct Screen *screen),
		REG(a1, IPCData *ipc),
		REG(a2, IPCData *owner_ipc),
		REG(d0, ULONG command_list),
		REG(a3, char *name))
{
	return L_Config_Filetypes(
		(struct Screen *)REG_A0, (IPCData *)REG_A1, (IPCData *)REG_A2, (ULONG)REG_D0, (char *)REG_A3);
}
LIBSTUB(L_Config_ListerButtons,
		long,
		REG(a0, char *bank_name),
		REG(a1, IPCData *ipc),
		REG(a2, IPCData *owner_ipc),
		REG(a3, struct Screen *screen),
		REG(a4, Cfg_ButtonBank *def_bank),
		REG(d0, ULONG command_list),
		REG(d1, LONG initial))
{
	return L_Config_ListerButtons((char *)REG_A0,
								  (IPCData *)REG_A1,
								  (IPCData *)REG_A2,
								  (struct Screen *)REG_A3,
								  (Cfg_ButtonBank *)REG_A4,
								  (ULONG)REG_D0,
								  (LONG)REG_D1);
}
LIBSTUB(L_Config_Menu,
		ULONG,
		REG(a0, char *menu_name),
		REG(a1, IPCData *ipc),
		REG(a2, IPCData *owner_ipc),
		REG(a3, struct Screen *screen),
		REG(a4, Cfg_ButtonBank *def_bank),
		REG(a5, char *title),
		REG(d0, ULONG command_list),
		REG(d1, char *default_name),
		REG(d2, short type),
		REG(d3, Att_List *script_list))
{
	return L_Config_Menu((char *)REG_A0,
						 (IPCData *)REG_A1,
						 (IPCData *)REG_A2,
						 (struct Screen *)REG_A3,
						 (Cfg_ButtonBank *)REG_A4,
						 (char *)REG_A5,
						 (ULONG)REG_D0,
						 (char *)REG_D1,
						 (short)REG_D2,
						 (Att_List *)REG_D3);
}
LIBSTUB(L_Config_Menus,
		ULONG,
		REG(a0, IPCData *ipc),
		REG(a1, IPCData *owner_ipc),
		REG(a2, struct Screen *screen),
		REG(a3, Cfg_ButtonBank *def_bank),
		REG(d0, ULONG command_list),
		REG(d1, ULONG type),
		REG(d2, char *menu_path))
{
	return L_Config_Menus((IPCData *)REG_A0,
						  (IPCData *)REG_A1,
						  (struct Screen *)REG_A2,
						  (Cfg_ButtonBank *)REG_A3,
						  (ULONG)REG_D0,
						  (ULONG)REG_D1,
						  (char *)REG_D2);
}
LIBSTUB(L_EditFiletype,
		Cfg_Filetype *,
		REG(a0, Cfg_Filetype *type),
		REG(a1, struct Window *window),
		REG(a2, IPCData *ipc),
		REG(a3, IPCData *dopus_ipc),
		REG(d0, ULONG flags))
{
	return L_EditFiletype(
		(Cfg_Filetype *)REG_A0, (struct Window *)REG_A1, (IPCData *)REG_A2, (IPCData *)REG_A3, (ULONG)REG_D0);
}
LIBSTUB(L_Config_EditFunction,
		Cfg_Function *,
		REG(a0, IPCData *ipc),
		REG(a1, IPCData *opus_ipc),
		REG(a2, struct Window *window),
		REG(a3, Cfg_Function *function),
		REG(a4, APTR memory),
		REG(d0, ULONG command_list))
{
	return L_Config_EditFunction((IPCData *)REG_A0,
								 (IPCData *)REG_A1,
								 (struct Window *)REG_A2,
								 (Cfg_Function *)REG_A3,
								 (APTR)REG_A4,
								 (ULONG)REG_D0);
}
LIBSTUB(L_ShowPaletteBox,
		long,
		REG(a0, struct Window *parent),
		REG(a1, DOpusScreenData *screen_data),
		REG(a2, short *fgpen),
		REG(a3, short *bgpen),
		REG(a4, struct TextAttr *font),
		REG(a5, ColourSpec32 *spec),
		REG(d0, short *spec_pen))
{
	return L_ShowPaletteBox((struct Window *)REG_A0,
							(DOpusScreenData *)REG_A1,
							(short *)REG_A2,
							(short *)REG_A3,
							(struct TextAttr *)REG_A4,
							(ColourSpec32 *)REG_A5,
							(short *)REG_D0);
}
LIBSTUB(L_FunctionExportASCII,
		BOOL,
		REG(a0, char *filename),
		REG(a1, Cfg_Button *button),
		REG(a2, Cfg_Function *function),
		REG(d0, ULONG a4))
{
	return L_FunctionExportASCII((char *)REG_A0, (Cfg_Button *)REG_A1, (Cfg_Function *)REG_A2, (ULONG)REG_D0);
}