#include <stddef.h>
#include <ctype.h>
/* makesafe: make a string safe for printing */
const char * makesafe(const char *p)
{
	static char out[30 + 1];
	size_t i;

	for (i = 0; *p != '\0' && i < sizeof(out) - 2; p++, i++)
		if (!isprint(*p))
			out[i] = '?';
		else
			out[i] = *p;
	out[i] = '\0';
	return out;
}
