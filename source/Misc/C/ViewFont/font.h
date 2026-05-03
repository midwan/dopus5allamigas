/*

Directory Opus 5
Original APL release version 5.82
Copyright 1993-2012 Jonathan Potter & GP Software
Copyright 2012-2013 DOPUS5 Open Source Team
Copyright 2023-2026 Dimitris Panokostas (dopus5allamigas fork)

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
#define __ARGS(x) x

#include "font.strings"

#include <proto/dopus5.h>
#ifdef __AROS__
	#include <diskfont/diskfont.h>
#endif
#include <dopus/common.h>

enum { ARG_FONT, ARG_SIZE, ARG_BOLD, ARG_ITALIC, ARG_ULINE, ARG_SCREEN, ARG_COUNT };

typedef struct
{
	struct Screen *screen;
	struct Window *window;
	struct DOpusLocale locale;

	ConfigWindow win_dims;
	NewConfigWindow new_win;
	ObjectList *list;

	struct TextAttr attr;
	struct TextFont *font;

	char font_name[256];
	short font_size;
	short font_style;

	char title[80];

	IPTR arg_array[ARG_COUNT];

	char font_text[256];

	struct MsgPort *appport;
	struct AppWindow *appwindow;

	struct RDArgs *args;
	BOOL resized;
	BOOL first;

	struct Hook refresh_hook;

	char last_font_name[256];
	char **size_labels;

	struct Window *about;

	BPTR lock;
} font_data;

BOOL font_open(font_data *data);
void font_close(font_data *data);
void font_free(font_data *data);
void font_get_font(font_data *data);
void font_show_font(font_data *data, BOOL refresh);
void font_get_name(font_data *data, char *name);
void font_ask_name(font_data *data);

extern const ConfigWindow font_window;
extern const ObjectDef font_objects[];

enum {
	GAD_FONT_LAYOUT,

	GAD_FONT_FONT,
	GAD_FONT_SIZE,
	GAD_FONT_PRINT,
	GAD_FONT_UP,
	GAD_FONT_DOWN,
	GAD_FONT_BOLD,
	GAD_FONT_ITALIC,
	GAD_FONT_ULINE,
	GAD_FONT_DISPLAY,
	GAD_FONT_FONT_POPUP,
	GAD_FONT_CYCLE,

	MENU_OPEN_FONT,
	MENU_SAVE_SETTINGS,
	MENU_QUIT,
	MENU_ABOUT,
};

extern const MenuData font_menus[];

void init_locale_data(struct DOpusLocale *locale);
ULONG ASM font_refresh(REG(a0, struct Hook *hook), REG(a1, struct IntuiMessage *msg));
short font_get_size(font_data *data, short dir);
void font_build_labels(font_data *data, struct FontContentsHeader *fch);
void font_show_about(font_data *data);
void font_save_settings(font_data *data);
void font_read_settings(font_data *data);
void read_parse_set(char **ptr, unsigned short *val);

// void lsprintf __ARGS((char *,...));
void VARARGS68K lsprintf __ARGS((char *, char *, ...));
int font_openlibs(void);
