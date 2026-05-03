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

#include "dopus.h"

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define is_space(c) ((c) == ' ' || (c) == '\t')

// Get an ARexx command
short rexx_get_command(char **commandptr)
{
	short cmd;
	char *command = *commandptr;

	// Skip whitespaces
	rexx_skip_space(&command);

	// Go through rexx command list
	for (cmd = 0; rexx_commands[cmd].command; cmd++)
	{
		short len;

		// Get command length
		len = strlen(rexx_commands[cmd].command);

		// Check correct length (to save time)
		if (!command[len] || is_space(command[len]))
		{
			// Compare command
			if (strnicmp(command, rexx_commands[cmd].command, len) == 0)
			{
				// Bump pointer to end of command
				*commandptr = command + len;
				return rexx_commands[cmd].id;
			}
		}
	}

	// No match
	return 0;
}

// Skip whitespaces
void rexx_skip_space(char **command)
{
	while (*(*command) && is_space(*(*command)))
		++(*command);
}

void rexx_skip_space_reverse(char **command, char *end)
{
	while (*command > end && is_space(*(*command)))
		--(*command);
}

// Parse number out of string
long rexx_parse_number(char **ptr, BOOL next, long def)
{
	// Digit?
	if (is_digit(*(*ptr)) || *(*ptr) == '-')
	{
		def = atoi(*ptr);
		while (*(*ptr) && (is_digit(*(*ptr)) || *(*ptr) == '-'))
			++*ptr;
		if (next)
		{
			while (*(*ptr) && (!is_digit(*(*ptr)) && *(*ptr) != '-'))
				++*ptr;
		}
	}
	return def;
}

IPTR rexx_parse_iptr(char **ptr, BOOL next, IPTR def)
{
	if (is_digit(*(*ptr)))
	{
		UQUAD value = 0;

		while (*(*ptr) && is_digit(*(*ptr)))
		{
			value = (value * 10) + (*(*ptr) - '0');
			++*ptr;
		}

		def = (IPTR)value;
		if (next)
		{
			while (*(*ptr) && !is_digit(*(*ptr)))
				++*ptr;
		}
	}
	return def;
}

void rexx_format_iptr(char *buf, IPTR value)
{
	char tmp[32];
	int pos = sizeof(tmp) - 1;
	UQUAD number = (UQUAD)value;

	tmp[pos] = 0;
	do
	{
		tmp[--pos] = '0' + (number % 10);
		number /= 10;
	} while (number && pos > 0);

	strcpy(buf, tmp + pos);
}

// Parse number out of string (2nd version)
BOOL rexx_parse_number_byte(char **ptr, UBYTE *val)
{
	// Digit?
	rexx_skip_space(ptr);
	if (is_digit(*(*ptr)) || *(*ptr) == '-')
	{
		*val = atoi(*ptr);
		while (*(*ptr) && (is_digit(*(*ptr)) || *(*ptr) == '-'))
			++*ptr;
		return 1;
	}
	return 0;
}

// Parse a word out of a string
short rexx_parse_word(char **ptrptr, char *buffer, short bufferlen)
{
	short len = 0, quote = 0;
	char *ptr;

	// Skip preceeding spaces
	rexx_skip_space(ptrptr);
	ptr = *ptrptr;

	// Clear buffer
	*buffer = 0;

	// Copy until end, whitespace or buffer full
	while (*ptr)
	{
		// Quote?
		if (*ptr == '\"' && quote != 2)
		{
			if (quote == 0)
				quote = 1;
			else
				quote = 0;
		}

		// Other quote?
		else if (*ptr == '\'' && quote != 1)
		{
			if (quote == 0)
				quote = 2;
			else
				quote = 0;
		}

		// White space?
		else if (is_space(*ptr) && !quote)
			break;

		// Store character?
		else if (len < bufferlen - 1)
			buffer[len++] = *ptr;

		// Increment pointer
		++ptr;
		++*ptrptr;
	}

	// If not on a whitespace, find next space
	while (*ptr && !(is_space(*ptr)))
	{
		++*ptrptr;
		++ptr;
	}

	// Null-terminate buffer
	buffer[len] = 0;
	return len;
}

// Match a keyword
short rexx_match_keyword(char **ptr, char **keys, long *type)
{
	short num, len;

	// Clear type
	if (type)
		*type = KEY_NONE;

	// Go through array
	for (num = 0; keys[num]; num++)
	{
		char *str;

		// Get length of keyword
		len = strlen(keys[num]);

		// Get string pointer
		str = *ptr;

		// Can specify type?
		if (type)
		{
			// Add?
			if (*str == '+')
			{
				*type = KEY_ADD;
				++str;
			}

			// Subtract?
			else if (*str == '-')
			{
				*type = KEY_SUB;
				++str;
			}

			// Toggle?
			else if (*str == '/')
			{
				*type = KEY_TOGGLE;
				++str;
			}
		}

		// Correct length?
		if (!str[len] || is_space(str[len]))
		{
			// Match?
			if (strnicmp(str, keys[num], len) == 0)
			{
				// Yep, bump pointer to end of word
				*ptr = str + len;

				// Skip any spaces
				rexx_skip_space(ptr);
				return num;
			}
		}
	}

	// No match
	return -1;
}

// See if a lister is valid
BOOL rexx_lister_valid(Lister *lister)
{
	IPCData *ipc;

	// Go through listers
	for (ipc = (IPCData *)GUI->lister_list.list.lh_Head; ipc->node.mln_Succ; ipc = (IPCData *)ipc->node.mln_Succ)
	{
		// Does lister match?
		if (IPCDATA(ipc) == (APTR)lister && !(lister->flags & LISTERF_CLOSING))
			return 1;
	}

	// Didn't find lister
	return 0;
}

// Set a variable
void rexx_set_var(struct RexxMsg *msg, char *stem, char *name, IPTR data, short type)
{
	char buf[80], buf2[60];

	// Long?
	if (type == RX_LONG)
	{
		lsprintf(buf2, "%ld", data);
		data = (IPTR)buf2;
	}

	// Char?
	else if (type == RX_CHAR)
	{
		lsprintf(buf2, "%lc", data);
		data = (IPTR)buf2;
	}

	// Build variable name
	if (name)
		lsprintf(buf, (stem[strlen(stem) - 1] == '.') ? "%s%s" : "%s.%s", stem, name);
	else
		strcpy(buf, stem);

	// Set variable
	SetRexxVarEx(msg, buf, (char *)data, -1);
}

// Get a variable
BOOL rexx_get_var(struct RexxMsg *msg, char *stem, char *name, char *buffer, short size)
{
	char buf[80], *ptr;

	// Build variable name
	lsprintf(buf, "%s%s", stem, name);

	// Get variable
	if (!(GetRexxVarEx(msg, buf, &ptr)) && ptr)
	{
		// Check buffer doesn't equal variable name
		if (stricmp(buf, ptr) != 0)
		{
			// Copy variable
			stccpy(buffer, ptr, size);
			return 1;
		}
	}

	// Clear buffer
	*buffer = 0;
	return 0;
}
