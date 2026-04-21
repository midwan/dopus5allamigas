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

#include "misc.h"

ModuleInfo_3 module_info = {1,
							"misc.module",
							"misc.catalog",
							0,
							3,
							{{BEEP, "Beep", MSG_BEEP_DESC, 0, 0},
							 {FLASH, "Flash", MSG_FLASH_DESC, 0, 0},
							 {ALARM, "Alarm", MSG_ALARM_DESC, 0, 0}}};

// Beep wave-form
__chip char beepwave[16] = {0, 30, 60, 95, 127, 95, 60, 30, 0, -30, -60, -95, -127, -95, -60, -30};

// Audio channels to allocate
UBYTE achannels[8] = {3, 5, 10, 12, 1, 2, 4, 8};
