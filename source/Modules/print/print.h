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
#include "print.strings"
#include "module_deps.h"

#ifndef __amigaos3__
	#pragma pack(2)
#endif
typedef struct
{
	struct FileInfoBlock fib;

	struct Window *window;
	struct Screen *screen;
	struct DrawInfo *drawinfo;

	IPCData *ipc;
	struct List *files;

	NewConfigWindow newwin;
	ObjectList *objlist;

	struct Preferences prefs;

	char print_quality;
	char print_spacing;
	char print_pitch;
	short left_margin;
	short right_margin;
	short page_length;
	short tab_size;
	char header_flags[2];
	char header_title[2][40];
	char header_style[2];
	short output_type;
	char output_name[256];

	char buffer[8192];
	char line_buffer[1024];
	char wrap_buffer[1024];
	char header_buffer[1024];

	struct DateTime file_date;
	char date_buf[16];
	char time_buf[16];

	short line_width;
	short page;

	struct MsgPort *printer_port;
	union printerIO *printer_io;

	IPCData *main_ipc;

	APTR output_file;

	APTR progress;
	struct Window *prog_win;
	short abort_bit;

	struct FileRequester *filereq;
} print_data;
#ifndef __amigaos3__
	#pragma pack()
#endif

#define DATA_SIZE (offsetof(print_data, progress) - offsetof(print_data, print_quality))

extern ConfigWindow print_window, print_status_window;
extern ObjectDef print_objects[], print_status_objects[];

extern char *print_styles[];

void print_free(print_data *);
void print_fix_header(print_data *data);
void print_get_header(print_data *data, short h);
BOOL print_open(print_data *data);
void print_print(print_data *data);
BOOL print_check_abort(print_data *data);
BOOL print_print_file(print_data *data, BPTR file);
BOOL print_print_string(print_data *data, char *buf);
BOOL print_header_footer(print_data *data, short which);

enum {
	GAD_PRINT_LAYOUT,
	GAD_PRINT_OKAY,
	GAD_PRINT_CANCEL,
	GAD_PRINT_TEXT_LAYOUT,
	GAD_PRINT_TEXT_QUALITY,
	GAD_PRINT_TEXT_SPACING,
	GAD_PRINT_TEXT_PITCH,
	GAD_PRINT_LEFT_MARGIN,
	GAD_PRINT_RIGHT_MARGIN,
	GAD_PRINT_PAGE_LENGTH,
	GAD_PRINT_TAB_SIZE,
	GAD_PRINT_HEADER_LAYOUT,
	GAD_PRINT_HEADER_FOOTER,
	GAD_PRINT_TITLE,
	GAD_PRINT_TITLE_STRING,
	GAD_PRINT_DATE,
	GAD_PRINT_PAGE,
	GAD_PRINT_STYLE,
	GAD_PRINT_FILENAME,
	GAD_PRINT_STATUS,
	GAD_PRINT_ABORT,
	GAD_PRINT_OUTPUT,
};

enum { PRINT_QUALITY_DRAFT, PRINT_QUALITY_LETTER };

enum { PRINT_SPACING_6, PRINT_SPACING_8 };

enum { PRINT_PITCH_PICA, PRINT_PITCH_ELITE, PRINT_PITCH_FINE };

enum { PRINT_HEADER, PRINT_FOOTER };

#define PRINT_HEADERF_TITLE (1 << 0)
#define PRINT_HEADERF_DATE (1 << 1)
#define PRINT_HEADERF_PAGE (1 << 2)

enum { PRINT_STYLE_NORMAL, PRINT_STYLE_BOLD, PRINT_STYLE_ITALICS, PRINT_STYLE_UNDERLINE };
