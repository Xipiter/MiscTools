/*
  S1C Flatline.
  	Yet another nifty S1C utility.
	Miami. Joeboy. Quickstudy.
	
	Before you lambaste me for how disgusting the code below is, I must remind you that 
		I CANT CODE!!!!!
	sa7ori@tasam.com 2003
*/

#include "s1cflatline.h"
/*PROTOTYPING*/ //whattacool word.
void done(void);
void fail(void);
void fixtty(void);
void getmaster(void);
void getslave(void);
void doinput(void);
void dooutput(void);
void doshell(const char*);

char	*shell;
FILE	*fscript;
int	master;
int	slave;
int	child;
int	subchild;
char	*fname;

struct	termios tt;
struct	winsize win;
int	lb;
int	l;
#if !defined(SVR4)
#ifndef HAVE_openpty
char	line[] = "/dev/ptyXX";
#endif
#endif /* !SVR4 */
int	aflg;
int	uflg;
int 	lflg;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern int optind;
	unsigned short newmode;
	int ch, mk;
	void finish();
	char *getenv();
	char *command = NULL;
	char *tmp = NULL;
	char *logfile = NULL;
	while ((ch = getopt(argc, argv, "aul::h?")) != EOF)
		switch((char)ch) {
		case 'a':
			aflg++;
			break;
		case 'u':
		        uflg++;
			break;
                case 'l':
			lflg++;
			logfile = strtok(optarg, ":");
                        break;
		case 'h':
		case '?':
		default:
                        fprintf(stderr, _("\n*** S1C Flatline ***\nYet another nifty s1c utility...\n"));
                        fprintf(stderr, _("sa7ori@tasam.com\n"));
			fprintf(stderr, _("\nUsage: [-u] [-a] [-l]"));
			fprintf(stderr, _("\n\t-u sets uuencode 'smart mode'."));
			fprintf(stderr, _("\n\t-a sets log write mode to APPEND"));		
			fprintf(stderr, _("\n\t-l turn on logging (default:./logs/s1cflatline.log)\n\n"));	
			exit(1);
		}
	argc -= optind;
	argv += optind;
	if (lflg) {
		printf("\nLogging enabled...\n"); fname=LOGFILE; 
	        if ((fscript = fopen(fname, aflg ? "a" : "w")) == NULL) {
                	perror(fname);
                	fail();
		}
        } else {printf("\nLogging not enabled...'exit' to quit.\n");}

	printf("\t\t\n \"He slotted some ice, connected the");
	printf("\t\t\n construct, and jacked in.");
	printf("\t\t\n It was exactly the sensation of");
	printf("\t\t\n someone reading over his shoulder\"\n\n");

	shell = getenv("SHELL");
	if (shell == NULL)
		shell = "/bin/sh";
	getmaster();
	fixtty();
	(void) signal(SIGCHLD, finish);
	child = fork();
	if (child < 0) {
		perror("fork");
		fail();
	}
	if (child == 0) { //if inside the child
		subchild = child = fork(); 
		if (child < 0) {
			perror("fork");
			fail();
		}
		if (child)
			dooutput();
		else 
			doshell(command); 
	}
	doinput();
	return 0;
}

void
doinput()
{
	int retval, stdinfd, n;
	register int cc, stdincc;
	char ibuf[BUFSIZ];
	fd_set rfds;
	if (lflg) 
		(void) fclose(fscript);
#ifdef HAVE_openpty
        (void) close(slave);
#endif
        stdinfd = open(FIFOFILE, O_NONBLOCK);
        FD_ZERO(&rfds);
        FD_SET(stdinfd, &rfds);
        FD_SET(0, &rfds);
	n = stdinfd + 1; cc=1;
	while (cc > 0) {		
		FD_ZERO(&rfds);
		FD_SET(stdinfd, &rfds);
		FD_SET(0, &rfds);
		retval = select(n, &rfds, NULL, NULL, 0);
        	if(FD_ISSET(stdinfd, &rfds)) {
			stdincc=read(stdinfd, ibuf, BUFSIZ);
			(void) write(master, ibuf, stdincc);	
		}
        	if(FD_ISSET(0, &rfds)) {
                	cc=read(0, ibuf, BUFSIZ);  
                        (void) write(master, ibuf, cc);
        	}
	}
	done();
}

#include <sys/wait.h>

void
finish()
{
#if defined(SVR4)
	int status;
#else /* !SVR4 */
	union wait status;
#endif /* !SVR4 */
	register int pid;
	register int die = 0;

	while ((pid = wait3((int *)&status, WNOHANG, 0)) > 0)
		if (pid == child)
			die = 1;
	if (die){
		done();
	}
}

struct linebuf {
    char str[BUFSIZ + 1]; /* + 1 for an additional NULL character.*/
    int len;
};


void
check_line (const char *line)
{
    static int uuencode_mode = 0;
    static FILE *uudecode;
    chdir("files");
    if (uuencode_mode == 1) {
	fprintf(uudecode, "%s", line);
	if (strcmp(line, "end\n") == 0) {
	    pclose(uudecode);
	    uuencode_mode = 0;
	}
    } else {
	int dummy; char dummy2[BUFSIZ];
	if (sscanf(line, "begin %o %s", &dummy, dummy2) == 2) {
	    /* 
	     * uuencode line found! 
	     */
	    uudecode = popen("uudecode", "w");
	    fprintf(uudecode, "%s", line);
	    uuencode_mode = 1;
	}
    }
    chdir("..");
}

void
check_output(const char *str, int len)
{
    static struct linebuf lbuf = {"", 0};
    int i;

    for (i = 0; i < len; i++) {
	if (lbuf.len < BUFSIZ) {
	    lbuf.str[lbuf.len] = str[i];
	    if (lbuf.str[lbuf.len] == '\r') {
		lbuf.str[lbuf.len] = '\n';
	    }
	    lbuf.len++;
	    if (lbuf.str[lbuf.len - 1] == '\n') {
		if (lbuf.len > 1) { /* skip a blank line. */
		    lbuf.str[lbuf.len] = '\0';
		    check_line(lbuf.str);
		}
		lbuf.len = 0;
	    }
	} else {/* buffer overflow */
	    lbuf.len = 0;
	}
    }
}

void
dooutput()
{
	int cc;
	char obuf[BUFSIZ];

	setbuf(stdout, NULL);
	(void) close(0);
#ifdef HAVE_openpty
	(void) close(slave);
#endif
	for (;;) {
		cc = read(master, obuf, BUFSIZ);
		if (cc <= 0)
			break;
		if (uflg)
		    check_output(obuf, cc);
		(void) write(1, obuf, cc);
		if (lflg) 
			(void) fwrite(obuf, 1, cc, fscript);
	}
	done();
}

void
doshell(const char* command)
{
	getslave();
	(void) close(master);
	if (lflg)
		(void) fclose(fscript);
	(void) dup2(slave, 0);
	(void) dup2(slave, 1);
	(void) dup2(slave, 2);
	(void) close(slave);

	if (!command) {
		execl(shell, strrchr(shell, '/') + 1, "-i", 0);
	} else {
		execl(shell, strrchr(shell, '/') + 1, "-c", command, 0);	
	}
	perror(shell);
	fail();
}

void
fixtty()
{
	struct termios rtt;

	rtt = tt;
#if defined(SVR4)
	rtt.c_iflag = 0;
	rtt.c_lflag &= ~(ISIG|ICANON|XCASE|ECHO|ECHOE|ECHOK|ECHONL);
	rtt.c_oflag = OPOST;
	rtt.c_cc[VINTR] = CDEL;
	rtt.c_cc[VQUIT] = CDEL;
	rtt.c_cc[VERASE] = CDEL;
	rtt.c_cc[VKILL] = CDEL;
	rtt.c_cc[VEOF] = 1;
	rtt.c_cc[VEOL] = 0;
#else /* !SVR4 */
	cfmakeraw(&rtt);
	rtt.c_lflag &= ~ECHO;
#endif /* !SVR4 */
	(void) tcsetattr(0, TCSAFLUSH, &rtt);
}

void
fail()
{

	(void) kill(0, SIGTERM);
	done();
}

void
done()
{
	if (subchild) { //if in process above subchild
		if (lflg)
			(void) fclose(fscript);
		(void) close(master);
	} else {
		(void) tcsetattr(0, TCSAFLUSH, &tt);
                printf("\n\t\t\"He disconnected the construct.");
                printf("\n\t\t  The presence was gone.\"\n\n\n");

	}
	exit(0);
}

void
getmaster()
{
#if defined(SVR4)
	(void) tcgetattr(0, &tt);
	(void) ioctl(0, TIOCGWINSZ, (char *)&win);
	if ((master = open("/dev/ptmx", O_RDWR)) < 0) {
		perror("open(\"/dev/ptmx\", O_RDWR)");
		fail();
	}
#else /* !SVR4 */
#ifdef HAVE_openpty
	(void) tcgetattr(0, &tt);
	(void) ioctl(0, TIOCGWINSZ, (char *)&win);
	if (openpty(&master, &slave, NULL, &tt, &win) < 0) {
		fprintf(stderr, _("openpty failed\n"));
		fail();
	}
#else
	char *pty, *bank, *cp;
	struct stat stb;

	pty = &line[strlen("/dev/ptyp")];
	for (bank = "pqrs"; *bank; bank++) {
		line[strlen("/dev/pty")] = *bank;
		*pty = '0';
		if (stat(line, &stb) < 0)
			break;
		for (cp = "0123456789abcdef"; *cp; cp++) {
			*pty = *cp;
			master = open(line, O_RDWR);
			if (master >= 0) {
				char *tp = &line[strlen("/dev/")];
				int ok;

				/* verify slave side is usable */
				*tp = 't';
				ok = access(line, R_OK|W_OK) == 0;
				*tp = 'p';
				if (ok) {
					(void) tcgetattr(0, &tt);
				    	(void) ioctl(0, TIOCGWINSZ, 
						(char *)&win);
					return;
				}
				(void) close(master);
			}
		}
	}
	fprintf(stderr, _("Out of pty's\n"));
	fail();
#endif /* not HAVE_openpty */
#endif /* !SVR4 */
}

void
getslave()
{
#if defined(SVR4)
	(void) setsid();
	grantpt( master);
	unlockpt(master);
	if ((slave = open((const char *)ptsname(master), O_RDWR)) < 0) {
		perror("open(fd, O_RDWR)");
		fail();
	}
	if (isastream(slave)) {
		if (ioctl(slave, I_PUSH, "ptem") < 0) {
			perror("ioctl(fd, I_PUSH, ptem)");
			fail();
		}
		if (ioctl(slave, I_PUSH, "ldterm") < 0) {
			perror("ioctl(fd, I_PUSH, ldterm)");
			fail();
		}
#ifndef _HPUX_SOURCE
		if (ioctl(slave, I_PUSH, "ttcompat") < 0) {
			perror("ioctl(fd, I_PUSH, ttcompat)");
			fail();
		}
#endif
		(void) ioctl(0, TIOCGWINSZ, (char *)&win);
	}
#else /* !SVR4 */
#ifndef HAVE_openpty
	line[strlen("/dev/")] = 't';
	slave = open(line, O_RDWR);
	if (slave < 0) {
		perror(line);
		fail();
	}
	(void) tcsetattr(slave, TCSAFLUSH, &tt);
	(void) ioctl(slave, TIOCSWINSZ, (char *)&win);
#endif
	(void) setsid();
	(void) ioctl(slave, TIOCSCTTY, 0);
#endif /* SVR4 */
}
