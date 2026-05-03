#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	IPCData *ipc;
	struct Screen *screen;
	struct DOpusLocale *locale;
	struct Window *window;
	ObjectList *objlist;
	ObjectList *option_list;
	GL_Object *edit_area;
	Att_List *options;
	struct IBox pos;
	BOOL pos_ok;
	short option;

	CFG_SETS *settings;
	Cfg_Environment *env;
	APTR memory;

	struct MinList path_formats;  // Path format list
	Att_List *path_list;		  // List of paths
	Att_Node *sel_path;			  // Selected path

	unsigned short flags;

	ULONG seconds, micros;

	IPCData *main_ipc;
	char *settings_name;
	char last_saved[256];
} config_settings_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

#define CFGSETF_NEW_PATH (1 << 0)

BOOL _config_settings_open(config_settings_data *);
void _config_settings_cleanup(config_settings_data *);
void _config_settings_close(config_settings_data *);
void _config_settings_set(config_settings_data *);
void _config_settings_store(config_settings_data *);
void _config_sets_end_path_format(config_settings_data *, short);
void _config_sets_add_path_format(config_settings_data *);
short _config_sets_edit_path_format(config_settings_data *data);
BOOL config_settings_save(config_settings_data *data, short saveas);
void config_settings_open(config_settings_data *data, unsigned short);
BOOL config_settings_popkey(config_settings_data *);

extern MenuData button_toolbar_menu[];
