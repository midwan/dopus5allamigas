#include "config_lib.h"

void write_str(APTR file, char *str, ...);

extern long _funced_flaglist[];

BOOL LIBFUNC L_FunctionExportASCII(REG(a0, char *filename),
								   REG(a1, Cfg_Button *button),
								   REG(a2, Cfg_Function *function),
								   REG(d0, ULONG a4))
{
	APTR file;
	char *func_ptr;
	short func_len, type_len = 0, a;
	BOOL ret = 1;
	short export_type = 0;

	// A4 supplied?
	if (a4)
	{
	}
	// Export for Key Finder
	else
		export_type = 1;

	// Open file
	if (!(file = OpenBuf(filename, MODE_READWRITE, 2048)))
		return 0;

	// Seek to end (necessary for buffered io I think)
	SeekBuf(file, 0, OFFSET_END);

	// If we've been given a button, get first function
	if (button)
		function = (Cfg_Function *)button->function_list.mlh_Head;

	// Go through function type strings, find longest one
	for (a = MSG_FUNCED_FUNCTYPE_COMMAND; a <= MSG_FUNCED_FUNCTYPE_AREXX; a++)
	{
		short len;

		// Get length
		len = strlen(GetString(locale, a));

		// Longest?
		if (len > type_len)
			type_len = len;
	}

	// Cache pointer to Function label
	func_ptr = GetString(locale, MSG_FUNCTION_ASCII_FUNCTION);
	func_len = (export_type == 1) ? 0 : strlen(func_ptr);

	// Loop while we have a valid function
	while (function && (!button || function->node.ln_Succ))
	{
		Cfg_Instruction *ins;

		// Valid name?
		if (function->node.ln_Name)
		{
			char *ptr;

			// Get pointer
			ptr = GetString(locale, MSG_FUNCTION_ASCII_NAME);

			// Skip spaces for type 1
			if (export_type == 1)
				while (*ptr == ' ')
					++ptr;

			// Write string
			write_str(file, ptr, function->node.ln_Name, 0);
		}

		// If we had a button, that means we have a label
		if (button && ((Cfg_ButtonFunction *)function)->label)
		{
			char *ptr;

			// Get pointer
			ptr = GetString(
				locale, (button->button.flags & BUTNF_GRAPHIC) ? MSG_FUNCTION_ASCII_IMAGE : MSG_FUNCTION_ASCII_LABEL);

			// Skip spaces for type 1
			if (export_type == 1)
				while (*ptr == ' ')
					++ptr;

			write_str(file, ptr, ((Cfg_ButtonFunction *)function)->label, 0);
		}

		// Any key?
		if (function->function.code != 0xffff && export_type != 1)
		{
			char buf[80];

			// Get key string
			BuildKeyString(function->function.code,
						   function->function.qual,
						   function->function.qual_mask,
						   function->function.qual_same,
						   buf);

			// Output key
			write_str(file, GetString(locale, MSG_FUNCTION_ASCII_KEY), buf, 0);
		}

		// Function label
		if (export_type != 1)
			WriteBuf(file, "\n", 1);
		WriteBuf(file, (export_type == 1) ? "---\n" : func_ptr, -1);

		// Go through instructions
		for (ins = (Cfg_Instruction *)function->instructions.mlh_Head, a = 0; ins->node.mln_Succ;
			 ins = (Cfg_Instruction *)ins->node.mln_Succ)
		{
			char *str;
			short len;

			// Skip labels
			if (ins->type >= INST_LABEL)
				continue;

			// If this isn't the first instruction, pad line start with spaces
			if (a++ > 0)
			{
				for (len = 0; len < func_len; len++)
					WriteBuf(file, " ", 1);
			}

			// Get pointer to type string
			str = GetString(locale, MSG_FUNCED_FUNCTYPE_COMMAND + ins->type);
			len = strlen(str);

			// Write type string
			WriteBuf(file, str, len--);

			// Pad with spaces
			while (len++ <= type_len)
				WriteBuf(file, " ", 1);

			// Write instruction string
			if (ins->string)
				WriteBuf(file, ins->string, -1);

			// CR
			WriteBuf(file, "\n", 1);
		}

		// End of the function definition
		WriteBuf(file, "\n", 1);

		// Any flags?
		if (function->function.flags && export_type != 1)
		{
			long flag;
			BOOL first = 1;

			// Write flags label
			WriteBuf(file, GetString(locale, MSG_FUNCTION_ASCII_FLAGS), -1);

			// Go through flags
			for (flag = 0; _funced_flaglist[flag]; flag += 2)
			{
				// Is this flag set?
				if (function->function.flags & _funced_flaglist[flag])
				{
					// If not the first flag, pad with spaces
					if (!first)
					{
						short a;

						for (a = 0; a < func_len; a++)
							WriteBuf(file, " ", 1);
					}
					else
						first = 0;

					// Write flag string
					WriteBuf(file, GetString(locale, _funced_flaglist[flag + 1]), -1);
					WriteBuf(file, "\n", 1);
				}
			}

			// End of the flags
			WriteBuf(file, "\n", 1);
		}

		// If not a button, break out
		if (!button)
			break;

		// Get next function
		function = (Cfg_Function *)function->node.ln_Succ;
	}

	// Close file
	CloseBuf(file);
	return ret;
}

// Write strings to a file
void write_str(APTR file, char *str, ...)
{
	//	char **ptr;
	char *string = NULL;
	va_list va;

	// Write first string
	if (str)
		WriteBuf(file, str, -1);

	va_start(va, str);

	// While string is valid
	while ((string = va_arg(va, STRPTR)))
	{
		WriteBuf(file, string, -1);
	}
	/*
		// Get first string
		ptr=&str;

		// While string is valid
		while (*ptr)
		{
			WriteBuf(file,*ptr,-1);
			ptr++;
		}
	*/
	// CR
	WriteBuf(file, "\n", 1);
}

void function_export_cmd(char *filename, Cfg_Button *button, Cfg_Function *function)
{
	APTR iff;

	// If we've been given a button, get first function
	if (button)
		function = (Cfg_Function *)button->function_list.mlh_Head;

	// Open IFF file
	if (!(iff = IFFOpen(filename, IFF_WRITE, ID_OPUS)))
		return;

	// Loop while we have a valid function
	while (function && (!button || function->node.ln_Succ))
	{
		// Save the function
		SaveFunction(iff, function);

		// No more functions?
		if (!button)
			break;

		// Get next function
		function = (Cfg_Function *)function->node.ln_Succ;
	}

	// Close file
	IFFClose(iff);
}
