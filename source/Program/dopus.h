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

#ifndef DOPUS_INCLUDE
#define DOPUS_INCLUDE

// Moved to Include/dopus/stack.h
//#define STACK_DEFAULT	6144
//#define STACK_LARGE	8192

#include <proto/dopus5.h>
#include <proto/configopus.h>

#include <dopus/common.h>
#include "main_commands.h"

// Locale
#define CATCOMP_NUMBERS
#include "string_data.h"

#define APPWINID 1

#define VALID_QUALIFIERS                                                                                           \
	(IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND | IEQUALIFIER_CONTROL | IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT | \
	 IEQUALIFIER_LALT | IEQUALIFIER_RALT)

#define IEQUAL_ANYSHIFT (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)
#define IEQUAL_ANYALT (IEQUALIFIER_LALT | IEQUALIFIER_RALT)
#define IEQUAL_ANYAMIGA (IEQUALIFIER_LCOMMAND | IEQUALIFIER_RCOMMAND)

#define KEY_CURSORUP 0x3e
#define KEY_CURSORDOWN 0x1e
#define PAGEUP 0x3f
#define PAGEDOWN 0x1f
#define HOME 0x3d
#define END 0x1d

#define isonlyword(c) (!c || c == 10 || c == 13 || isspace(c) || ispunct(c))

#define NUMFONTS 16	 // Number of fonts

enum {
	MAIN_MENU_EVENT,  // Menu event from a sub-process
};

#define REQ_OFF(save)                       \
	{                                       \
		save = main_proc->pr_WindowPtr;     \
		main_proc->pr_WindowPtr = (APTR)-1; \
	}
#define REQ_ON(save)                    \
	{                                   \
		main_proc->pr_WindowPtr = save; \
	}

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct _rego_data
{
	char serial_number[20];
	char name[40];
	char company[40];
	char address1[40];
	char address2[40];
	char address3[40];
	struct DateStamp install_date;
	struct DateStamp harddrive_date;
	short pirate;
	short pirate_count;
	long checksum;
} rego_data;

typedef struct
{
	IPTR id;
	struct MenuItem *menu;
	struct Window *window;
} MenuEvent;

typedef struct
{
	long offset;
	APTR function;
} PatchList;

typedef struct
{
	short type;
	char name[2];
} env_packet;

#if defined(__amigaos4__)
struct LocaleBase
{
	struct Library lb_LibNode;
	BOOL lb_SysPatches;
};
#endif /* __amigaos4__ */

#ifndef __amigaos3__
	#pragma pack()
#endif

// Include files
#include "function_select.h"
#include "pattern.h"
#include "toolbar.h"
#include "display.h"
#include "reselect.h"
#include "lister.h"
#include "buttons.h"
#include "dirlist.h"
#include "app_msg.h"
#include "function_launch.h"
#include "function_protos.h"
#include "function_data.h"
#include "config.h"
#include "environment.h"

#include "misc.h"
#include "rexx.h"
#include "requesters.h"
#include "search.h"
#include "filetypes.h"
#include "buffers.h"
#include "path_routines.h"
#include "dates.h"
#include "links.h"
#include "cx.h"
#include "wb.h"
#include "backdrop.h"
#include "popup.h"
#include "desktop.h"
#include "groups.h"
#include "help.h"
#include "scripts.h"
#include "start_menu.h"

#include "position.h"
#include "dopusdata.h"
#include "palette_routines.h"
#include "envoy.h"
#include "notify.h"
#include "graphics.h"
#include "file_select.h"
#include "window_activate.h"
#include "cleanup.h"
#include "icons.h"
#include "menu_data.h"
#include "tile.h"
#include "protos.h"

#include "callback.h"

#define WBArgDir(a) (!(a)->wa_Name || !*(a)->wa_Name)

#define ABS(x) (((x) > 0) ? (x) : (-x))

#define SNIFF_REXX 0
#define SNIFF_BOTH 1
#define SNIFF_USER 2

#define FILENAME_LEN 31

/***  an offset to correctly position the iconify and padlock
	  gadgets in the window titlebar
****/
#ifdef __amigaos4__
	#define TBGADGETOFFSET -10
#else
	#define TBGADGETOFFSET 0
#endif

/*********************************************************************/

#if defined(__MORPHOS__)
	#define Module_Expunge() LP0NR(12, Module_Expunge, , ModuleBase, 0, 0, 0, 0, 0, 0)
#elif defined(__amigaos4__)
	#define Module_Expunge() IModule->Expunge()
#elif defined(__AROS__)
	#define Module_Expunge() AROS_LC0(BPTR, Module_Expunge, struct Library *, MODULE_BASE_NAME, 3, /* s */)
#else
//#pragma libcall ModuleBase Module_Expunge 12 0
void Module_Expunge(void);
#endif

#endif /* DOPUS_INCLUDE */
