#include <stdlib.h>
#include <syslog.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
void sendfile(const char *file, int fd);
int validselector(const char *selector, const char *host);
const char *makesafe(const char *p);
int _timeout;
/* session: serve a client */
void session(int fd, const char *hostname)
{
	char c, buf[256];
	size_t len;
	int n;

	_timeout = 0;
	/* read selector */
	len = 0;
	while ((n = read(fd, &c, sizeof(char))) > 0) {
		if (c == '\r' || c == '\n')
			break;
		buf[len++] = c;
		if (len+1 == sizeof(buf))
			break;
	}
	if (n <= 0) {
		if (_timeout)
			syslog(LOG_ERR, "timeout");
		else if (n == 0)
			syslog(LOG_ERR, "EOF");
		else if (n == -1)
			syslog(LOG_ERR, "read: %m");
		return;
	}
	buf[len] = '\0';
	/* write selector content */
	if (len == 0 || strcmp(buf, "/") == 0)
		sendfile("map", fd);
	else if (validselector(buf, hostname))
		sendfile(buf, fd);
	else
		syslog(LOG_ERR, "non-existent '%s'", makesafe(buf));
}
