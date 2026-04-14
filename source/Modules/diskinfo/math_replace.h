// by Thomas and Salas00 from amigans.net

#include <math.h>

#ifndef PI
	#define PI M_PI
#endif

float SPFlt(int inum)
{
	return (inum);
}

int SPFix(float fnum)
{
	return (fnum);
}

float SPSub(float fnum1, float fnum2)
{
	return (fnum2 - fnum1);
}

float SPMul(float fnum1, float fnum2)
{
	return (fnum1 * fnum2);
}

float SPDiv(float fnum1, float fnum2)
{
	return (fnum2 / fnum1);
}

float SPSincos(float *pfnum2, float fnum1)
{
	*pfnum2 = cos(fnum1);
	return (sin(fnum1));
}

/*
 * sacredbanana/amiga-compiler libm020/libm.a(cexp.o) has undefined refs
 * to `cimag` and `creal` (no archive member provides them). GCC 6.5.0b
 * with -Os -flto pulls cexp.o in spuriously while merging math sections.
 * Diskinfo never actually calls cexp — these stubs just satisfy the
 * linker. If something ever routes through cexp, these return the real
 * and imaginary parts correctly via GCC's __real__ / __imag__.
 */
double creal(double _Complex z) { return __real__ z; }
double cimag(double _Complex z) { return __imag__ z; }
