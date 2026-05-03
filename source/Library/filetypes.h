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
#ifndef _FILETYPES_H
#define _FILETYPES_H

#ifndef __amigaos3__
	#pragma pack(2)
#endif

typedef struct
{
	char *name;					  // Filename
	BPTR lock;					  // File lock
	APTR file;					  // File handle
	unsigned char buffer[1024];	  // Buffer for matching
	unsigned char wild_buf[260];  // Buffer for wildcards
	struct FileInfoBlock fib;	  // File information
	ULONG iff_form;				  // IFF form
	ULONG last_chunk;			  // Last chunk position
	ULONG chunk_size;			  // Size of last chunk
	struct DataType *datatype;	  // DataType handle
	long flags;
	// char			fullname[256];
	char *fullname;
	char pad1[252];			  // the fields below are only accessed fron Library/filetypes.c, but just to be sure
	struct InfoData info;	  // Disk information
	unsigned long disk_type;  // Disk type
} MatchHandle;

#define MATCHF_TRIED_DT (1 << 0)	  // Tried to get datatypes
#define MATCHF_TRIED_MODULE (1 << 1)  // Tried to get module
#define MATCHF_IS_MODULE (1 << 2)	  // Is a module
#define MATCHF_EXECUTABLE (1 << 3)	  // File is executable
#define MATCHF_IS_DISK (1 << 4)		  // Disk

BOOL filetype_match_chars(MatchHandle *handle, BOOL nocase);
void parse_prot_string(unsigned char *string, ULONG *prot);
void parse_date_string(char *string, struct DateStamp *date);

// Filetype matching commands
enum {
	FTOP_NOOP,			// No operation
	FTOP_MATCH,			// Match text
	FTOP_MATCHNAME,		// Match filename
	FTOP_MATCHBITS,		// Match protection bits
	FTOP_MATCHCOMMENT,	// Match comment
	FTOP_MATCHSIZE,		// Match size
	FTOP_MATCHDATE,		// Match date
	FTOP_MOVETO,		// Move to absolute location
	FTOP_MOVE,			// Move to relative location
	FTOP_SEARCHFOR,		// Search for text
	FTOP_MATCHFORM,		// Match an IFF FORM
	FTOP_FINDCHUNK,		// Find an IFF chunk
	FTOP_MATCHDTGROUP,	// Match datatypes group
	FTOP_MATCHDTID,		// Match datatypes ID
	FTOP_MATCHNOCASE,	// Match text case insensitive
	FTOP_DIRECTORY,		// Match directory
	FTOP_MODULE,		// Match sound module
	FTOP_DISK,			// Match disk
	FTOP_SEARCHRANGE,	// Search for text (limited range)
	FTOP_MATCHCHUNK,	// Match an IFF chunk

	FTOP_LAST,	// Last valid command

	FTOP_SPECIAL = 252,	 // Start of special instructions

	FTOP_OR,		 // Or
	FTOP_AND,		 // And
	FTOP_ENDSECTION	 // End of a section
};

#define MATCH_LESS -1
#define MATCH_EQUAL 0
#define MATCH_GREATER 1
#define MATCH_INVALID 2

WORD IsModule(char *);

#define is_digit(c) ((c) >= '0' && (c) <= '9')

typedef struct
{
	struct Node ftc_Node;
	struct MinList ftc_List;
	char ftc_Name[1];
} FileTypeCache;

typedef struct
{
	struct MinNode fte_Node;
	APTR fte_Pointer;
	ULONG fte_Result;
} FileTypeEntry;

void AddFiletypeCache(MatchHandle *, Cfg_Filetype *, ULONG, struct LibData *);
ULONG FindFiletypeCache(MatchHandle *, Cfg_Filetype *, struct LibData *);
void FreeFiletypeCache(struct LibData *, FileTypeCache *);

#ifndef __amigaos3__
	#pragma pack()
#endif

#endif
