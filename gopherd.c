#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pwd.h>
#include <err.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int serve(int);
int tcpbind(const char *, int);
void session(int, const char *);

char* ipport(char *str, int *port)
{
	char *p;

	p = strchr(str, ':');
	if (p != NULL) {
		*p++ = '\0';
		*port = atoi(p);
	}

	return str;
}

static void usage(char *prog)
{
	fprintf(stderr, "Usage: %s <ip>[:<port>] [rootpath] [hostname]\n",
	    prog);
	exit(1);
}

/* main: simple gopher daemon */
int main(int argc, char **argv)
{
	int sfd;
	struct passwd *pw;
	gid_t gid;
	uid_t uid;
	char *root = "/var/gopher";
	int port = 70;
	char *ip;
	char *hostname = NULL;
	struct stat sb;
	struct in_addr ipa;
	struct hostent *he;

	if (argc < 2 || argc > 4)
		usage(argv[0]);

	if (argc == 4)
		hostname = argv[3];
	if (argc >= 3)
		root = argv[2];

	ip = ipport(argv[1], &port);
	if (strcmp(ip, "*") == 0)
		ip = "0.0.0.0";

	if (hostname == NULL) {
		he = gethostbyaddr(&ipa, sizeof(ipa), AF_INET);
		if (he == NULL) {
			warnx("warn: no name associated with %s", ip);
			hostname = ip;
		} else {
			hostname = he->h_name;
			warnx("warn: using hostname %s", hostname);
		}
	}

	openlog(argv[0], 0, LOG_MAIL);
	syslog(LOG_INFO, "started");
	sfd = tcpbind(ip, port);
	pw = getpwnam("daemon");

	if (stat(root, &sb) != 0) {
		warn("stat %s", root);
		usage(argv[0]);
	}
	if (!(S_ISDIR(sb.st_mode) || S_ISLNK(sb.st_mode)))
		errx(1, "%s: not a directory", root);

	if (access(root, R_OK | X_OK) != 0)
		err(1, "access %s", root);

#ifdef __OpenBSD__
	if (unveil(root, "r") != 0)
		err(1, "unveil %s", root);
	if (unveil(NULL, NULL) != 0)
		err(1, "unveil");
	if (chdir(root) != 0)
		err(1, "chdir %s", root);
#else
	if (chroot(root) != 0 || chdir("/") != 0)
		err(1, "chroot %s", root);
#endif
	uid = pw->pw_uid;
	gid = pw->pw_gid;
	setgroups(1, &gid);
	setresgid(gid, gid, gid);
	setresuid(uid, uid, uid);
	if (daemon(0, 0) < 0)
		err(1, "daemon");
#ifdef __OpenBSD__
	if (pledge("stdio rpath inet proc", NULL) < 0)
		err(1, "pledge");
#endif
	for (;;) {
		int fd, status;
		pid_t pid;

		fd = serve(sfd);
		if (fd < 0)
			continue;
		if ((pid = fork()) == 0) {
			alarm(10);	/* session expire time */
			setproctitle("session");
#ifdef __OpenBSD__
			if (pledge("stdio rpath", NULL) < 0)
				err(1, "pledge");
#endif
			session(fd, hostname);
			exit(127);
		}
		close(fd);
		if (pid != -1)
			wait(&status);
	}
	return 0;
}
