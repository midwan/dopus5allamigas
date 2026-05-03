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
#include "themes.strings"
#include "module_deps.h"

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

#define SAVETHEME 0
#define LOADTHEME 1
#define BUILDTHEME 2
#define CONVERTTHEME 3

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define is_space(c) ((c) == ' ' || (c) == '\t')

extern char *func_templates[];

long save_theme(struct Screen *, DOpusCallbackInfo *, char *, BOOL);
void rexx_skip_space(char **);
short rexx_parse_word(char **, char *, short);
BOOL save_theme_background(APTR, DOpusCallbackInfo *, char *, struct MsgPort *, char *, APTR);
BOOL save_theme_sound(APTR, DOpusCallbackInfo *, char *, struct MsgPort *, char *, APTR);
BOOL save_theme_font(APTR, DOpusCallbackInfo *, char *, struct MsgPort *);
BOOL save_theme_pens(APTR, DOpusCallbackInfo *, char *, struct MsgPort *);
BOOL save_theme_palette(APTR, DOpusCallbackInfo *, struct MsgPort *);
BOOL theme_copy_file(char *, char *);
Att_List *theme_build_list(char *path);
void write_theme_intro(APTR, char *);
void write_theme_outro(APTR);
short convert_theme(DOpusCallbackInfo *, char *, char *);
void convert_theme_file(char *);

extern char *pen_settings[];

enum {
	APPLY_PALETTE,
	APPLY_FONTS,
	APPLY_PICTURES,
	APPLY_SOUNDS,

	APPLY_LAST,
};

extern char apply_lookup[], *sound_lookup[];
