/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software

This program is free software; you can redistribute it and/or
modify it under the terms of the AROS Public License version 1.1.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
AROS Public License for more details.

The release of Directory Opus 5 under the GPL in NO WAY affects
the existing commercial status of Directory Opus for Windows.

For more information on Directory Opus for Windows please see:

				 http://www.gpsoft.com.au

*/

#define CATCOMP_NUMBERS
#include "read.strings"
#include "module_deps.h"

#define HEX_DISPLAY_WIDTH 16
#define PEN_COUNT 8

enum {
	READCOM_BASE,
	READCOM_READ,
};

#define READCHUNK_SIZE 65536
#define READ_READ_SIZE 32768

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct MinNode node;
	long size;
	char buffer[1];
} text_chunk;

typedef struct
{
	struct MinNode node;
	char *text;
	unsigned short length;
} read_line;

#define LINES_PER_BLOCK 512

typedef struct
{
	read_line lines[LINES_PER_BLOCK];
} read_block;

typedef struct
{
	struct Screen *screen;
	struct Screen *my_screen;
	struct Window *window;
	struct DrawInfo *drawinfo;
	APTR vi;
	struct Menu *menus;
	struct List boopsi_list;

	IPCData *ipc;
	char *file;

	APTR memory;

	char *text_buffer;
	long text_buffer_size;

	struct MinList text;
	read_line *top_line;
	long top_position;

	long lines;
	long columns;

	long v_visible;
	long top;
	long h_visible;
	long left;

	struct Gadget *vert_scroller;
	struct Gadget *horiz_scroller;

	short tab_size;
	short mode;

	struct Rectangle disprect;

	char title[128];
	char date[16];
	long size;

	char search_text[80];
	short search_flags;

	read_line *search_line;
	short search_char;
	short search_len;

	read_line *search_last_line;
	short search_last_linepos;
	short search_last_c_x;
	short search_last_c_xend;

	long text_lines;
	char hex_display[80];

	short ansi_pen_array[16];
	unsigned short pen_alloc;

	long style;
	struct TextFont *font;

	BOOL no_next;

	short slider_div;
	short flags;

	read_block *current_block;
	short current_line;

	char screen_name[40];

	IPCData *main_ipc;

	char *initial_search;

	long sel_top_pos;
	long sel_top_line;
	long sel_bottom_pos;
	long sel_bottom_line;
	struct Rectangle text_box;

	char line_buffer[368];

	struct AppWindow *appwindow;
	struct MsgPort *appport;

	struct List *files;
	struct Node *current;

	struct FileRequester *filereq;

	struct read_startup *startup;

	BOOL flags_ok;

	WindowID window_id;

	TimerHandle *scroll_timer;
	short scroll;

	struct IOStdReq input_req;
	// struct Library			*input_base;
	ULONG pad1;

	struct MinList text_data;
	text_chunk *search_last_chunk;

	APTR progress;
	short abort_bit;

	struct AppIcon *app_icon;
	struct DiskObject *app_diskobj;
} read_data;

struct read_startup
{
	struct List *files;
	char initial_search[80];
	IPCData *owner;
	struct IBox dims;
	BOOL got_pos;
	UWORD wheel_scroll_lines;
};
#ifndef __amigaos3__
	#pragma pack()
#endif

#define READF_DONE_ANSI (1 << 0)
#define READF_HIGH_OK (1 << 1)
#define READF_RESIZED (1 << 2)

extern const MenuData read_menus[];
extern const ConfigWindow search_window;
extern const ObjectDef search_objects[];

#define SEARCHF_NOCASE (1 << 0)
#define SEARCHF_WILDCARD (1 << 1)
#define SEARCHF_ONLYWORDS (1 << 2)
#define SEARCHF_REVERSE (1 << 3)

enum {
	MENU_BASE,
	MENU_NEXT,
	MENU_SEARCH,
	MENU_REPEAT,
	MENU_PRINT,
	MENU_QUIT,
	MENU_TAB_1,
	MENU_TAB_2,
	MENU_TAB_4,
	MENU_TAB_8,
	MENU_MODE_NORMAL,
	MENU_MODE_ANSI,
	MENU_MODE_HEX,
	MENU_SAVE,

	GAD_SEARCH_LAYOUT,
	GAD_SEARCH_TEXT,
	GAD_SEARCH_CASE,
	GAD_SEARCH_WILD,
	GAD_SEARCH_ONLYWORD,
	GAD_SEARCH_REVERSE,
	GAD_SEARCH_OKAY,
	GAD_SEARCH_CANCEL,

	MENU_USE_SCREEN,
	MENU_SCREEN_MODE,
	MENU_SELECT_FONT,

	MENU_SAVE_AS,
	MENU_TO_EDITOR,
	MENU_SELECT_EDITOR_NORMAL,
	MENU_SELECT_EDITOR_ANSI,
	MENU_SELECT_EDITOR_HEX,
};

enum {
	MODE_NORMAL,
	MODE_ANSI,
	MODE_HEX,
	MODE_SMART,
};

#define ABS(x) (((x) < 0) ? (-x) : (x))

extern const ULONG ansi_palette[PEN_COUNT];

/*struct Window *STDARGS my_OpenWindowTags(Tag tag,...);
struct Screen *STDARGS my_OpenScreenTags(Tag tag,...);
APTR STDARGS my_AllocAslRequest(ULONG type,Tag tag,...);
ULONG STDARGS my_SetGadgetAttrs(struct Gadget *gadget,struct Window *window,Tag tag,...);
APTR STDARGS my_AllocAslRequestTags(ULONG type,Tag tag,...);
BOOL STDARGS my_LayoutMenus(struct Menu *menu,APTR vi,Tag tag,...);*/
void read_parse_set(char **ptr, unsigned short *);

void read_free(read_data *, struct read_startup *, IPCData *);
BOOL read_file(read_data *, ULONG);
struct Window *read_open_window(read_data *);
void read_close_window(read_data *);
BOOL read_set_mode(read_data *, short, BOOL);
BOOL read_view(read_data *);
void read_calc_size(read_data *);
void read_init(read_data *);
void read_build_title(read_data *);
void read_show_text(read_data *, long, long, short);
void read_update_text(read_data *, long, long, short);
void read_display_text(read_data *, read_line *, long, text_chunk *);
void read_get_width(read_data *);
void read_search(read_data *, BOOL);
BOOL read_search_line(read_data *, read_line *, short);
void read_show_search(read_data *, long);
void read_remove_highlight(read_data *);
BOOL read_pick_screen_mode(read_data *);
void read_pick_font(read_data *);
read_line ASM *read_alloc_node(REG(a0, read_data *));
void read_print(read_data *);
void read_clipboard_copy(read_data *);
void clip_write_data(struct IOClipReq *, char *, long);
void read_save(read_data *);
void read_pick_editor(read_data *, short);
void read_check_bounds(read_data *);
void read_parse_string(char **, char *, short);
text_chunk *read_hex_pointer(read_data *, long *);
