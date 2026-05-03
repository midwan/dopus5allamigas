#define CONFIG_MAGIC 0xFACE

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

#ifndef __amigaos3__
	#pragma pack(2)
#endif

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
	APTR memory;
} ConfigStuff;
#ifndef __amigaos3__
	#pragma pack()
#endif

#define MODE_WORKBENCHUSE 1
#define MODE_WORKBENCHCLONE 2

#define SCRFLAGS_HALFHEIGHT 4

/* Flags relating to configuration options */

#ifndef DOPUS_CONFIGFLAGS
	#define DOPUS_CONFIGFLAGS

/* ErrorFlags */

	#define ERROR_ENABLE_DOS 1
	#define ERROR_ENABLE_OPUS 2

/* GeneralFlags */

	#define GENERAL_DISPLAYINFO 2
	#define GENERAL_DOUBLECLICK 8
	#define GENERAL_ACTIVATE 64
	#define GENERAL_DRAG 128

/* SortFlags */

	#define SORT_LREVERSE 1
	#define SORT_RREVERSE 2

/* UpdateFlags */

	#define UPDATE_FREE 1
	#define UPDATE_SCROLL 2
	#define UPDATE_REDRAW 4
	#define UPDATE_NOTIFY 8
	#define UPDATE_LEFTJUSTIFY 16
	#define UPDATE_PROGRESSINDICATOR 32

/* ScreenFlags */

	#define SCRFLAGS_HALFHEIGHT 4

/* DirFlags */

	#define DIRFLAGS_EXALL 2
	#define DIRFLAGS_AUTODISKC 4
	#define DIRFLAGS_AUTODISKL 8
	#define DIRFLAGS_FINDEMPTY (1 << 0)
	#define DIRFLAGS_CHECKBUFS (1 << 4)
	#define DIRFLAGS_REREADOLD (1 << 5)
	#define DIRFLAGS_SMARTPARENT (1 << 6)

/* ViewBits */

	#define VIEWBITS_SHOWBLACK 1
	#define VIEWBITS_FILTEROFF 2
	#define VIEWBITS_8BITSPERGUN 4
	#define VIEWBITS_PLAYLOOP 8
	#define VIEWBITS_TEXTBORDERS 16
	#define VIEWBITS_PAUSEANIMS 32

/* IconType */

	#define ICON_MEMORY 1
	#define ICON_CPU 2
	#define ICON_DATE 4
	#define ICON_TIME 8
	#define ICON_NOWINDOW 16
	#define ICON_APPICON 32
	#define ICON_BYTES 64
	#define ICON_C_AND_F 128

/* ScrClkType */

	#define SCRCLOCK_MEMORY 1
	#define SCRCLOCK_CPU 2
	#define SCRCLOCK_DATE 4
	#define SCRCLOCK_TIME 8
	#define SCRCLOCK_BYTES 64
	#define SCRCLOCK_C_AND_F 128

/* ShowFree */

/* AddIconFlags */

	#define ADDICONFLAGS_USEDEFAULTICONS 1

/* GeneralScreenFlags */

	#define SCR_GENERAL_WINBORDERS 1
	#define SCR_GENERAL_TINYGADS 2
	#define SCR_GENERAL_GADSLIDERS 4
	#define SCR_GENERAL_INDICATERMB 8
	#define SCR_GENERAL_NEWLOOKPROP 16
	#define SCR_GENERAL_REQDRAG 32
	#define SCR_GENERAL_NEWLOOKMENU 64
	#define SCR_GENERAL_TITLESTATUS 128

/* LoadExternal */

	#define LOAD_DISK 1
	#define LOAD_PRINT 2
	#define LOAD_ICON 4
	#define LOAD_CONFIG 65536

/* HotkeyFlags */

	#define HOTKEY_USEMMB 1

#endif

int ReadOldConfig(char *name, ConfigStuff *cstuff);
void FreeOldConfig(ConfigStuff *cstuff);
