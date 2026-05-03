#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	char bank_name[256];  // Current bank path
	IPCData *ipc;		  // Our IPC
	IPCData *owner_ipc;	  // Owner IPC

	struct ListLock proc_list;	// Launched process list

	NewConfigWindow newwin;
	struct Window *window;	// Window pointer
	ObjectList *objlist;	// Object list

	Cfg_ButtonBank *bank;  // Current button bank

	struct Rectangle display_area;	// Display area
	short display_width;			// Width of display area
	short display_count;			// Number of buttons displayed at once
	short display_left;				// Left offset of display
	short display_top;				// Top offset of display

	short button_count;	  // Number of buttons
	short button_width;	  // Width of buttons
	short button_height;  // Height of buttons

	short sel_offset;  // Selection offset
	short offset;	   // Display offset
	short old_offset;  // Last display offset

	DOpusScreenData screen_data;  // Screen data
	unsigned char pen_array[16];  // Pen array
	struct Rectangle last_sel_rect;

	GL_Object *display_obj;	 // Button display area object

	struct List edit_list;	// List of button editors

	short change;	   // Has anything changed?
	short ret_change;  // Return change?

	Point window_pos;
	BOOL pos_valid;
	ConfigWindow dims;

	DragInfo *drag;
	short drag_item;
	short drag_x, drag_y;

	TimerHandle *timer;

	unsigned long seconds, micros;
	ULONG command_list;

	struct AppWindow *app_window;
	struct MsgPort *app_port;

	char work_buf[256];

	Cfg_ButtonBank *restore_bank;
	char last_save_name[256];

	unsigned long tick_count;
	unsigned long last_tick;
} lister_buttons_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

BOOL lister_buttons_open(lister_buttons_data *data, struct Screen *screen);
void lister_buttons_close(lister_buttons_data *data, BOOL);
void lister_buttons_show(lister_buttons_data *data);
BOOL lister_buttons_select(lister_buttons_data *data, short sel, short update);
void lister_buttons_show_sel(lister_buttons_data *data, struct Rectangle *rect, short sel);
Cfg_Button *lister_buttons_sel_button(lister_buttons_data *data);
Cfg_Button *lister_buttons_get_button(lister_buttons_data *data, short offset);
void lister_buttons_edit_button(lister_buttons_data *data);
BOOL lister_buttons_receive_edit(lister_buttons_data *data, ButtonReturn *ret, BOOL redraw);
Cfg_ButtonBank *lister_buttons_load_bank(lister_buttons_data *data, Cfg_ButtonBank *, char *);
void lister_buttons_update_bank(lister_buttons_data *data);
void lister_buttons_save(lister_buttons_data *data, unsigned short gadgetid);
short lister_buttons_check_change(lister_buttons_data *data, BOOL);
BOOL lister_buttons_load(lister_buttons_data *data, BOOL new);
void lister_buttons_end_drag(lister_buttons_data *data, BOOL ok);
Cfg_Button *lister_buttons_new_button(lister_buttons_data *data, UWORD gadgetid);

extern ConfigWindow _lister_buttons_window;
extern ObjectDef _lister_buttons_objects[];
extern MenuData _lister_buttons_menus[];
