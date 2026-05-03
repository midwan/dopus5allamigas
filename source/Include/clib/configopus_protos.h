#ifndef _CONFIGOPUS
#define _CONFIGOPUS

void Test(void);
int Config_Settings(Cfg_Environment *, struct Screen *, IPCData *, IPCData *, char *);
unsigned long Config_Environment(Cfg_Environment *,
								 struct Screen *,
								 UWORD *,
								 IPCData *,
								 IPCData *,
								 UWORD,
								 ULONG *,
								 char *,
								 Att_List *);
int Config_Buttons(ButtonsStartup *, IPCData *, IPCData *, struct Screen *, ULONG);
short Config_Filetypes(struct Screen *, IPCData *, IPCData *, ULONG, char *);
ULONG Config_ListerButtons(char *, IPCData *, IPCData *, struct Screen *, Cfg_ButtonBank *, ULONG, LONG);
ULONG Config_Menu(char *,
				  IPCData *,
				  IPCData *,
				  struct Screen *,
				  Cfg_ButtonBank *,
				  char *,
				  ULONG,
				  char *,
				  short,
				  Att_List *);
BOOL ConvertConfig(char *, struct Screen *, IPCData *);
ULONG Config_Menus(IPCData *, IPCData *, struct Screen *, Cfg_ButtonBank *, ULONG, ULONG, char *);
Cfg_Function *Config_EditFunction(IPCData *, IPCData *, struct Window *, Cfg_Function *, APTR, ULONG);
long ShowPaletteBox(struct Window *, DOpusScreenData *, short *, short *, struct TextAttr *, ColourSpec32 *, short *);
BOOL FunctionExportASCII(char *, Cfg_Button *, Cfg_Function *, ULONG);

// Edit filetype
Cfg_Filetype *EditFiletype(Cfg_Filetype *, struct Window *, IPCData *, IPCData *, ULONG);

#endif
