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

#include "dopuslib.h"
#include "rexx.h"

#ifdef __AROS__
	#define rm_avail rm_Unused1
#endif

#ifdef __amigaos4__
/* OS4 newlib does not provide amiga.lib's SetRexxVar/GetRexxVar. Wrap the
 * rexxsyslib interface (IRexxSys->Set/GetRexxVarFromMsg) to preserve the
 * classic API that the DOpus library uses. */
LONG SetRexxVar(struct RexxMsg *rexxmsg, CONST_STRPTR name, CONST_STRPTR value, LONG length)
{
	if (!IRexxSys)
		return 10;

	if (value && length >= 0)
	{
		STRPTR tmp = AllocVec(length + 1, MEMF_ANY);
		LONG rc;
		if (!tmp)
			return 3;
		CopyMem((APTR)value, tmp, length);
		tmp[length] = '\0';
		rc = SetRexxVarFromMsg(name, tmp, rexxmsg);
		FreeVec(tmp);
		return rc;
	}
	return SetRexxVarFromMsg(name, value, rexxmsg);
}

LONG GetRexxVar(CONST struct RexxMsg *rexxmsg, CONST_STRPTR name, STRPTR *result)
{
	/* Classic amiga.lib returned a pointer to ARexx-internal storage valid
	 * for the lifetime of the message. We emulate that with a per-call static
	 * buffer: callers in DOpus consume the value immediately before issuing
	 * another rexx call. */
	static char buf[4096];
	LONG rc;

	if (!result)
		return 10;
	*result = NULL;
	if (!IRexxSys)
		return 10;

	rc = GetRexxVarFromMsg(name, buf, (struct RexxMsg *)rexxmsg);
	if (rc == 0)
		*result = buf;
	return rc;
}
#endif

static char *RexxMsgIdentifier = "DOPUS";

// Free an ARexx message
void LIBFUNC L_FreeRexxMsgEx(REG(a0, struct RexxMsg *msg))
{
	short args;

	// Need rexx library
	if (!RexxSysBase)
		return;

	// Get argument count
	args = msg->rm_Action & RXARGMASK;

	// If this says there are no arguments, but Arg0 is non-null, we'll free it anyway
	if (args == 0 && msg->rm_Args[0])
		args = 1;

	// Free all arguments
	ClearRexxMsg(msg, args);

	// Is this a DOpus message?
	if (msg->rm_Node.mn_Node.ln_Name == RexxMsgIdentifier)
	{
		struct List *list;

		// Get list pointer
		if ((list = (struct List *)msg->rm_avail))
		{
			struct RexxStem *node, *next;

			// Go through list
			for (node = (struct RexxStem *)list->lh_Head; node->rs_Node.ln_Succ; node = next)
			{
				// Cache next
				next = (struct RexxStem *)node->rs_Node.ln_Succ;

				// Free this node
				FreeVec(node);
			}

			// Free list
			FreeVec(list);
		}
	}

	// Delete the message
	DeleteRexxMsg(msg);
}

// Allocate an ARexx message with space for stem variables
struct RexxMsg *LIBFUNC L_CreateRexxMsgEx(REG(a0, struct MsgPort *port),
										  REG(a1, UBYTE *extension),
										  REG(d0, UBYTE *host))
{
	struct RexxMsg *msg;
	struct List *list;

	// Need rexx library
	if (!RexxSysBase)
		return 0;

	// Create message
	if (!(msg = CreateRexxMsg(port, (CONST_STRPTR)extension, (CONST_STRPTR)host)))
		return 0;

	// Allocate list
	if (!(list = AllocVec(sizeof(struct List), MEMF_CLEAR)))
		return msg;

	// Turn message into a special Opus one
	msg->rm_Node.mn_Node.ln_Name = RexxMsgIdentifier;
	msg->rm_avail = (ULONG)list;

	// Initialise list
	NewList(list);

	return msg;
}

// Set variable for a REXX message
long LIBFUNC L_SetRexxVarEx(REG(a0, struct RexxMsg *msg),
							REG(a1, char *varname),
							REG(d0, char *value),
							REG(d1, long length))
{
	struct RexxStem *var;
	struct List *list;
	short namelen;

	// Invalid varname or message?
	if (!msg || !varname)
		return 10;

	// Valid value?
	if (value)
	{
		// Need string length?
		if (length == -1)
			length = strlen(value);
	}
	else
		length = 0;

	// Standard ARexx message?
	if (IsRexxMsg(msg))
	{
		// Pass through to amiga.lib call
		return SetRexxVar((struct RexxMsg *)msg, varname, value, length);
	}

	// Check for valid DOpus ARexx message
	if (msg->rm_Node.mn_Node.ln_Name != RexxMsgIdentifier)
		return 10;

	// Get list pointer
	if (!(list = (struct List *)msg->rm_avail))
		return 10;

	// See if variable already exists
	if ((var = (struct RexxStem *)L_FindNameI(list, varname)))
	{
		// Remove and free it
		Remove((struct Node *)var);
		FreeVec(var);
	}

	// Invalid value?
	if (!value)
		return 0;

	// Get variable name length
	namelen = strlen(varname) + 1;

	// Allocate variable
	if (!(var = AllocVec(sizeof(struct RexxStem) + namelen + length, MEMF_CLEAR)))
		return 3;

	// Initialise variable
	var->rs_Node.ln_Name = (char *)(var + 1) + length;

	// Copy name and value
	strcpy(var->rs_Node.ln_Name, varname);
	strcpy(var->rs_Value, value);

	// Add to list
	AddTail(list, (struct Node *)var);

	return 0;
}

// Get variable from a REXX message
long LIBFUNC L_GetRexxVarEx(REG(a0, struct RexxMsg *msg), REG(a1, char *varname), REG(a2, char **bufpointer))
{
	struct RexxStem *var;
	struct List *list;

	// Clear pointer
	if (bufpointer)
		*bufpointer = 0;

	// Invalid pointer, varname or message?
	if (!msg || !varname || !bufpointer)
		return 10;

	// Standard ARexx message?
	if (IsRexxMsg(msg))
		return GetRexxVar((struct RexxMsg *)msg, varname, (STRPTR *)bufpointer);

	// Check for valid DOpus ARexx message
	if (msg->rm_Node.mn_Node.ln_Name != RexxMsgIdentifier)
		return 10;

	// Get list pointer
	if (!(list = (struct List *)msg->rm_avail))
		return 10;

	// See if variable exists
	if ((var = (struct RexxStem *)L_FindNameI(list, varname)))
	{
		// Return pointer to value
		*bufpointer = var->rs_Value;
		return 0;
	}

	// Not found
	return 3;
}

// Allocate an ARexx message, and set arguments and stem values automatically
struct RexxMsg *LIBFUNC L_BuildRexxMsgEx(REG(a0, struct MsgPort *port),
										 REG(a1, UBYTE *extension),
										 REG(d0, UBYTE *host),
										 REG(a2, struct TagItem *tags))
{
	struct RexxMsg *msg;
	struct TagItem *tag, *tstate;
	char *varname = 0;

	// Create message
	if (!(msg = L_CreateRexxMsgEx(port, extension, host)))
		return 0;

	// Go through tags
	tstate = tags;
	while ((tag = NextTagItem(&tstate)))
	{
		// Argument string?
		if (tag->ti_Tag >= RexxTag_Arg0 && tag->ti_Tag <= RexxTag_Arg15)
		{
			short arg;

			// Get argument number
			arg = tag->ti_Tag - RexxTag_Arg0;

			// Create argument string
			msg->rm_Args[arg] = CreateArgstring((CONST STRPTR)tag->ti_Data, strlen((char *)tag->ti_Data) + 1);
		}

		// Variable name?
		else if (tag->ti_Tag == RexxTag_VarName)
		{
			// Store pointer to variable name
			varname = (char *)tag->ti_Data;
		}

		// Variable value?
		else if (tag->ti_Tag == RexxTag_VarValue)
		{
			// Got valid name pointer?
			if (varname)
			{
				// Set value
				L_SetRexxVarEx(msg, varname, (char *)tag->ti_Data, -1);
				varname = 0;
			}
		}
	}

	// Return message
	return msg;
}
