// by Thomas and Salas00 from amigans.net

#include <math.h>

/* Resolve PI at runtime (via atan(1.0)*4) instead of as a compile-time
 * constant. The AROS x86_64 cross-compiler shipped in
 * midwan/aros-compiler:x86_64-aros (GCC 10.5) parses every literal
 * float and folds every compile-time double->float conversion through
 * MPFR/GMP. GMP's `__gmpn_mul_1` is hand-written x86_64 assembly that
 * hits an instruction Apple Rosetta 2 mis-emulates - cc1 dies with
 * "internal compiler error: Illegal instruction" before it even
 * reaches the optimiser. The minimal trigger is one line:
 *
 *     float t(void) { return (float)3.141592653589793; }   // ICE
 *
 * Even `static const float PI = 3.14159265f;` crashes for the same
 * reason (GMP-precision parsing of the literal). Real x86_64 Linux
 * hosts (the GitHub CI runner) are unaffected, so this only matters
 * for local cross-builds on Apple Silicon. atan(1.0) is a function
 * call that GCC does not const-fold at -O2 here, so the GMP path
 * is never reached. */
static float dopus_diskinfo_pi(void)
{
	static float pi_value;
	if (pi_value == 0.0f)
		pi_value = (float)atan(1.0) * 4.0f;
	return pi_value;
}

#ifndef PI
	#define PI dopus_diskinfo_pi()
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
