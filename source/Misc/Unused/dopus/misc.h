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

#ifndef _DOPUS_MISC
#define _DOPUS_MISC

/*****************************************************************************

 Miscellaneous functions

 *****************************************************************************/

// Hex ascii to long
long Atoh(char *, long);

// Long to ascii (various methods)
void Itoa(long, char *, char);
void ItoaU(unsigned long, char *, char);

// Convert long value to byte string
void BytesToString(unsigned long, char *, short, char);

// Division to a string
void DivideToString(char *, unsigned long, unsigned long, short, char);

// 32-bit divide with remainder
ULONG DivideU(unsigned long, unsigned long, unsigned long *, struct Library *);

// Convert BSTR to char *
void BtoCStr(BSTR, char *, long);

// Random numbers
void Seed(long);
long RandomDopus(long);

// Build ascii string from rawkey values
void BuildKeyString(unsigned short, unsigned short, unsigned short, unsigned short, char *);

// Valid key qualifiers
UWORD QualValid(unsigned short);

// Convert rawkey values to a character
BOOL ConvertRawKey(unsigned short, unsigned short, char *);

// String manipulation
BOOL StrCombine(char *, char *, char *, long);
BOOL StrConcat(char *, char *, long);

// Calculate a percentage
long CalcPercent(ULONG, ULONG, struct Library *);

#endif
