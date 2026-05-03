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
#include <Program/dopus_config.h>

char *copy_string(APTR, char *);

// Create a new button bank
Cfg_ButtonBank *LIBFUNC L_NewButtonBank(REG(d0, BOOL init), REG(d1, short type))
{
	Cfg_ButtonBank *bank;

	// Allocate button bank
	if (!(bank = AllocVec(sizeof(Cfg_ButtonBank), MEMF_CLEAR)) ||
		!(bank->memory = L_NewMemHandle(2048, 768, MEMF_CLEAR)))
	{
		FreeVec(bank);
		return 0;
	}

	// Initialise bank
	NewList(&bank->buttons);
	InitSemaphore(&bank->lock);

	// Default settings?
	if (init)
	{
		Cfg_Button *button;

		// Initialise with default settings
		strcpy(bank->window.name, "Untitled");
		bank->window.pos.Left = -1;
		bank->window.pos.Width = 128;
		bank->window.pos.Height = -1;
		bank->window.columns = 1;
		bank->window.rows = 1;
		strcpy(bank->window.font_name, ((struct GfxBase *)GfxBase)->DefaultFont->tf_Message.mn_Node.ln_Name);
		bank->window.font_size = ((struct GfxBase *)GfxBase)->DefaultFont->tf_YSize;
		if (type)
			bank->window.flags = BTNWF_GFX;

		// Create a button
		if ((button = L_NewButton(bank->memory)))
		{
			AddTail(&bank->buttons, &button->node);
			if (type)
				button->button.flags |= BUTNF_GRAPHIC;
		}
	}

	return bank;
}

// Create a new button
Cfg_Button *LIBFUNC L_NewButton(REG(a0, APTR memory))
{
	Cfg_Button *button;

	// Allocate button
	if (!(button = L_AllocMemH(memory, sizeof(Cfg_Button))))
		return 0;

	// Initialise button
	NewList((struct List *)&button->function_list);
	button->button.fpen = 1;
	button->current = FTYPE_LEFT_BUTTON;
	return button;
}

// New button with function
Cfg_Button *LIBFUNC L_NewButtonWithFunc(REG(a0, APTR memory), REG(a1, char *label), REG(d0, short type))
{
	Cfg_Button *button;
	Cfg_ButtonFunction *func;

	// Create button
	if (!(button = L_NewButton(memory)))
		return 0;

	// Create new function
	if (!(func = L_NewButtonFunction(memory, type)))
	{
		// Failed, free button
		L_FreeButton(button);
		return 0;
	}

	// Add function to button
	AddTail((struct List *)&button->function_list, &func->node);

	// Allocate label
	if (label && (func->label = L_AllocMemH(memory, strlen(label) + 1)))
		strcpy(func->label, label);

	// Allocate name
	if (label && (func->node.ln_Name = L_AllocMemH(memory, strlen(label) + 1)))
		strcpy(func->node.ln_Name, label);

	return button;
}

// Create a new lister
Cfg_Lister *LIBFUNC L_NewLister(REG(a0, char *path))
{
	Cfg_Lister *lister;

	// Create lister
	if (!(lister = AllocVec(sizeof(Cfg_Lister), MEMF_CLEAR)))
		return 0;

	// If path is supplied, copy it
	if (path)
		lister->path = copy_string(0, path);

	return lister;
}

// Create a new function
Cfg_Function *LIBFUNC L_NewFunction(REG(a0, APTR memory), REG(d0, UWORD type))
{
	Cfg_Function *function;

	// Allocate function
	if (!(function = L_AllocMemH(memory, sizeof(Cfg_Function))))
		return 0;

	// Initialise function
	NewList((struct List *)&function->instructions);
	function->function.code = 0xffff;
	function->function.qual_mask = 0xffff;
	function->function.func_type = type;

	return function;
}

// Create a new button function
Cfg_ButtonFunction *LIBFUNC L_NewButtonFunction(REG(a0, APTR memory), REG(d0, UWORD type))
{
	Cfg_ButtonFunction *function;

	// Allocate function
	if (!(function = L_AllocMemH(memory, sizeof(Cfg_ButtonFunction))))
		return 0;

	// Initialise function
	NewList((struct List *)&function->instructions);
	function->function.code = 0xffff;
	function->function.func_type = type;

	return function;
}

// Create a new instruction
Cfg_Instruction *LIBFUNC L_NewInstruction(REG(a0, APTR memory), REG(d0, short type), REG(a1, char *string))
{
	Cfg_Instruction *ins;

	// Create instruction and string
	if ((ins = L_AllocMemH(memory, sizeof(Cfg_Instruction))) &&
		(!string || (ins->string = L_AllocMemH(memory, strlen(string) + 1))))
	{
		ins->type = type;
		if (string)
			strcpy(ins->string, string);
		return ins;
	}
	L_FreeMemH(ins);
	return 0;
}

// Create a new filetype
Cfg_Filetype *LIBFUNC L_NewFiletype(REG(a0, APTR memory))
{
	Cfg_Filetype *type;

	// Create filetype
	if ((type = L_AllocMemH(memory, sizeof(Cfg_Filetype))) &&
		(type->actions = L_AllocMemH(memory, sizeof(char *) * 16)))
	{
		NewList(&type->function_list);
		return type;
	}

	// Failed
	L_FreeMemH(type);
	return 0;
}

// Copy a string
char *copy_string(APTR memory, char *original)
{
	char *copy = 0;

	if (original && (copy = L_AllocMemH(memory, strlen(original) + 1)))
		strcpy(copy, original);

	return copy;
}
