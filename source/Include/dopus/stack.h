#ifndef DOPUS_STACK_H
#define DOPUS_STACK_H

// transforms a define into a string
#ifndef STRI
	#define STRI(x) STRI2(x)
	#define STRI2(x) #x
#endif

// Stack sizes
// STACK_LARGE used for program, configopus & DO_LAUNCHER
// STACK_DEFAULT used for starting processes
#ifndef __amigaos3__
	#define STACK_DEFAULT 16384
	#define STACK_LARGE 32768
#else
	#define STACK_DEFAULT 8192
	#define STACK_LARGE 16384
#endif

// Stack string for OS4 stack cookie
#define STACK_STRING STRI(STACK_LARGE)

#endif /* DOPUS_STACK_H */
