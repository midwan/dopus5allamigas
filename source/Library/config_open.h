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

/* Old structures for converting to newer Configuration */

#include <Program/dopus_config.h>

#ifndef __amigaos3__
	#pragma pack(2)
#endif

// Old BTNW - Button window
typedef struct
{
	char name[32];		 // Button window name
	struct IBox pos;	 // Window position
	char font_name[31];	 // Font to use
	UBYTE font_size;	 // Font size
	UWORD columns;		 // Number of columns
	UWORD rows;			 // Number of rows
	ULONG flags;		 // Flags
} OLD_CFG_BTNW;

// Old List format
typedef struct
{
	UBYTE files_unsel[2];  // Unselected files
	UBYTE files_sel[2];	   // Selected files
	UBYTE dirs_unsel[2];   // Unselected directories
	UBYTE dirs_sel[2];	   // Selected directories

	SortFormat sort;	   // Sort method
	BYTE display_pos[16];  // Item display position
	BYTE display_len[15];  // Display length

	UBYTE flags;

	BYTE show_free;			// Show free space type
	char show_pattern[40];	// Show pattern
	char hide_pattern[40];	// Hide pattern

	char show_pattern_p[40];  // Show pattern parsed
	char hide_pattern_p[40];  // Hide pattern parsed
} OldListFormat;

// Old LSTR structure
typedef struct
{
	struct IBox pos[2];	 // Window position
	ULONG icon_x;		 // Icon position
	ULONG icon_y;
	OldListFormat format;  // Lister format
	ULONG flags;		   // Flags
	ULONG pad[4];
	// Followed by path to load
} OLD_CFG_LSTR;

// Old Cfg_Lister structure
typedef struct
{
	struct Node node;
	OLD_CFG_LSTR lister;  // Lister definition
	char *path;			  // Path to load
} Old_Cfg_Lister;

// OLD ENVR - defines the previous screen and other display info
typedef struct
{
	ULONG screen_mode;			 // Screen mode
	UWORD screen_flags;			 // Screen flags
	UWORD screen_depth;			 // Screen depth
	UWORD screen_width;			 // Screen width
	UWORD screen_height;		 // Screen height
	ULONG palette[50];			 // Screen palette
	char pubscreen_name[80];	 // Public screen name to open on
	struct IBox window_pos;		 // Window dimensions
	ULONG general_screen_flags;	 // General screen flags

	UBYTE source_col[2];  // Source window colours
	UBYTE dest_col[2];	  // Destination window colours

	WORD palette_count;	 // Number of user colours
	UWORD pad1;

	char font_name[4][40];	// Fonts to use

	ULONG env_Colours[CUST_PENS][2][3];	 // Custom colours
	ULONG env_ColourFlag;				 // Which custom colours are in use

	ULONG env_NewIconsFlags;  // NewIcons flags

	ULONG pad3[2];

	char status_text[80];  // Status bar text

	UBYTE font_size[4];	 // Font sizes

	char pad4[12];

	UWORD display_options;	 // Display options
	UWORD main_window_type;	 // Main window type

	ULONG hotkey_flags;	 // Hotkey flags
	UWORD hotkey_code;	 // Hotkey code
	UWORD hotkey_qual;	 // Hotkey qualifier

	OldListFormat list_format;	// Default list format

	char backdrop_prefs[80];  // Backdrop pattern prefs file

	char desktop_location[240];	 // Desktop folder location

	char output_window[80];	 // Output window
	char output_device[80];	 // Output device

	unsigned long default_stack;  // Default stack size

	/*
		char		scr_title_text[120];	// Screen title text

		ULONG		pad6[17];
	*/

	char scr_title_text[188];  // Screen title text

	UWORD lister_options;  // Lister options

	ULONG flags;  // Misc. flags

	UWORD lister_popup_code;  // Lister popup code
	UWORD lister_popup_qual;  // Lister popup qualifier

	ULONG env_flags;  // Some other flags

	unsigned short clock_left;	// Clock left position
	unsigned short clock_top;	// Clock top position

	UBYTE devices_col[2];  // Source window colours
	UBYTE volumes_col[2];  // Destination window colours

	UWORD lister_width;	 // Default lister sizes
	UWORD lister_height;

	short version;

	UBYTE gauge_col[2];	 // Free space gauge

	unsigned char icon_fpen;
	unsigned char icon_bpen;
	unsigned char icon_style;

	char env_NewIconsPrecision;

	unsigned long desktop_flags;

	unsigned char iconw_fpen;
	unsigned char iconw_bpen;
	unsigned char iconw_style;

	unsigned char desktop_popup_default;

	ULONG pad[17];	// Save recompilation

	char env_BackgroundPic[4][256];	 // Background pictures
	UWORD env_BackgroundFlags[4];	 // Background flags

	CFG_SETS settings;
	UWORD pad2;

	ULONG env_BackgroundBorderColour[4];

	char themes_location[256];	// Themes folder location

	ULONG pad_big[296];
} OLD_CFG_ENVR;

#ifndef __amigaos3__
	#pragma pack()
#endif
