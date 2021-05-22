#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <err.h>
#include <sys/socket.h>
int tcpbind(const char *ip, int port)
{
	int fd;
	struct sockaddr_in a;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err(1, "socket");
	bzero(&a, sizeof(a));
	a.sin_addr.s_addr = inet_addr(ip);
	a.sin_port = htons(port);
	a.sin_family = AF_INET;
	if (bind(fd, (struct sockaddr *) &a, sizeof(a)) < 0)
		err(1, "bind %s", ip);
	if (listen(fd, 128) < 0)
		err(1, "listen");
	return fd;
}
