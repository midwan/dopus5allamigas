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
#ifndef _CONFIG_H
#define _CONFIG_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

#define CONFIG_MAGIC 0xFACE

#define OLD_CONFIG_VERSION 9999
#define CONFIG_LESS_DODRIVES 10001
#define CONFIG_OLD_FILETYPE 10002
#define CONFIG_CHANGE_DOSREQ 10003
#define CONFIG_CHANGE_BUFCOUNT 10004
#define CONFIG_CHANGE_FILETYPE 10009
#define CONFIG_NEW_FUNCTIONS 10010
#define CONFIG_CHANGE_PALETTE 10012
#define CONFIG_CHANGE_ARROWS 10013
#define CONFIG_CHANGE_FTYPE 10014
#define CONFIG_CHANGE_EXTERN 10015

#define NEW_CONFIG_VERSION 10016
#define CONFIG_CHANGE_DISPLAY 10017
#define CONFIG_CHANGE_CONFIGXY 10018
#define CONFIG_CHANGE_DIMENS 10019
#define CONFIG_NEW_SLIDERS 10020

#define DATE_DOS 1
#define DATE_INT 2
#define DATE_USA 4
#define DATE_AUS 8

#define FTYC_MATCH 1
#define FTYC_MATCHNAME 2
#define FTYC_MATCHBITS 3
#define FTYC_MATCHCOMMENT 4
#define FTYC_MATCHSIZE 5
#define FTYC_MATCHDATE 6
#define FTYC_MOVETO 7
#define FTYC_MOVE 8
#define FTYC_SEARCHFOR 9
#define FTYC_OR 253
#define FTYC_AND 254
#define FTYC_ENDSECTION 255

#define SCRCLOCK_MEMORY 1
#define SCRCLOCK_CPU 2
#define SCRCLOCK_DATE 4
#define SCRCLOCK_TIME 8
#define SCRCLOCK_BYTES 64
#define SCRCLOCK_C_AND_F 128

#define ICON_MEMORY 1
#define ICON_CPU 2
#define ICON_DATE 4
#define ICON_TIME 8
#define ICON_NOWINDOW 16
#define ICON_APPICON 32

#define DISPLAY_NAME 0
#define DISPLAY_SIZE 1
#define DISPLAY_PROTECT 2
#define DISPLAY_DATE 3
#define DISPLAY_COMMENT 4
#define DISPLAY_FILETYPE 5

#define FLAG_OUTWIND 1
#define FLAG_OUTFILE 2
#define FLAG_WB2F 4
#define FLAG_DOPUSF 8
#define FLAG_ASYNC 16
#define FLAG_CDSOURCE 32
#define FLAG_CDDEST 64
#define FLAG_DOALL 128
#define FLAG_RECURSE 256
#define FLAG_RELOAD 512
#define FLAG_ICONIFY 1024
#define FLAG_NOQUOTE 2048
#define FLAG_SCANSRCE 4096
#define FLAG_SCANDEST 8192
#define FLAG_SHELLUP 16384
#define FLAG_DOPUSUP 32768

#define MENUCOUNT 100
#define GADCOUNT 84
#define DRIVECOUNT 32
#define ARCHIVECOUNT 6
#define NUMFONTS 16

#define OLDDRIVECOUNT 24

#define FILETYPE_FUNCNUM 16

#define FTFUNC_AUTOFUNC1 0
#define FTFUNC_AUTOFUNC2 1
#define FTFUNC_DOUBLECLICK 2
#define FTFUNC_CLICKMCLICK 3
#define FTFUNC_ANSIREAD 4
#define FTFUNC_AUTOFUNC3 5
#define FTFUNC_HEXREAD 6
#define FTFUNC_LOOPPLAY 7
#define FTFUNC_PLAY 8
#define FTFUNC_AUTOFUNC4 9
#define FTFUNC_READ 10
#define FTFUNC_SHOW 11

struct olddopusgadget
{
	char name[10];
	int which, stack;
	unsigned char key, qual;
	char type, pri, delay;
	char fpen, bpen;
	char pad[3];
	char *function;
};

struct dopusfunction
{
	char name[16];
	int which, stack;
	unsigned char key, qual;
	char type, pri, delay;
	char fpen, bpen;
	char pad;
	char *function;
};

typedef struct newdopusfunction
{
	char *name;
	int pad2[3];
	int which, stack;
	unsigned char key, qual;
	char type, pri, delay;
	char fpen, bpen;
	char pad;
	char *function;
} old_NewFunction;

struct dopusdrive
{
	char name[10];
	char path[33];
	unsigned char key, qual;
	char fpen, bpen;
	char pad;
};

struct olddopusfiletype
{
	struct dopusfiletype *next;
	char type[40];
	char filepat[60];
	char recogchars[100];
	char actionstring[4][60];
	int which[4], stack[4];
	char pri[4], delay[4];
	char and;
	char pad[3];
	char *function[4];
};

typedef struct dopusfiletype
{
	struct dopusfiletype *next;
	char type[32];
	char typeid[8];
	char actionstring[FILETYPE_FUNCNUM][40];
	int which[FILETYPE_FUNCNUM], stack[FILETYPE_FUNCNUM];
	char pri[FILETYPE_FUNCNUM], delay[FILETYPE_FUNCNUM];
	unsigned char *recognition;
	char *function[FILETYPE_FUNCNUM];
	char *iconpath;
} Filetype;

struct wr_dopusfiletype
{
	struct dopusfiletype *next;
	char type[40];
	char actionstring[FILETYPE_FUNCNUM][40];
	int which[FILETYPE_FUNCNUM], stack[FILETYPE_FUNCNUM];
	char pri[FILETYPE_FUNCNUM], delay[FILETYPE_FUNCNUM];
};

typedef struct dopusgadgetbanks
{
	struct newdopusfunction gadgets[GADCOUNT];
	struct dopusgadgetbanks *next;
	struct dopusgadgetbanks *prev;
} GadgetBank;

typedef struct dopushotkey
{
	struct dopushotkey *next;
	UWORD code, qualifier;
	char name[40];
	struct dopusfunction func;
} Hotkey;

struct Config
{
	UWORD version;
	UWORD magic;

	char copyflags;
	char deleteflags;
	char errorflags;
	unsigned char generalflags;
	char iconflags;
	char existflags;
	char sepflags;
	char sortflags;
	char dynamicflags;
	char sortmethod[2];

	char hotkeyflags;

	char menutit[5][16];
	struct newdopusfunction menu[MENUCOUNT];

	struct dopusfunction drive[DRIVECOUNT];

	char outputcmd[80], output[80];
	int gadgetrows;

	char separatemethod[2];

	char language[30];

	char displaypos[2][16];
	char displaylength[2][16];

	char pubscreen_name[80];

	UWORD Palette[16];
	char gadgettopcol, gadgetbotcol;
	char statusfg, statusbg;
	char filesfg, filesbg, filesselfg, filesselbg;
	char dirsfg, dirsbg, dirsselfg, dirsselbg;
	char clockfg, clockbg;
	char requestfg, requestbg;
	char disknamefg, disknamebg, disknameselfg, disknameselbg;
	char slidercol, arrowfg, arrowbg, littlegadfg, littlegadbg;

	char pad3;

	char scrdepth;
	char screenflags;
	int screenmode;
	int scrw, scrh;
	char fontbuf[40];
	char arrowpos[3];

	char pad4;

	char startupscript[80];
	char dirflags;
	unsigned char bufcount;

	char pad5[2];

	char autodirs[2][30];
	char pad5a[80];
	UWORD hotkeycode, hotkeyqual;

	char toolicon[80], projecticon[80], drawericon[80], defaulttool[80];
	char priority;
	char showdelay, viewbits, fadetime, tabsize;

	char pad7[2];

	char hiddenbit;
	char showpat[40], hidepat[40];
	char showpatparsed[40], hidepatparsed[40];
	char icontype, scrclktype, showfree;

	char pad8;

	short iconx, icony;
	short wbwinx, wbwiny;

	char configreturnscript[80];

	char fontsizes[NUMFONTS];
	char fontbufs[NUMFONTS][40];

	char uniconscript[80];
	char sliderbgcol;

	char pad_foo;

	short scr_winx, scr_winy;
	short scr_winw, scr_winh;

	char morepadding[231];

	char old_displaypos[2][8];
	char dateformat, addiconflags;
	char stringfgcol, stringbgcol;
	char namelength[2];
	char sliderwidth, sliderheight;
	char formatflags;
	short iconbutx, iconbuty;
	char stringheight;
	char stringselfgcol, stringselbgcol;
	char generalscreenflags;

	struct Rectangle scrollborders[2];

	char old_displaylength[2][8];

	char shellstartup[30];

	char windowdelta;

	char pad9a[397];

	int loadexternal;

	ULONG new_palette[48];

	char arrowsize[3];

	char slider_pos;

	short config_x;
	short config_y;

	char pad10[1414];
};

typedef struct _ConfigStuff
{
	struct Config *config;	// Configuration structure
	Filetype *firsttype;	// First filetype
	GadgetBank *firstbank;	// First gadget bank
	Hotkey *firsthotkey;	// First hotkey
} ConfigStuff;

#define MODE_WORKBENCHUSE 1
#define MODE_WORKBENCHCLONE 2

#define SCRFLAGS_HALFHEIGHT 4

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
