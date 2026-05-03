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
#include "config.h"
#include <Program/dopus_config.h>

char *copy_string(APTR, char *);

// Copy a button bank
Cfg_ButtonBank *LIBFUNC L_CopyButtonBank(REG(a0, Cfg_ButtonBank *orig))
{
	Cfg_ButtonBank *bank;
	Cfg_Button *button;

	// Valid bank?
	if (!orig)
		return 0;

	// Allocate a new bank
	if (!(bank = L_NewButtonBank(0, 0)))
		return 0;

	// Copy bank details
	bank->window = orig->window;
	strcpy(bank->path, orig->path);
	strcpy(bank->backpic, orig->backpic);

	// Copy buttons
	for (button = (Cfg_Button *)orig->buttons.lh_Head; button->node.ln_Succ;
		 button = (Cfg_Button *)button->node.ln_Succ)
	{
		Cfg_Button *copy;

		// Copy this button
		if (!(copy = L_CopyButton(button, bank->memory, -1)))
			break;

		// Add to list
		AddTail(&bank->buttons, &copy->node);
	}

	// Did we copy all the buttons?
	if (button->node.ln_Succ)
	{
		// Failed; free what we did get
		L_CloseButtonBank(bank);
		bank = 0;
	}

	// Start menu?
	else if (orig->startmenu && (bank->startmenu = L_AllocMemH(bank->memory, sizeof(CFG_STRT))))
		CopyMem((char *)orig->startmenu, (char *)bank->startmenu, sizeof(CFG_STRT));

	return bank;
}

// Copy a button
Cfg_Button *LIBFUNC L_CopyButton(REG(a0, Cfg_Button *orig), REG(a1, APTR memory), REG(d0, short type))
{
	Cfg_Button *button;

	// Valid button?
	if (!orig)
		return 0;

	// Allocate a new button
	if ((button = L_NewButton(memory)))
	{
		Cfg_ButtonFunction *func, *newfunc;
		BOOL ok = 1;

		// Copy button information
		button->button = orig->button;

		// Fix type
		if (type != -1)
		{
			if (type == 0)
				button->button.flags &= ~BUTNF_GRAPHIC;
			else
				button->button.flags |= BUTNF_GRAPHIC;
		}

		// Go through button functions
		for (func = (Cfg_ButtonFunction *)orig->function_list.mlh_Head; func->node.ln_Succ;
			 func = (Cfg_ButtonFunction *)func->node.ln_Succ)
		{
			// Allocate new function
			if ((newfunc = L_NewButtonFunction(memory, 0)))
			{
				// Copy function
				if (!(L_CopyButtonFunction(func, memory, newfunc)))
					ok = 0;

				// Add to button function list
				AddTail((struct List *)&button->function_list, &newfunc->node);
			}

			// Fail
			else
			{
				ok = 0;
				break;
			}
		}

		// Did we copy all the button functions?
		if (!ok)
		{
			// Failed; free what we did get
			L_FreeButton(button);
			button = 0;
		}
	}

	return button;
}

// Copy a filetype
Cfg_Filetype *LIBFUNC L_CopyFiletype(REG(a0, Cfg_Filetype *orig), REG(a1, APTR memory))
{
	Cfg_Filetype *type;

	// Valid filetype?
	if (!orig)
		return 0;

	// Allocate a new filetype
	if ((type = L_NewFiletype(memory)))
	{
		short a;
		Cfg_Function *function;

		// Copy filetype information
		type->type = orig->type;

		// Copy recognition string and icon path
		type->recognition = (unsigned char *)copy_string(memory, (char *)orig->recognition);
		type->icon_path = copy_string(memory, orig->icon_path);

		// Copy action strings
		if (orig->actions && type->actions)
			for (a = 0; a < 16; a++)
				type->actions[a] = copy_string(memory, orig->actions[a]);

		// Copy function list
		for (function = (Cfg_Function *)orig->function_list.lh_Head; function->node.ln_Succ;
			 function = (Cfg_Function *)function->node.ln_Succ)
		{
			Cfg_Function *copy;

			// Copy this function
			if (!(copy = L_CopyFunction(function, memory, 0)))
				break;

			// Add to function list
			AddTail(&type->function_list, &copy->node);
		}

		// Did we copy all the functions?
		if (function->node.ln_Succ)
		{
			// Failed; free what we did get
			L_FreeFiletype(type);
			type = 0;
		}
	}

	return type;
}

// Copy a function
Cfg_Function *LIBFUNC L_CopyFunction(REG(a0, Cfg_Function *orig), REG(a1, APTR memory), REG(a2, Cfg_Function *newfunc))
{
	Cfg_Function *function;
	Cfg_Instruction *instruction;

	// Valid function?
	if (!orig)
		return 0;

	// Function supplied?
	if (newfunc)
		function = newfunc;

	// Allocate a new function
	else if (!(function = L_NewFunction(memory, 0)))
		return 0;

	// Copy function information
	function->function = orig->function;

	// Copy instructions
	for (instruction = (Cfg_Instruction *)orig->instructions.mlh_Head; instruction->node.mln_Succ;
		 instruction = (Cfg_Instruction *)instruction->node.mln_Succ)
	{
		Cfg_Instruction *copy;

		// Allocate a new instruction
		if (!(copy = L_AllocMemH(memory, sizeof(Cfg_Instruction))) ||
			!(copy->string = copy_string(memory, instruction->string)))
		{
			L_FreeMemH(copy);
			break;
		}

		// Copy type
		copy->type = instruction->type;

		// Add to instruction list
		AddTail((struct List *)&function->instructions, (struct Node *)copy);
	}

	// Did we copy all the instructions?
	if (instruction->node.mln_Succ)
	{
		// Failed; free what we did get
		if (!newfunc)
			L_FreeFunction(function);
		function = 0;
	}

	return function;
}

// Copy a button function
Cfg_ButtonFunction *LIBFUNC L_CopyButtonFunction(REG(a0, Cfg_ButtonFunction *func),
												 REG(a1, APTR memory),
												 REG(a2, Cfg_ButtonFunction *newfunc))
{
	// Copy name
	newfunc->node.ln_Name = copy_string(memory, func->node.ln_Name);

	// Copy label
	newfunc->label = copy_string(memory, func->label);

	// Does original function have an image?
	if (func->image)
	{
		// Copy image
		if (!(newfunc->image = L_CopyImage(func->image)) && func->label)

			// Try to open image
			newfunc->image = L_OpenImage(func->label, 0);
	}

	// Copy function
	if (!(L_CopyFunction((Cfg_Function *)func, memory, (Cfg_Function *)newfunc)))
		return 0;

	return newfunc;
}
