#include <libgen.h>
#include <limits.h>
#ifdef __linux__
#include <linux/limits.h>
#endif
#include <stddef.h>
#include <string.h>
#include <stdio.h>
/* validselector: check if a given selector is included in a map */
int validselector(const char *selector, const char *host)
{
	enum { TITLE=0, SELECTOR, HOST, PORT };
	FILE *fp;
	char line[256], file[PATH_MAX];
	char sel[PATH_MAX], *dn;

	if (snprintf(sel, sizeof(sel), "%s", selector) >= sizeof(sel))
		return 0;		/* truncated selector */

	dn = dirname(sel);

	if (*dn != '/')
		return 0;		/* relative path */
	snprintf(file, sizeof(file), "%smap", dn);
	if ((fp = fopen(file, "r")) == NULL)
		return 0;		/* cannot access file */
	while (fgets(line, sizeof(line), fp) != NULL) {
		char *p;
		size_t i;
		char *ent[PORT + 1];	/* field ptrs to line */

		p = strtok(line, "\t");
		i = 0;
		while (p != NULL && i < sizeof(ent)) {
			ent[i++] = p;
			p = strtok(NULL, "\t");
		}
		if (i != PORT+1)
			continue;	/* wrong number of fields */
		if (strcmp(ent[SELECTOR], selector) == 0 &&
		    strcmp(ent[HOST], host) == 0)
			return 1;	/* match */
	}
	return 0;			/* no match */
}
