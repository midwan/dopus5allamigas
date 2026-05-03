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

#ifndef _DOPUS_REQ
#define _DOPUS_REQ

/*****************************************************************************

 Requesters

 *****************************************************************************/

// Requesters available
enum {
	REQTYPE_FILE,
	REQTYPE_SIMPLE,
};

#define AR_Requester TAG_USER + 1  // Pointer to requester

#define AR_Window TAG_USER + 2		// Window
#define AR_Screen TAG_USER + 3		// Screen
#define AR_Message TAG_USER + 4		// Text message
#define AR_Button TAG_USER + 5		// Button label
#define AR_ButtonCode TAG_USER + 6	// Code for this button
#define AR_Title TAG_USER + 7		// Title string
#define AR_Buffer TAG_USER + 8		// String buffer
#define AR_BufLen TAG_USER + 9		// Buffer length
#define AR_History TAG_USER + 10	// History list
#define AR_CheckMark TAG_USER + 11	// Check mark text
#define AR_CheckPtr TAG_USER + 12	// Check mark data storage
#define AR_Flags TAG_USER + 13		// Flags

// Flags for REQTYPE_SIMPLE
#define SRF_LONGINT (1 << 0)	  // Integer gadget
#define SRF_CENTJUST (1 << 1)	  // Center justify
#define SRF_RIGHTJUST (1 << 2)	  // Right justify
#define SRF_HISTORY (1 << 8)	  // History supplied
#define SRF_PATH_FILTER (1 << 9)  // Filter path characters
#define SRF_CHECKMARK (1 << 11)	  // Checkmark supplied
#define SRF_SECURE (1 << 12)	  // Secure field
#define SRF_MOUSE_POS (1 << 13)	  // Position over mouse pointer

// Used to declare a callback for a requester
#ifndef REF_CALLBACK
	#error foo
	#define REF_CALLBACK(name) void __asm (*name)(register __d0 ULONG, register __a0 APTR, register __a1 APTR)
#endif

long AsyncRequest(IPCData *, long, struct Window *, REF_CALLBACK(), APTR, struct TagItem *);
long __stdargs AsyncRequestTags(IPCData *, long, struct Window *, REF_CALLBACK(), APTR, Tag, ...);

// SelectionList

#define SLF_DIR_FIELD (1 << 0)	// Directory field

short SelectionList(Att_List *, struct Window *, struct Screen *, char *, short, ULONG, char *, char *, char *);

// Status window
struct Window *OpenStatusWindow(char *, char *, struct Screen *, LONG, ULONG);
void SetStatusText(struct Window *, char *);

#endif
