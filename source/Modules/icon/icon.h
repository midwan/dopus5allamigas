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
#include "icon.strings"
#include "module_deps.h"
#include <Program/main_commands.h>

#define BACKDROP_GROUP_OBJECT (1 << 30)

// Special flag for IconEdit launching
#define LAUNCHF_OPEN_UNDER_MOUSE (1 << 30)

// Our custom IPC events
#define ICONINFO_TOOLTYPE 980316
#define ICONINFO_ICON 980317

// Seperate enabled/disabled tooltypes
#define SORT_TOOLTYPE_SEPERATED 32

// RMB popup menu
enum {
	IIM_EDIT,
	IIM_PICK_EDITOR,
	IIM_BARN1,
	IIM_DISK,
	IIM_DRAWER,
	IIM_TOOL,
	IIM_PROJECT,
	IIM_BAR0,
	IIM_BORDERLESS,
	IIM_LABEL,
	IIM_BAR1,
	IIM_STRIP_NEWICONS,
	IIM_STRIP_OLD,
	IIM_BAR2,
	IIM_NEWICONS,
	IIM_OLDICONS,
	IIM_GARBAGE,

	IIM_COUNT
};

// Drag'n'drop popup menu
enum {
	IDM_OLDICONS,
	IDM_NEWICONS,
	IDM_BOTHICONS,
	IDM_BAR,
	IDM_TOOLTYPES,

	IDM_COUNT
};

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct Screen *screen;
	struct Window *window;
	IPCData *ipc;

	struct FileInfoBlock fib;
	struct InfoData info;

	NewConfigWindow new_win;
	ObjectList *list;

	char icon_name[256];
	char prog_name[256];
	char name[256];
	char title[256];
	char object_name[256];

	struct DiskObject *icon;
	struct DiskObject icon_copy;

	struct Image *image[2];
	short image_num;
	struct Rectangle icon_area;

	Att_List *prot_list;
	Att_List *tool_list;
	Att_Node *edit_tool;
	BOOL tool_new;

	struct DateTime datetime;
	char datebuf[16];
	char timebuf[16];

	char decimal_sep;

	BOOL group;

	IPCData *main_ipc;

	ULONG disktype;

	struct DiskObject *new_icon;
	ULONG icon_highlight;
	short icon_width;
	short icon_height;

	struct MsgPort *app_port;
	struct AppWindow *app_window;

	struct MsgPort *notify_port;
	APTR notify_req;

	long icon_type;

	unsigned long seconds;
	unsigned long micros;
	char buffer[256];
	char author[128];

	PopUpMenu menu;
	PopUpItem menu_items[IIM_COUNT + 1];

	PopUpMenu dropmenu;
	PopUpItem dropmenu_items[IDM_COUNT + 1];

	BOOL wb_info;
	BOOL remap;
	BOOL newicon_flag;
	BOOL strip_newicon;
	BOOL strip_oldicon;

	char path[256];
	char newicon_path[256];

	short icon_mode;

	/*struct Library		*newicon_base;
#ifdef __amigaos4__
	struct NewIconIFace 	*newicon_iface;
#endif*/
	ULONG pad1;

	struct DiskObject *new_oldicon;
	struct DiskObject *new_newicon;
	int label;
	BOOL first_remap;
	int which_images;
	int drag_item;
	int drag_x;
	int drag_y;
	DragInfo *draginfo;
	ULONG old_idcmp;
	ULONG old_winflags;
	TimerHandle *timer;
	int tick_count;
	int last_tick;
	BOOL busy;

	short use_iconedit;
	char edit_command[256];

	BOOL modified;

	char tempname[256];
} icon_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

void icon_free(icon_data *data);
void closewindow(icon_data *data);
short icon_info(icon_data *data, char *name, struct Node *);
void icon_draw_icon(icon_data *data);
void icon_fix_toolgads(icon_data *data, Att_Node *sel);
BOOL icon_save(icon_data *data, char *save_name, BOOL);
void image_to_bitmap(icon_data *, struct Image *image, struct BitMap *bitmap, short depth);

extern ConfigWindow *icon_windows[];
extern ObjectDef icon_info_objects[], icon_info_next_object[], icon_file_info_objects[], icon_tool_objects[],
	icon_project_objects[], icon_toolproj_objects[], icon_disk_objects[];

enum {
	GAD_BASE,

	GAD_ICON_LAYOUT_AREA,
	GAD_ICON_IMAGE,
	GAD_ICON_SAVE,
	GAD_ICON_NEXT,
	GAD_ICON_CANCEL,

	GAD_ICON_BLOCKS,
	GAD_ICON_BYTES,
	GAD_ICON_STACK,
	GAD_ICON_PROTECTION,
	GAD_ICON_LAST_CHANGED,
	GAD_ICON_COMMENT,
	GAD_ICON_TOOLTYPES,
	GAD_ICON_TOOLTYPES_EDIT,
	GAD_ICON_TOOLTYPES_NEW,
	GAD_ICON_TOOLTYPES_DELETE,
	GAD_ICON_TOOLTYPES_SORT,
	GAD_ICON_DEFAULT_TOOL_GLASS,
	GAD_ICON_DEFAULT_TOOL,
	GAD_ICON_SIZE,
	GAD_ICON_USED,
	GAD_ICON_FREE,
	GAD_ICON_FILE_SYSTEM,
	GAD_ICON_STATUS,

	MENU_BORDERLESS,
	MENU_NO_LABEL,
	MENU_STRIP_NEWICONS,

	GAD_ICON_LOCATION,

	MENU_STRIP_OLD,
	MENU_NEWICONS,
	MENU_OLDICONS,

	MENU_ICON_EDIT,
	MENU_ICON_PICK_EDITOR,
	MENU_DISK,
	MENU_DRAWER,
	MENU_TOOL,
	MENU_PROJECT,
	MENU_GARBAGE,

	MENU_COPY_OLDICON,
	MENU_COPY_NEWICON,
	MENU_COPY_BOTHICONS,
	MENU_COPY_TOOLTYPES,
};

extern short protect_lookup[];
extern unsigned long filesystem_table[];

char *strstri(char *string, char *substring);

extern MenuData icon_menus[];

extern short icon_type_labels[];

BOOL icon_switch_image(icon_data *data, short new);

extern struct Image oldicon_image;

Att_Node *tooltype_newnode(Att_List *list, char *name, ULONG data, ULONG flags);

/* Ambient icons */

#if defined(__MORPHOS__)
	/*
	 * This is from Ambient source code.
	 */
	#define OWN_MAGIC 0x466f4164

struct OwnDiskObject
{
	struct DiskObject diskobj;
	struct FreeList *fl; /* this is safe for WB 3.1 */
	/* start of additions */
	ULONG ownmagic;
	APTR ownptr;
	APTR png_context;
	ULONG ttnum;
	ULONG ttcur;
	STRPTR path;
	APTR glowchunk;
	ULONG glowsize;
	struct BitMap *pngimage;
	ULONG pngimage_width;
	ULONG pngimage_height;
	struct BitMap *pngimage2;
};

	#define ISOWN(x) (((struct OwnDiskObject *)x)->ownmagic == OWN_MAGIC && ((struct OwnDiskObject *)x)->ownptr == x)
#endif
