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
/* Flags relating to configuration options */

#ifndef _CONFIGFLAGS_H
#define _CONFIGFLAGS_H

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
