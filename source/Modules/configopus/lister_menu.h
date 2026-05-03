enum {
	TYPE_MENU,
	TYPE_HOTKEYS,
	TYPE_SCRIPTS,
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	char menu_name[256];  // Current bank path
	IPCData *ipc;		  // Our IPC
	IPCData *owner_ipc;	  // Owner IPC

	struct ListLock proc_list;	// Launched process list
	struct List edit_list;		// List of button editors

	NewConfigWindow newwin;
	struct Window *window;	// Window pointer
	ObjectList *objlist;	// Object list

	Cfg_ButtonBank *bank;  // Current button bank

	short change;	   // Has anything changed?
	short ret_change;  // Return change?

	struct IBox window_pos;
	BOOL pos_valid;
	ConfigWindow dims;

	Att_List *menu_list;  // Menu list
	Att_Node *sel_item;	  // Selected item

	char *title;

	unsigned long seconds, micros;

	ULONG command_list;

	struct AppWindow *app_window;
	struct MsgPort *app_port;

	char work_buf[256];

	Cfg_ButtonBank *restore_bank;
	char last_save_name[256];

	char *default_name;

	unsigned long tick_count;
	unsigned long last_tick;

	short type;

	CfgDragInfo drag;

	short flags;

	short font_size;
	struct IBox old_pos;

	Att_List *script_list;
} lister_menu_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

#define LMF_NEW_ENTRY (1 << 0)

BOOL lister_menu_open(lister_menu_data *data, struct Screen *screen);
void lister_menu_close(lister_menu_data *data, BOOL free);
Cfg_ButtonBank *lister_menu_load_menu(lister_menu_data *data, Cfg_ButtonBank *, char *);
void lister_menu_fix_gadgets(lister_menu_data *data);
void lister_menu_end_edit(lister_menu_data *data, BOOL disable, Att_Node *);
void lister_menu_edit_item(lister_menu_data *data);
BOOL lister_menu_receive_edit(lister_menu_data *data, FunctionReturn *ret);
BOOL lister_menu_save(lister_menu_data *data, unsigned short gadgetid);
short lister_menu_check_change(lister_menu_data *data, BOOL can_cancel);
BOOL lister_menu_load(lister_menu_data *data, short new);
void lister_menu_update_bank(lister_menu_data *data);
void lister_menu_end_drag(lister_menu_data *data, BOOL ok);
Att_Node *lister_menu_add_item(lister_menu_data *data, Att_Node *insert);
void lister_menu_build_buttons(lister_menu_data *data);
Att_Node *lister_menu_new_item(lister_menu_data *, Cfg_ButtonFunction *, Cfg_Button *);
void lister_menu_set_name(lister_menu_data *data, char *name);
void scripts_check_bank(lister_menu_data *data);
void lister_menu_fix_functionlist(lister_menu_data *data);
void lister_menu_del_item(lister_menu_data *data, Att_Node *item);

extern ConfigWindow hotkeys_window, scripts_window;
extern ObjectDef hotkeys_objects[], scripts_objects[];
extern MenuData _lister_menu_menus[];
