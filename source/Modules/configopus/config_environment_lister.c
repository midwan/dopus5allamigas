#include "config_lib.h"
#include "config_environment.h"
#include <proto/module.h>

// Show list of status bar options
void _config_env_status_list(ObjectList *objlist, ULONG id, long first, long last)
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
			funced_edit_insertstring(objlist, id, buf, DOpusBase, (struct Library *)IntuitionBase);
		}
	}

	// Free list
	Att_RemList(list, 0);
}
