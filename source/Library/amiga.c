/* Replacement functions */

#if !defined(__MORPHOS__) && !defined(__AROS__)
int stccpy(char *p, const char *q, int n)
{
	char *t = p;

	while ((*p++ = *q++) && --n > 0)
	{
	};
	p[-1] = '\0';

	return p - t;
}
#endif
