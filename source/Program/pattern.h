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

#ifndef _PATTERN_H
#define _PATTERN_H

#define PATTILE_SIZE 64

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct PatternInstance
{
	struct Hook hook;			  // Backfill hook
	struct PatternData *pattern;  // Pattern info

	unsigned short fill_fix[8][16];	 // Adjusted pattern
	short offset_x;					 // Pattern offset
	short offset_y;					 // Pattern offset

	BOOL disabled;	// Pattern is not to be shown
} PatternInstance;

typedef struct PatternData
{
	BOOL valid;	 // Valid pattern?

	struct WBPatternPrefs prefs;  // Preferences data
	char *data;					  // Pattern data

	short width;   // Pattern width
	short height;  // Pattern height

	unsigned short fill[8][16];		// Fill pattern
	unsigned short fill_plane_key;	// Indicates planes that are used

	Object *object;					// Datatypes object
	struct BitMap *bitmap;			// Bitmap to blit from
	struct BitMap *pattern_bitmap;	// Might be allocated

	short old_depth;   // Old bitmap depth
	short border_pen;  // Allocated border pen
	short random;

	struct Screen *screen;	// Screen pattern is displayed on

	ULONG flags;	  // FastIPrefs flags
	short precision;  // Precision type
} PatternData;

#define PATF_LOCK 0x00000002
#define PATF_STRETCH 0x10000000
#define PATF_CENTER 0x20000000
#define PATF_TILE 0x40000000
#define PATF_FILL 0x80000000

#define PATF_RANDOM 0x00004000

#define PATF_WB_MODE(f) (((f)&0x3000000) >> 20)
#define PATF_LISTER_MODE(f) (((f)&0x30000) >> 16)

#define ID_FIP0 MAKE_ID('F', 'I', 'P', '0')
#define ID_WPOS MAKE_ID('W', 'P', 'O', 'S')

void GetPattern(PatternData *, struct Screen *, ULONG);
void GetPatternBorder(PatternData *, struct Screen *, ULONG);
void FreePattern(PatternData *);

void ASM SAVEDS PatternBackfill(REG(a0, PatternInstance *), REG(a2, struct RastPort *), REG(a1, struct BackFillInfo *));

#define MOD(x, y) ((x) < 0 ? (y) - ((-(x)) % (y)) : (x) % (y))

BOOL pattern_check_random(PatternData *, char *);

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
