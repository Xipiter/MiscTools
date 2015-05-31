#ifndef _S1CFLATLINE_H
#define _S1CFLATLINE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <getopt.h>

#if defined(SVR4)
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#endif /* SVR4 */

#define HAVE_inet_aton
#define HAVE_scsi_h
#define HAVE_kd_h
#define _(FOO) FOO
#ifdef HAVE_openpty
#include <libutil.h>
#endif

#define FIFOFILE ".s1cflatlinefifo"
#define LOGFILE "logs/s1cflatline.log"  //these files reside in WORKINGDIR


#if defined(SVR4) && !defined(CDEL)
#if defined(_POSIX_VDISABLE)
#define CDEL _POSIX_VDISABLE
#elif defined(CDISABLE)
#define CDEL CDISABLE
#else /* not _POSIX_VISIBLE && not CDISABLE */
#define CDEL 255
#endif /* not _POSIX_VISIBLE && not CDISABLE */
#endif /* SVR4 && ! CDEL */

#endif /*_S1CFLATLINE_H*/
