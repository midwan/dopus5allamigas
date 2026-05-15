#include "config_lib.h"
#include "config_environment.h"
#include <proto/module.h>

static void config_env_insert_raw_string(ObjectList *list, ULONG id, char *string)
{
	char buffer[256], tempbuf[256];
	struct Gadget *gadget;
	struct StringInfo *info;
	GL_Object *object;
	short len, buflen, maxlen, insert_len;

	// Get gadget
	if (!(object = GetObject(list, id)) || !(gadget = GADGET(object)) ||
		!(info = (struct StringInfo *)gadget->SpecialInfo))
		return;

	// Get current string
	maxlen = sizeof(buffer) - 1;
	if (info->MaxChars > 0 && info->MaxChars - 1 < maxlen)
		maxlen = info->MaxChars - 1;
	stccpy(buffer, info->Buffer, maxlen + 1);
	buflen = strlen(buffer);
	len = info->BufferPos;
	if (len < 0)
		len = 0;
	else if (len > buflen)
		len = buflen;

	// Copy from current position into second buffer
	stccpy(tempbuf, buffer + len, sizeof(tempbuf));
	buffer[len] = 0;

	// Insert new string
	insert_len = strlen(string);
	if (insert_len > maxlen - len)
		insert_len = maxlen - len;
	if (insert_len > 0)
	{
		stccpy(buffer + len, string, insert_len + 1);
		len += insert_len;
	}
	if (len < maxlen)
		stccpy(buffer + len, tempbuf, maxlen - len + 1);
	else
		buffer[len] = 0;

	// Set new string in gadget
	SetGadgetValue(list, id, (IPTR)buffer);

	// Bump buffer position
	buflen = strlen(info->Buffer);
	if (len > buflen)
		len = buflen;
	info->BufferPos = len;

	// Activate gadget
	ActivateGadget(gadget, list->window, 0);
}

// Show list of text format codes
static void config_env_code_list(ObjectList *objlist, ULONG id, long first, long last, BOOL raw_insert)
{
	Att_List *list;
	short a, b;
	char name[80];

	// Build list
	if (!(list = Att_NewList(0)))
		return;
	for (a = first; a <= ((last) ? last : 32767); a++)
	{
		stccpy(name, GetString(locale, a), sizeof(name));
		if (name[0] == '-' && name[1] == 0)
			break;
		for (b = 0; name[b] && name[b] != '\t'; b++)
			if (name[b] == '*')
				name[b] = '%';
		Att_NewNode(list, name, 0, 0);
	}

	// Make window busy
	SetWindowBusy(objlist->window);

	// Display selection list
	a = SelectionList(list,
					  objlist->window,
					  0,
					  GetString(locale, MSG_ENVIRONMENT_LISTER_SELECT_STATUS),
					  -1,
					  0,
					  0,
					  GetString(locale, MSG_OKAY),
					  GetString(locale, MSG_CANCEL),
					  0,
					  0);

	// Clear busy
	ClearWindowBusy(objlist->window);

	// Selection?
	if (a != -1)
	{
		Att_Node *node;

		// Get node
		if ((node = Att_FindNode(list, a)))
		{
			char *ptr, buf[10];

			// Get string pointer
			ptr = strchr(node->node.ln_Name, '\t');
			if (ptr)
				stccpy(buf, node->node.ln_Name, (ptr - node->node.ln_Name) + 1);
			else
				stccpy(buf, node->node.ln_Name, sizeof(buf));

			// Insert into gadget
			if (raw_insert)
				config_env_insert_raw_string(objlist, id, buf);
			else
				funced_edit_insertstring(objlist, id, buf, DOpusBase, (struct Library *)IntuitionBase);
		}
	}

	// Free list
	Att_RemList(list, 0);
}

// Show list of status bar options
void _config_env_status_list(ObjectList *objlist, ULONG id, long first, long last)
{
	config_env_code_list(objlist, id, first, last, FALSE);
}

// Show list of clock format options
void _config_env_clock_format_list(ObjectList *objlist, ULONG id, long first, long last)
{
	config_env_code_list(objlist, id, first, last, TRUE);
}
