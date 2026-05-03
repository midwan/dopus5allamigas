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

#ifndef __amigaos3__
	#pragma pack(2)
#endif

struct MusicBase
{
	struct Library MLib;
	BPTR ml_SegList;
	APTR ml_Module;
	LONG ml_Len;
	WORD ml_Playing, ml_ModType, ml_Volume, ml_Tempo, ml_Tracks, ml_SongPos;
	struct Library *ml_DOSBase, *ml_PPBase;
	char *ml_SongName;
};

#ifndef __amigaos3__
	#pragma pack()
#endif

/* Module types currently supported */
#define MOD_STNTPT 1 /* Sound/Noise/ProTracker */
#define MOD_MED 2	 /* MED Module (MMD0) */
#define MOD_OKTA 3	 /* Oktalyzer mod */
#define MOD_OLDST 4	 /* 15 Inst SoundTracker */

/* Error codes returned by PlayModule

**********************NOTE********************************
 if err code is < 0, then the failure code is a PowerPacker.Library fail

*/

#define ML_NOMEM 101		/* not enuff memory to load module */
#define ML_BADMOD 102		/* garbled or not-supported */
#define ML_NOMOD 103		/* DOS Open failed */
#define ML_NOPLAYER 104		/* CIA Player Allocations Failed */
#define ML_LIBLOCKED 105	/* LIBBASE is Locked for processing */
#define ML_INVALIDFNAME 106 /* Bad file name */
#define ML_NOAUDIO 107		/* Couldn't lock Audio channels */

WORD PlayModule(char *, BOOL);
VOID StopModule(void);
WORD IsModule(char *);
VOID FlushModule(void);
VOID ContModule(void);
VOID SetVolume(WORD);
VOID PlayFaster(void);
VOID PlaySlower(void);
VOID TempoReset(void);
