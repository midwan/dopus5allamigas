BOOL CreateFunctionFile(char *name, char *instruction)
{
	Cfg_Function *func;
	Cfg_Instruction *ins;
	APTR iff;

	// Create function
	if (!(func = NewFunction(0, FTYPE_LEFT_BUTTON)))
		return 0;

	// Create instruction (you could have multiple instructions if you wanted)
	if (ins = NewInstruction(0, INST_COMMAND, instruction))
		AddTail((struct List *)&func->instructions, (struct Node *)ins);

	// Open IFF file
	if (!(iff = IFFOpen(name, IFF_WRITE, ID_OPUS)))
		return;

	// Save the function
	SaveFunction(iff, func);

	// Close file
	IFFClose(iff);

	// Free function
	FreeFunction(func);
}
