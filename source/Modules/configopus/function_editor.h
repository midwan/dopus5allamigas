#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct Window *window;
	Cfg_Function *function;
	APTR memory;
	IPCData *owner_ipc;
	struct _FuncEdData *data;

	/*struct Library		*dopus_base;
	struct Library		*dos_base;
	struct Library		*int_base;
	struct Library		*util_base;
	struct Library		*cx_base;
	struct Library		*wb_base;
	struct Library		*gfx_base;
	struct Library		*asl_base;*/
	ULONG pad1[8];

	struct DOpusLocale *locale;

	ConfigWindow *win_def;
	ObjectDef *obj_def;

	ConfigWindow *req_win_def;
	ObjectDef *req_obj_def;

	UWORD *func_labels;
	long *flag_list;
	ULONG func_list;
	UWORD *arg_list;

	APTR object;
	ULONG object_flags;

	char title[80];

	ULONG flags;

	IPCData *main_owner;

	// struct Library		*layers_base;
	ULONG pad2;

	ULONG a4;

	ULONG pad;
} FunctionStartup;

#define FUNCEDF_NO_KEY (1 << 0)
#define FUNCEDF_LABEL (1 << 1)
#define FUNCEDF_CENTER (1 << 2)
#define FUNCEDF_SAVE (1 << 3)
#define FUNCEDF_IMAGE (1 << 4)

typedef struct
{
	APTR object;
	ULONG object_flags;
	Cfg_Function *function;
} FunctionReturn;

typedef struct _FuncEdData
{
	struct Window *window;	// Window
	ObjectList *objlist;	// List of objects

	struct MsgPort *appport;	  // WB App port
	struct AppWindow *appwindow;  // WB App window
	struct AppIcon *appicon;	  // WB App icon

	Cfg_Function *function;		  // Function to be edited
	Att_Node *edit_node;		  // Line being edited
	Att_List *function_list;	  // Function list
	Att_List *func_display_list;  // Function display list
	Att_List *flag_list;		  // List of flags

	struct DOpusLocale *locale;
	FunctionStartup *startup;

	short last_type;

	CfgDragInfo drag;
	short drag_shift;

	char buffer[256];

	ObjectList *p_objlist;

	char label[80];
} FuncEdData;
#ifndef __amigaos3__
	#pragma pack()
#endif

extern ConfigWindow _function_editor_window, _function_editor_window_center, _function_editor_label_window,
	_function_ed_req_window;
extern ObjectDef _function_editor_label_objects[], _function_editor_image_objects[], _function_editor_normal_objects[],
	_function_editor_objects[], _function_editor_objects_use[], _function_editor_objects_save[],
	_function_ed_req_objects[];

#define FUNCTYPE_BUTTON 1
#define FUNCTYPE_MENU 2
#define FUNCTYPE_FILETYPE 3
#define FUNCTYPE_HOTKEY 4

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct _FunctionEntry
{
	Att_Node *node;	   // Points back to node
	int type;		   // Entry type
	char buffer[256];  // Entry buffer
} FunctionEntry;
#ifndef __amigaos3__
	#pragma pack()
#endif

extern long _funced_flaglist[];
extern UWORD _function_type_labels[];
extern UWORD _argument_list[];

#define END_DELETE 1
#define END_DISABLE 2

#define FB_OUTPUTWIN 0
#define FB_OUTPUTFILE 1
#define FB_WB2F 2
#define FB_DOPUS2F 3
#define FB_RUNASYNC 4
#define FB_CDSOURCE 5
#define FB_CDDEST 6
#define FB_DOALLFILES 7
#define FB_RECURSEDIR 8
#define FB_RELOADFILE 9
#define FB_AUTOICONIFY 10
#define FB_NOQUOTE 11
#define FB_RESCANSOURCE 12
#define FB_RESCANDEST 13
#define FB_SHELLSTARTUP 14
#define FB_DOPUSSTARTUP 15

#define FF_OUTPUTWIN (1 << FB_OUTPUTWIN)
#define FF_OUTPUTFILE (1 << FB_OUTPUTFILE)
#define FF_WB2F (1 << FB_WB2F)
#define FF_DOPUS2F (1 << FB_DOPUS2F)
#define FF_RUNASYNC (1 << FB_RUNASYNC)
#define FF_CDSOURCE (1 << FB_CDSOURCE)
#define FF_CDDEST (1 << FB_CDDEST)
#define FF_DOALLFILES (1 << FB_DOALLFILES)
#define FF_RECURSEDIR (1 << FB_RECURSEDIR)
#define FF_RELOADFILE (1 << FB_RELOADFILE)
#define FF_AUTOICONIFY (1 << FB_AUTOICONIFY)
#define FF_NOQUOTE (1 << FB_NOQUOTE)
#define FF_RESCANSOURCE (1 << FB_RESCANSOURCE)
#define FF_RESCANDEST (1 << FB_RESCANDEST)
#define FF_SHELLSTARTUP (1 << FB_SHELLSTARTUP)
#define FF_DOPUSSTARTUP (1 << FB_DOPUSSTARTUP)

#define FF_ALLFLAGS (~0)
#define FF_HOTKEYFLAGS (FF_DOPUSSTARTUP | FF_SHELLSTARTUP | FF_NOQUOTE | FF_OUTPUTWIN | FUNCF_WAIT_CLOSE)

enum {
	GAD_FUNCEDREQ_LISTER,
	GAD_FUNCEDREQ_OKAY,
	GAD_FUNCEDREQ_CANCEL,
};

extern MenuData _funced_menus[];

// prototypes
void FunctionEditor(void);
void funced_cleanup(FuncEdData *);
void funced_build_flaglist(FuncEdData *);
void funced_update_flaglist(FuncEdData *);
void funced_decompile(FuncEdData *);
void funced_compile(FuncEdData *);
void funced_build_display(FuncEdData *);
void funced_build_entrydisplay(FuncEdData *, Att_Node *, FunctionEntry *);
void funced_start_edit(FuncEdData *);
BOOL funced_end_edit(FuncEdData *, Att_Node *, int, UWORD);
Att_Node *funced_new_entry(FuncEdData *, Att_Node *, FunctionEntry *);
void funced_check_flag(ULONG *, ULONG);
void funced_edit_insertstring(ObjectList *, ULONG, char *, struct Library *, struct Library *);
void funced_store_edits(FuncEdData *);
void funced_init_gads(FuncEdData *);
BOOL funced_command_req(FuncEdData *, char *, short);
void funced_appmsg(FuncEdData *, struct AppMessage *);
short funced_appmsg_arg(struct WBArg *, char *, struct Library *);
void functioned_end_drag(FuncEdData *, short);
void functioned_copy_line(FuncEdData *, FunctionEntry *, unsigned short, Point *);
short functioned_get_line(struct Window *, GL_Object *, unsigned short, unsigned short, struct Library *);
void funced_show_key(FuncEdData *data);
BOOL funced_rexx_args(char *name, char *buffer);
