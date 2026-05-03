#define CATCOMP_NUMBERS
#include "filetype.strings"

#include "module_deps.h"

/* mod_ids */
enum { MODID_FIND_FT, MODID_CREATE_FT };

#define FILENAME_MAXLEN (30)
#define FILETYPE_MAXLEN (31)
#define ID_MAXLEN (7)
#define PRI_MAXLEN (4)

/* How many bytes to look at at start of file */
#define BYTECOUNT 16

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct Screen *screen;
	IPCData *ipc;
	IPCData *main_ipc;
	EXT_FUNC(func_callback);

	NewConfigWindow new_win;
	struct Window *window;

	struct MsgPort *app_port;
	struct AppWindow *app_window;

	ObjectList *list;

	struct pointer_packet pointer_packet;
	Att_List *filetype_cache;

	Att_List *filetypes_list;
	Att_List *storage_list;

	Att_List *listview_list;

	char current_entry_path[256 + 1];

	int count_filetypes;
	int best_filetypes;
	int count_storage;
	int best_storage;

	Cfg_FiletypeList *filetype_list;
	Cfg_Filetype *best_installed_ft;
	Cfg_Filetype *best_stored_ft;
	Cfg_Filetype *edited_filetype;

	IPCData *editor_ipc;
	IPCData *creator_ipc;
	ULONG a4;

	char path[256];

} finder_data;

struct filetype_info
{
	char fti_path[256 + 1];
	char fti_filename[30 + 1];		 /* Null terminated. Max 30 chars used for now */
	LONG fti_iff_type;				 /* IFF FORM type */
	ULONG fti_datatype_group;		 /* Group that the DataType is in */
	ULONG fti_datatype_ID;			 /* ID for DataType (same as IFF FORM type) */
	UBYTE fti_filebytes[BYTECOUNT];	 /* First BYTECOUNT bytes of the file */
	UBYTE fti_bytemask[BYTECOUNT];	 /* Mask for matching bytes */
	int fti_bytecount;				 /* 0 to BYTECOUNT */
	UBYTE fti_filebytesc[BYTECOUNT]; /* First BYTECOUNT bytes of the file (case insensitive) */
	UBYTE fti_bytemaskc[BYTECOUNT];	 /* Mask for matching bytes (case insensitive) */
	int fti_bytecountc;				 /* 0 to BYTECOUNT (case insensitive) */
	ULONG fti_flags;				 /* See below */
};

typedef struct
{
	struct Screen *screen;
	IPCData *ipc;
	IPCData *main_ipc;
	IPCData *opus_ipc;
	EXT_FUNC(func_callback);

	APTR memh;

	NewConfigWindow new_win;
	struct Window *window;
	struct MsgPort *app_port;
	struct AppWindow *app_window;

	ObjectList *list;

	Att_List *file_list;

	char filetype_name[FILETYPE_MAXLEN + 1];

	int caseflag;

	struct filetype_info master_fti;
	char *match_name;
	char *match_group;
	char *match_id;
	char *match_iff;
	char *match_bytes;
	char *match_bytesc;

	Cfg_FiletypeList *filetype_list;
	Cfg_Filetype *filetype;

	int edited;

	struct FileRequester *filereq;
	char req_dir[256 + 1];

	IPCData *editor_ipc;
	ULONG a4;
} creator_data;

#ifndef __amigaos3__
	#pragma pack()
#endif

extern ConfigWindow _finder_window;
extern ObjectDef _finder_objects[];

extern ConfigWindow _creator_window;
extern ObjectDef _creator_objects[];

enum {
	GAD_FIND_LAYOUT,
	GAD_FIND_LISTVIEW,
	GAD_FIND_TEXT1,
	GAD_FIND_TEXT2,
	GAD_FIND_TEXT3,
	GAD_FIND_TEXT4,
	GAD_FIND_TEXT5,
	GAD_FIND_TEXT6,
	GAD_FIND_TEXT7,
	GAD_FIND_TEXT8,
	GAD_FIND_USE,
	GAD_FIND_INSTALL,
	GAD_FIND_CREATE,
	GAD_FIND_EDIT,
	GAD_FIND_CANCEL,

	GAD_CREATE_LAYOUT,
	GAD_CREATE_LISTVIEW,
	GAD_CREATE_ADD,
	GAD_CREATE_DELETE,
	GAD_CREATE_CLEAR,
	GAD_CREATE_FILETYPE,
	GAD_CREATE_NAME,
	GAD_CREATE_NAME_FIELD,
	GAD_CREATE_IFF,
	GAD_CREATE_IFF_FIELD,
	GAD_CREATE_GROUP,
	GAD_CREATE_GROUP_FIELD,
	GAD_CREATE_ID,
	GAD_CREATE_ID_FIELD,
	GAD_CREATE_BYTES,
	GAD_CREATE_BYTES_FIELD,
	GAD_CREATE_CYCLE,
	GAD_CREATE_EDIT,
	GAD_CREATE_SAVE,
	GAD_CREATE_CANCEL,
};

#define FTIF_USED (1 << 0)
#define FTIF_FILENAME (1 << 1)
#define FTIF_FILENAME_END (1 << 2)
#define FTIF_GROUP (1 << 3)
#define FTIF_ID (1 << 4)
#define FTIF_IFF (1 << 5)
#define FTIF_BYTES (1 << 6)
#define FTIF_BYTESC (1 << 7)

// Filetype matching commands
enum {
	FTOP_NOOP,			// No operation (0)
	FTOP_MATCH,			// Match text (1)
	FTOP_MATCHNAME,		// Match filename (2)
	FTOP_MATCHBITS,		// Match protection bits (3)
	FTOP_MATCHCOMMENT,	// Match comment (4)
	FTOP_MATCHSIZE,		// Match size (5)
	FTOP_MATCHDATE,		// Match date (6)
	FTOP_MOVETO,		// Move to absolute location (7)
	FTOP_MOVE,			// Move to relative location (8)
	FTOP_SEARCHFOR,		// Search for text (9)
	FTOP_MATCHFORM,		// Match an IFF FORM (10)
	FTOP_FINDCHUNK,		// Find an IFF chunk (11)
	FTOP_MATCHDTGROUP,	// Match datatypes group (12)
	FTOP_MATCHDTID,		// Match datatypes ID (13)
	FTOP_MATCHNOCASE,	// Match text case insensitive (14)
	FTOP_DIRECTORY,		// Match directory (15)

	FTOP_LAST,	// Last valid command

	FTOP_SPECIAL = 252,	 // Start of special instructions

	FTOP_OR,		 // Or (253)
	FTOP_AND,		 // And (254)
	FTOP_ENDSECTION	 // End of a section (255)
};

/* YUCK! should not be here */
#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct _FunctionEntry
{
	struct MinNode node;
	char *name;
	struct DirEntry *entry;
	short type;
	short flags;
} FunctionEntry;
#ifndef __amigaos3__
	#pragma pack()
#endif
