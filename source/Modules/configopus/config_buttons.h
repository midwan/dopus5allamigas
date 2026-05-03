#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct _bank_node
{
	struct Node node;
	Cfg_ButtonBank *bank;		  // Bank pointer
	Cfg_ButtonBank *bank_backup;  // Backup of bank
	IPCData *button_ipc;		  // IPC of button bank process
	struct ListLock proc_list;	  // List of launched processes
	short pending_quit;			  // Quit pending on this bank
	IPCMessage *quit_msg;		  // Quit message
	short toolbar;				  // Node is a toolbar
} bank_node;

typedef struct
{
	struct Node node;
	Cfg_Button *button;
	IPCData *editor;
} edit_node;

typedef struct
{
	NewConfigWindow newwin;	 // New window definition
	struct Window *window;	 // Our window
	ObjectList *objlist;	 // Object list

	struct List bank_list;	// List of banks we know about
	bank_node *bank_node;	// Current bank we're editing

	struct FontRequester *font_req;	 // Font requester
	IPCData *ipc;					 // Our IPC port
	IPCData *owner_ipc;				 // Owner's IPC port

	struct MsgPort *app_port;  // Application port

	struct MsgPort *clip_port;			 // Clipboard port
	struct ClipboardHandle clip_handle;	 // Clipboard IO handle
	struct List clip_list;				 // List of clipped buttons
	long clip_count;					 // Count of entries in list
	struct Rectangle clip_rect;			 // Button clipboard position
	BOOL clip_valid;					 // Current clipboard is valid
	struct Gadget *clip_scroller;		 // Clipboard scroller

	DOpusScreenData screen_data;  // Screen data

	Cfg_Button *last_clip_draw;	 // Last button shown in clipboard

	short select_col;  // Selected column
	short select_row;  // Selected row

	struct List edit_list;	// List of button editors

	struct IBox window_pos;	 // Window position
	BOOL pos_valid;			 // Position valid
	ConfigWindow win_dims;	 // Window dimensions

	ULONG command_list;

	struct AppWindow *appwindow;

	BOOL change;

	IPCData *paint_box;
	PaletteBoxData palette_data;

	struct Window *clip_window;	 // Clipboard window
	ObjectList *clip_objlist;
	struct IBox clip_window_pos;
	BOOL clip_pos_valid;

	CfgDragInfo drag;

	ImageRemap remap;
} config_buttons_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

void _config_buttons_update(config_buttons_data *);
BOOL _config_buttons_open(config_buttons_data *data, struct Screen *screen);
void _config_buttons_cleanup(config_buttons_data *data);
void _config_buttons_new_bank(config_buttons_data *, Cfg_ButtonBank *, IPCData *);
void _config_buttons_remove_bank(config_buttons_data *, bank_node *, BOOL, BOOL);
// APTR STDARGS AllocAslRequestTags(unsigned long type,Tag tag1,...);
void _config_buttons_refresh(config_buttons_data *data, bank_node *, ULONG type);
void _config_buttons_reset(config_buttons_data *data);

BOOL _config_buttons_rows_add(config_buttons_data *data, short);
BOOL _config_buttons_columns_add(config_buttons_data *data, short);
BOOL _config_buttons_rows_remove(config_buttons_data *data);
BOOL _config_buttons_columns_remove(config_buttons_data *data);
BOOL _config_buttons_remove_fix(config_buttons_data *data, short old_count);
short _config_buttons_remove_empty_rows(config_buttons_data *data, short max);
short _config_buttons_remove_empty_columns(config_buttons_data *data, short max);
void _config_buttons_remove_rows(config_buttons_data *data, short pos, short count);
void _config_buttons_remove_columns(config_buttons_data *data, short pos, short count);
BOOL _config_buttons_handle_key(config_buttons_data *data, UWORD code, UWORD qual);
BOOL _config_buttons_delete_row(config_buttons_data *data, short);
BOOL _config_buttons_delete_column(config_buttons_data *data, short);

short _config_buttons_get_colrow(config_buttons_data *data, short *col, short *row);
Cfg_Button *_config_buttons_get_button(config_buttons_data *data, short col, short row);
void _config_buttons_flash(config_buttons_data *data, long);
void _config_buttons_redraw_button(config_buttons_data *data, bank_node *, Cfg_Button *button);
BOOL _config_buttons_create_new(config_buttons_data *data, struct List *list, short count);
void _config_buttons_edit_button(config_buttons_data *data, short col, short row);
BOOL _config_buttons_receive_edit(config_buttons_data *data, ButtonReturn *ret, BOOL);

void button_to_clipboard(config_buttons_data *data, Cfg_Button *button);
void button_save_clipboard(config_buttons_data *data, BOOL save);
void button_load_clipboard(config_buttons_data *data);
void show_button_clipboard(config_buttons_data *data, BOOL move);
void button_draw(Cfg_Button *,
				 struct IBox *,
				 struct Rectangle *,
				 struct Window *,
				 short,
				 Cfg_ButtonFunction *,
				 short *,
				 short,
				 short,
				 short,
				 short,
				 struct Library *,
				 struct Library *);
void _config_buttons_fix_controls(config_buttons_data *data);
void _config_buttons_drop_button(config_buttons_data *data, short x, short y);
void _config_buttons_fake_return(config_buttons_data *data, Cfg_Button *orig_button, Cfg_Button *new_button);
void _button_to_bank(config_buttons_data *data, Cfg_Button *button, short x, short y);
void config_buttons_show_clipboard(config_buttons_data *data);
void config_buttons_hide_clipboard(config_buttons_data *data);

void button_receive_appmsg(config_buttons_data *data, struct AppMessage *msg);

#define BUTREFRESH_SELECTOR (1 << 0)
#define BUTREFRESH_FONT (1 << 1)
#define BUTREFRESH_RESIZE (1 << 2)
#define BUTREFRESH_REFRESH (1 << 3)

#define WINDOW_BUTTONS 0x4000004

#define DRAW_MASK 1
#define DRAW_FIRST 2

extern const MenuData button_toolbar_menu[], button_menu[];

Cfg_Button *button_create_drop(config_buttons_data *data,
							   APTR memory,
							   struct WBArg *arg,
							   Cfg_Instruction *ins,
							   unsigned long flags,
							   struct Library *DOSBase,
							   struct Library *DOpusBase);
