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
#include "join.strings"
#include "module_deps.h"

#define COPY_INITIAL_BUFFER 8192
#define COPY_MIN_BUFFER 2048
#define COPY_LOW_THRESH 500000
#define COPY_HIGH_THRESH 1000000

// Using QUAD args with OS3/AROS Seek() & SetFileSize() can corrupt files.
// Do not change these defintions unless AROS becomes 64bit aware.
#undef DO_64BIT
#ifdef USE_64BIT
	#if defined(__amigaos4__) || defined(__MORPHOS__)
		#define DO_64BIT
	#endif
#endif

enum { JOIN, SPLIT };

enum { JOINARG_FROM, JOINARG_TO };

enum {
	SPLITARG_FROM,
	SPLITARG_TO,
	SPLITARG_CHUNK,
	SPLITARG_STEM,
};

enum {
	GAD_JOIN_LAYOUT,
	GAD_JOIN_LISTER,
	GAD_JOIN_ADD,
	GAD_JOIN_REMOVE,
	GAD_JOIN_CLEAR,
	GAD_JOIN_MOVE_UP,
	GAD_JOIN_MOVE_DOWN,
	GAD_JOIN_OK,
	GAD_JOIN_CANCEL,
	GAD_JOIN_TO_POPUP,
	GAD_JOIN_TO_FIELD,

	GAD_SPLIT_FROM_POPUP,
	GAD_SPLIT_FROM,
	GAD_SPLIT_TO_POPUP,
	GAD_SPLIT_TO,
	GAD_SPLIT_STEM,
	GAD_SPLIT_INTO,
	GAD_SPLIT_SKIP,
	GAD_SPLIT_OK,
	GAD_SPLIT_SIZES,
};

extern ConfigWindow join_window, split_window;
extern ObjectDef join_objects[], split_objects[];

/*#ifdef __amigaos4__
#define JPTR struct TimerIFace *
#else
#define JPTR APTR
#endif*/

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct Screen *screen;
	struct Window *window;
	IPCData *ipc;

	ConfigWindow win_dims;
	NewConfigWindow new_win;
	ObjectList *list;

	struct MsgPort *app_port;
	struct AppWindow *app_window;

	Att_List *join_list;

	char source[256];
	char dest[256];

	struct timerequest timer_req;
	/*struct Library		*TimerBase;
	JPTR				ITimer;*/
	ULONG pad1;

	short function;

	char buf[256];

	FuncArgs *args;
} join_data;

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

BOOL join_open(join_data *);
void join_close(join_data *);
void join_free(join_data *);
void join_add_file(join_data *, char *, short);
void join_do_gadgets(join_data *, short);
void join_remove(join_data *);
void join_clear(join_data *);
void join_move(join_data *, short);
void join_add(join_data *);
short join_check_filename(join_data *);
BOOL join_do_join(join_data *);
BOOL join_join_files(join_data *);
short join_show_error(join_data *, short, char *, BOOL);
void split_do_gadgets(join_data *data, Att_Node *node);
short split_do_split(join_data *data);
short split_split_file(join_data *data);

extern char *func_templates[];
