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
#include "play.strings"
#include "module_deps.h"
#include "music.h"
#include <proto/music.h>
#include <devices/audio.h>

#define ENV_PLAY "dopus/Player"

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	IPCData *ipc;
	char *file;
	BOOL multiple;

	struct Window *window;
	ObjectList *objlist;

	NewConfigWindow new_win;

	unsigned long clock;

	APTR dt_object;
	BYTE dt_signal;
	// struct Library      *dt_base;

	// struct Library      *music_base;
	ULONG pad1[2];
	short module_type;

	char *type_string;

	struct AppIcon *app_icon;
	struct MsgPort *app_port;
	struct DiskObject *icon;

	char play_name[40];
	char play_length[40];
	char play_type[40];
	char play_status[40];

	struct Screen *screen;

	ConfigWindow dims;
	struct IBox pos;
	long icon_x;
	long icon_y;
	char buf[50];

	ULONG flags;
} play_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

#define PLAYF_QUIET (1 << 0)
#define PLAYF_ICON (1 << 1)

//#define DataTypesBase   (data->dt_base) Don't redefine-opened in libinit.c
//#define MUSICBase   (data->music_base)

#define PLAY_DOS_ERROR 0
#define PLAY_OK 1
#define PLAY_ABORT -1
#define PLAY_MEMORY_ERROR -2
#define PLAY_INIT_ERROR -3

void play_free(play_data *);
BOOL play_init(play_data *);
BOOL play_file(play_data *);
void play_cleanup(play_data *);
void unpack_delta(char *, unsigned long, char *, unsigned char);
void play_update_length(play_data *);
BOOL wait_for_play(play_data *data, BOOL dt);
void play_iconify(play_data *data);
void play_open_window(play_data *data, struct Screen *screen, struct Window *parent);
void play_update_info(play_data *data, const char *, const char *, const char *, const char *);
void play_reveal(play_data *data, struct Screen *scr);
void read_parse_set(char **ptr, long *val);
void play_snapshot(play_data *data, struct AppSnapshotMsg *msg);

extern ConfigWindow play_window;
extern ObjectDef play_objects[];
extern ObjectDef play_objects_single[];

enum {
	GAD_BASE,

	GAD_PLAY_LAYOUT,
	GAD_PLAY_NEXT,
	GAD_PLAY_ABORT,
	GAD_PLAY_FILENAME,
	GAD_PLAY_LENGTH,
	GAD_PLAY_TYPE,
	GAD_PLAY_STATUS,
};
