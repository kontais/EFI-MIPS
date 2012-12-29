/*
 * main.c - Point-to-Point Protocol main module
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char rcsid[] = "$Id: main.c,v 1.1.1.1 2006/05/30 06:16:54 hhzhou Exp $";
#endif

#include <atk_libtty.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#ifdef _ORG_FREEBSD_
#include <syslog.h>
#else
#include "syslog.h"
#endif
#include <netdb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <setjmp.h>

#include "pppd.h"
#include "magic.h"
#include "fsm.h"
#include "lcp.h"
#include "ipcp.h"
#include "upap.h"
#include "chap.h"
#include "ccp.h"
#include "pathnames.h"
#include "patchlevel.h"


#include "atk_libc.h"

#ifdef CBCP_SUPPORT
#include "cbcp.h"
#endif

#if defined(SUNOS4)
extern char *strerror();
#endif

#ifdef IPX_CHANGE
#include "ipxcp.h"
#endif /* IPX_CHANGE */
#ifdef AT_CHANGE
#include "atcp.h"
#endif

/* interface vars */
char ifname[32];		/* Interface name */
int ifunit;			/* Interface unit number */

char *progname;			/* Name of this program */
char hostname[MAXNAMELEN];	/* Our hostname */
static char pidfilename[MAXPATHLEN];	/* name of pid file */
static char iffilename[MAXPATHLEN];	/* name of if file */
static char default_devnam[MAXPATHLEN];	/* name of default device */
time_t		etime,stime;	/* End and Start time */
int		minutes;	/* connection duration */
static int conn_running;	/* we have a [dis]connector running */

int ttyfd = -1;			/* Serial port file descriptor */
mode_t tty_mode = -1;		/* Original access permissions to tty */
int baud_rate;			/* Actual bits/second for serial device */
int hungup;			/* terminal has been hung up */
int privileged = TRUE;		/* we're running as real uid root */
int need_holdoff;		/* need holdoff period before restarting */
int detached;			/* have detached from terminal */

int phase;			/* where the link is at */
int kill_link;
int open_ccp_flag;

char **script_env;		/* Env. variable values for scripts */
int s_env_nalloc;		/* # words avail at script_env */

u_char outpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for outgoing packet */
u_char inpacket_buf[PPP_MRU+PPP_HDRLEN]; /* buffer for incoming packet */

static jmp_buf	timeout_jmpbuf;
static int	in_timeout_callback = FALSE;

static jmp_buf	ppp_event_jmpbuf;
static int	in_ppp_event_callback = FALSE;

static int	exit_reason = 0;

char *no_ppp_msg = "Sorry - this system lacks PPP kernel support\n";

/* Prototypes for procedures local to this file. */

static void close_tty __P((void));
static void get_input __P((void));
static void calltimeout __P((void));
static struct timeval *timeleft __P((struct timeval *));
static void holdoff_end __P((void *));
static int device_script __P((char *, int, int));
static void pr_log __P((void *, char *, ...));
static void call_monitor __P((void*));

extern	char	*ttyname __P((int));
extern  void   pseudoReadIntr(void);
extern  void pppExit(int ret);
int main __P((int, char *[]));

/*
 * PPP Data Link Layer "protocol" table.
 * One entry per supported protocol.
 * The last entry must be NULL.
 */
struct protent *protocols[] = {
    &lcp_protent,
    &pap_protent,
#ifdef LIBMD_SUPPORT
    &chap_protent,
#endif
#ifdef CBCP_SUPPORT
    &cbcp_protent,
#endif
    &ipcp_protent,
    &ccp_protent,
#ifdef IPX_CHANGE
    &ipxcp_protent,
#endif
#ifdef AT_CHANGE
    &atcp_protent,
#endif
    NULL
};

int
main(argc, argv)
    int argc;
    char *argv[];
{
    int i;
    struct protent *protp;
    char numbuf[16];

    phase = PHASE_INITIALIZE;

    ttyInit(NULL, NULL);

    strcpy(default_devnam, devnam);

    script_env = NULL;

    /* Initialize syslog facilities */
    openlog("pppd", LOG_PID | LOG_NDELAY, LOG_PPP);
    setlogmask(LOG_UPTO(LOG_INFO));

    if (gethostname(hostname, MAXNAMELEN) < 0 ) {
	option_error("Couldn't get hostname: %m");
	die(1);
    }
    hostname[MAXNAMELEN-1] = 0;

    /*
     * Initialize to the standard option set, then parse, in order,
     * the system options file, the user's options file,
     * the tty's options file, and the command line arguments.
     */
    for (i = 0; (protp = protocols[i]) != NULL; ++i)
        (*protp->init)(0);

    progname = *argv;

    if (!options_from_file(_PATH_SYSOPTIONS, !privileged, 0, 1)
	|| !options_from_user())
	exit(1);
    scan_args(argc-1, argv+1);	/* look for tty name on command line */
    if (!options_for_tty()
	|| !parse_args(argc-1, argv+1))
	exit(1);

    if (!ppp_available()) {
	option_error(no_ppp_msg);
	exit(1);
    }

    /*
     * Check that the options given are valid and consistent.
     */
    auth_check_options();
    for (i = 0; (protp = protocols[i]) != NULL; ++i)
	if (protp->check_options != NULL)
	    (*protp->check_options)();

    script_setenv("DEVICE", devnam);
    sprintf(numbuf, "%d", baud_rate);
    script_setenv("SPEED", numbuf);

    /*
     * If the user has specified the default device name explicitly,
     * pretend they hadn't.
     */
    if (!default_device && strcmp(devnam, default_devnam) == 0)
	default_device = 1;
    if (default_device)
	nodetach = 1;

    /*
     * Initialize system-dependent stuff and magic number package.
     */
    sys_init();
    magic_init();
    if (debug)
	setlogmask(LOG_UPTO(LOG_DEBUG));

    syslog(LOG_NOTICE, "pppd %s.%d%s",
	   VERSION, PATCHLEVEL, IMPLEMENTATION);

    /*
     *  Start the ball rolling
     */
    ppp_event(NULL);

    /*
     *  If they want us to wait around, do so.
     */
     
    while( client_wait && phase != PHASE_CONNECTED && exit_reason == 0 ) {
	usleep(2000000);	/* 2 second */
	calltimeout();
    }
    
    return (exit_reason);
}

#define	PPP_EVENT_ENTER() \
	static recurse = 0; \
	if (debug) syslog(LOG_INFO, "%s (%d)", \
	phase == PHASE_INITIALIZE    ? "PHASE_INITIALIZE"    : \
	phase == PHASE_WAIT_FOR_CALL ? "PHASE_WAIT_FOR_CALL" : \
	phase == PHASE_ESTABLISH     ? "PHASE_ESTABLISH"     : \
	phase == PHASE_AUTHENTICATE  ? "PHASE_AUTHENTICATE"  : \
	phase == PHASE_CALLBACK      ? "PHASE_CALLBACK"      : \
	phase == PHASE_NETWORK       ? "PHASE_NETWORK"       : \
	phase == PHASE_CONNECTED     ? "PHASE_CONNECTED"     : \
	phase == PHASE_TERMINATE     ? "PHASE_TERMINATE"     : \
	phase == PHASE_DEAD          ? "PHASE_DEAD"          : \
	phase == PHASE_HOLDOFF       ? "PHASE_HOLDOFF"       : \
	phase == PHASE_DORMANT       ? "PHASE_DORMANT"       : \
	"PHASE_UNKNOWN", recurse++ )

#define PPP_EVENT_EXIT()   recurse--

void
ppp_event(void* dummy)
{

    int fdflags, Bits;

    PPP_EVENT_ENTER();

    switch ( phase ) {

    case PHASE_INITIALIZE:
	need_holdoff = 1;

	/*
	 * Open the serial device and set it up to be the ppp interface.
	 * First we open it in non-blocking mode so we can set the
	 * various termios flags appropriately.  If we aren't dialling
	 * out and we want to use the modem lines, we reopen it later
	 * in order to wait for the carrier detect signal from the modem.
	 */
	while ((ttyfd = open(devnam, O_NONBLOCK | O_RDWR, 0)) < 0) {
	    if (errno != EINTR)
		syslog(LOG_ERR, "Failed to open %s: %m", devnam);
	    if (!persist || errno != EINTR)
		goto fail;
	}

	/* set line speed, flow control, etc.; clear CLOCAL if modem option */
	set_up_tty(ttyfd, 0);

	if ((fdflags = fcntl(ttyfd, F_GETFL)) == -1 ||
	     fcntl(ttyfd, F_SETFL, fdflags & ~O_NONBLOCK) < 0)
	    syslog(LOG_WARNING, "Couldn't reset non-blocking mode on device: %m");

	hungup = 0;
	kill_link = 0;

	phase = PHASE_WAIT_FOR_CALL;
	ppp_event(NULL);	/* recurse */
	break;

    case PHASE_WAIT_FOR_CALL:
	/* reopen tty if necessary to wait for carrier */
	if (modem) {
	    /*
	     *  Get modem control lines and see if we have a carrier.
	     *  If not, see if the phone is ringing.  If it is, tell
	     *  the modem to answer.
	     */
	    if (ioctl( ttyfd, TIOCMODG, &Bits ) < 0) {
		goto fail;
	    } else if (!(Bits & TIOCM_CD)) {
		if (Bits & TIOCM_RI) {
		    if (write(ttyfd, "ATA\r\n", 5) < 0) {
			goto fail;
		    }
		}

		/*
		 *  Set a timer to come back to check on things.
		 */
		TIMEOUT(ppp_event, NULL, 1);
		break;

	    } else {
		/*
		 *  We have a connection - set a timer to monitor the call
		 */
		TIMEOUT(call_monitor, NULL, 2);
	    }
	}

	/*
	 *  We should have someone at the other end so proceed.
	 *  Start by opening the connection and wait for incoming
	 *  events (reply, timeout, etc.).
	 */
	establish_ppp(ttyfd);

	syslog(LOG_INFO, "Using interface ppp%d", ifunit);
	(void) sprintf(ifname, "ppp%d", ifunit);	    
	script_setenv("IFNAME", ifname);
	syslog(LOG_NOTICE, "Connect: %s <--> %s", ifname, devnam);

	stime = time((time_t *) NULL);
	lcp_lowerup(0);
	lcp_open(0);		/* Start protocol */

	phase = PHASE_ESTABLISH;
	ppp_event(NULL);		/* recurse */
	break;

    case PHASE_ESTABLISH:
    case PHASE_AUTHENTICATE:
    case PHASE_CALLBACK:
    case PHASE_NETWORK:
    case PHASE_CONNECTED:
    case PHASE_TERMINATE:
	/*
	 *  Process input
	 */
	if (poll_input()) {
	    get_input();

	    if (kill_link) {
		lcp_close(0, "User request");
		kill_link = 0;
	    }

	    ppp_event(NULL);	/* recurse */
	}

	break;

    case PHASE_DEAD:
	/*
	 * If we may want to bring the link up again, transfer
	 * the ppp unit back to the loopback.  Set the
	 * real serial device back to its normal mode of operation.
	 */
	clean_check();
	disestablish_ppp(ttyfd);

	/*
	 * Run disconnector script, if requested.
	 * XXX we may not be able to do this if the line has hung up!
	 */
	if (disconnector && !hungup) {
	    set_up_tty(ttyfd, 1);
	    if (device_script(disconnector, ttyfd, ttyfd) < 0) {
		syslog(LOG_WARNING, "disconnect script failed");
	    } else {
		syslog(LOG_INFO, "Serial link disconnected.");
	    }
	}

	lcp_lowerdown(0);	/* serial link is no longer available */

fail:
	if (ttyfd >= 0)
	    close_tty();

	if (!persist)
	    die(1);

	if (holdoff > 0 && need_holdoff) {
	    phase = PHASE_HOLDOFF;
	    TIMEOUT(ppp_event, NULL, holdoff);
	} else {
	    phase = PHASE_INITIALIZE;
	    ppp_event(NULL);	/* recurse */
	}
        break;

    case PHASE_HOLDOFF:
    	/*
    	 * Waiting for holdoff timer to put us in PHASE_DORMANT
    	 */
	if (kill_link) {
	    if (!persist)
		die(0);
	    kill_link = 0;
	}
	/*
	 *  Go straight to the initialize phase - no need for dormant.
	 */
	phase = PHASE_INITIALIZE;
	ppp_event(NULL);	/* recurse */
	break;

    case PHASE_DORMANT:
    default:
    	break;
    }

    PPP_EVENT_EXIT();
}

/*
 *  Called through EFI event callback when full packet is ready for processing
 */
void
ppp_packet_event()
{
    /*
     *  Without this useless automatic, the VC++ 6.0 IA32 compiler
     *  does not deallocate the stack correctly!
     */
    void *x = NULL;

    if (setjmp(ppp_event_jmpbuf)) {
	return;
    }

    in_ppp_event_callback = TRUE;
    ppp_event(x);
    in_ppp_event_callback = FALSE;
}

void
call_monitor( void *dummy )
{
    int  Bits;

    /*
     *  If we are waiting for a call, simply return
     */
    if (phase == PHASE_WAIT_FOR_CALL)
	return;

    if (ioctl( ttyfd, TIOCMODG, &Bits ) < 0) {
	syslog(LOG_WARNING, "failed to get modem status");
	return;
    }
	
    if (!(Bits & TIOCM_CD)) {
	syslog(LOG_INFO, "call has hungup");
	hungup = 1;
	lcp_lowerdown(0);	/* serial link is no longer available */
	link_terminated(0);
    }

    if (phase == PHASE_DEAD) {
	ppp_event(NULL);
    } else {
	/*
	 *  Keep monitoriung the call
	 */
	TIMEOUT(call_monitor, NULL, 2);
    }
}

/*
 * holdoff_end - called via a timeout when the holdoff period ends.
 */
static void
holdoff_end(arg)
    void *arg;
{
    phase = PHASE_DORMANT;
}

/*
 * get_input - called when incoming data is available.
 */
static void
get_input()
{
    int len, i;
    u_char *p;
    u_short protocol;
    struct protent *protp;

    p = inpacket_buf;	/* point to beginning of packet buffer */

    len = read_packet(inpacket_buf);
    if (len < 0)
	return;

    if (len == 0) {
	etime = time((time_t *) NULL);
	minutes = (etime-stime)/60;
	syslog(LOG_NOTICE, "Modem hangup, connected for %d minutes", (minutes >1) ? minutes : 1);
	hungup = 1;
	lcp_lowerdown(0);	/* serial link is no longer available */
	link_terminated(0);
	return;
    }

    if (debug /*&& (debugflags & DBG_INPACKET)*/)
	log_packet(p, len, "rcvd ", LOG_DEBUG);

    if (len < PPP_HDRLEN) {
	MAINDEBUG((LOG_INFO, "io(): Received short packet."));
	return;
    }

    p += 2;				/* Skip address and control */
    GETSHORT(protocol, p);
    len -= PPP_HDRLEN;

    /*
     * Toss all non-LCP packets unless LCP is OPEN.
     */
    if (protocol != PPP_LCP && lcp_fsm[0].state != OPENED) {
	MAINDEBUG((LOG_INFO,
		   "get_input: Received non-LCP packet when LCP not open."));
	return;
    }

    /*
     * Until we get past the authentication phase, toss all packets
     * except LCP, LQR and authentication packets.
     */
    if (phase <= PHASE_AUTHENTICATE
	&& !(protocol == PPP_LCP || protocol == PPP_LQR
	     || protocol == PPP_PAP || protocol == PPP_CHAP)) {
	MAINDEBUG((LOG_INFO, "get_input: discarding proto 0x%x in phase %d",
		   protocol, phase));
	return;
    }

    /*
     * Upcall the proper protocol input routine.
     */
    for (i = 0; (protp = protocols[i]) != NULL; ++i) {
	if (protp->protocol == protocol && protp->enabled_flag) {
	    (*protp->input)(0, p, len);
	    return;
	}
        if (protocol == (protp->protocol & ~0x8000) && protp->enabled_flag
	    && protp->datainput != NULL) {
	    (*protp->datainput)(0, p, len);
	    return;
	}
    }

    if (debug)
    	syslog(LOG_WARNING, "Unsupported protocol (0x%x) received", protocol);
    lcp_sprotrej(0, p - PPP_HDRLEN, len + PPP_HDRLEN);
}

/*
 * quit - Clean up state and exit (with an error indication).
 */
void 
quit()
{
    die(1);
}

/*
 * die - like quit, except we can specify an exit status.
 */
void
die(status)
    int status;
{
    StopInterruptTimer();
    StopEventTimer();
    cleanup();
    syslog(LOG_INFO, "Exit.");

    exit_reason = status;
   
    if (in_ppp_event_callback)
        longjmp(ppp_event_jmpbuf,1);
    exit(status);
}

/*
 * cleanup - restore anything which needs to be restored before we exit
 */
/* ARGSUSED */
void
cleanup()
{
    sys_cleanup();

    if (ttyfd >= 0)
	close_tty();

    if (pidfilename[0] != 0 && unlink(pidfilename) < 0 && errno != ENOENT) 
	syslog(LOG_WARNING, "unable to delete pid file: %m");
    pidfilename[0] = 0;
}

/*
 * close_tty - restore the terminal device and close it.
 */
static void
close_tty()
{
    disestablish_ppp(ttyfd);

    /* drop dtr to hang up */
    if (modem) {
	setdtr(ttyfd, FALSE);
	/*
	 * This sleep is in case the serial port has CLOCAL set by default,
	 * and consequently will reassert DTR when we close the device.
	 */
	sleep(1);
    }

    restore_tty(ttyfd);
    close(ttyfd);
    ttyfd = -1;
}

struct	callout {
    struct timeval	c_time;		/* time at which to call routine */
    void		*c_arg;		/* argument to routine */
    void		(*c_func) __P((void *)); /* routine */
    struct		callout *c_next;
};

static struct callout *callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of seconds, NOT hz (as in
 * the kernel).
 */
void
timeout(func, arg, time)
    void (*func) __P((void *));
    void *arg;
    int time;
{
    struct callout *newp, *p, **pp;
    struct timeval tv;
    void *LockId;
  
    MAINDEBUG((LOG_DEBUG, "Timeout %lx:%lx in %d seconds.",
	       (long) func, (long) arg, time));
  
    /*
     * Allocate timeout.
     */
    if ((newp = (struct callout *) malloc(sizeof(struct callout))) == NULL) {
	syslog(LOG_ERR, "Out of memory in timeout()!");
	die(1);
    }
    newp->c_arg = arg;
    newp->c_func = func;
    gettimeofday(&timenow, NULL);
    newp->c_time.tv_sec = timenow.tv_sec + time;
    newp->c_time.tv_usec = timenow.tv_usec;
  
    /*
     * Find correct place and link it in.
     */
    LockId = CalloutLock();
    for (pp = &callout; (p = *pp); pp = &p->c_next)
	if (newp->c_time.tv_sec < p->c_time.tv_sec
	    || (newp->c_time.tv_sec == p->c_time.tv_sec
		&& newp->c_time.tv_usec < p->c_time.tv_sec))
	    break;

    newp->c_next = p;
    *pp = newp;

    if (timeleft(&tv))
	time = tv.tv_usec + (tv.tv_sec * 1000000);
    else
	time *= 1000000;

    CalloutUnlock( LockId );
}

/*
 * untimeout - Unschedule a timeout.
 */
void
untimeout(func, arg)
    void (*func) __P((void *));
    void *arg;
{
    struct callout **copp, *freep;
    void *LockId;
  
    MAINDEBUG((LOG_DEBUG, "Untimeout %lx:%lx.", (long) func, (long) arg));
  
    /*
     * Find first matching timeout and remove it from the list.
     */
    LockId = CalloutLock();
    for (copp = &callout; (freep = *copp); copp = &freep->c_next)
	if (freep->c_func == func && freep->c_arg == arg) {
	    *copp = freep->c_next;
	    (void) free((char *) freep);
	    break;
	}
    CalloutUnlock( LockId );
}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
static 
void
calltimeout()
{
    struct callout *p;

    if (setjmp(timeout_jmpbuf)) {
	return;
    }

    in_timeout_callback = TRUE;
    while (callout != NULL) {
    	void *LockId;
    	LockId = CalloutLock();
	p = callout;

	if (gettimeofday(&timenow, NULL) < 0) {
	    syslog(LOG_ERR, "Failed to get time of day: %m");
	    die(1);
	}
	if (!(p->c_time.tv_sec < timenow.tv_sec
	      || (p->c_time.tv_sec == timenow.tv_sec
		  && p->c_time.tv_usec <= timenow.tv_usec))) {
	    CalloutUnlock( LockId );
	    break;		/* no, it's not time yet */
	}

	callout = p->c_next;
	CalloutUnlock( LockId );

	(*p->c_func)(p->c_arg);

	free((char *) p);
    }
    in_timeout_callback = FALSE;

    /*
     *  Set the next timeout event if needed
     */
    if ( callout ) {
	struct timeval tv;
	int	time;

	if (timeleft(&tv))
	    time = (int)(tv.tv_usec + (tv.tv_sec * 1000000));
	else
	    time = 1;
    }
}

/*
 * timeleft - return the length of time until the next timeout is due.
 */
static struct timeval *
timeleft(tvp)
    struct timeval *tvp;
{
    if (callout == NULL)
	return NULL;

    gettimeofday(&timenow, NULL);
    tvp->tv_sec = callout->c_time.tv_sec - timenow.tv_sec;
    tvp->tv_usec = callout->c_time.tv_usec - timenow.tv_usec;
    if (tvp->tv_usec < 0) {
	tvp->tv_usec += 1000000;
	tvp->tv_sec -= 1;
    }
    if (tvp->tv_sec < 0)
	tvp->tv_sec = tvp->tv_usec = 0;

    return tvp;
}

/*
 * device_script - run a program to connect or disconnect the
 * serial device.
 */
static int
device_script(program, in, out)
    char *program;
    int in, out;
{
    int status;
    status = system( program );
    return (status == 0 ? 0 : -1);
}


/*
 * run-program - execute a program with given arguments,
 * but don't wait for it.
 * If the program can't be executed, logs an error unless
 * must_exist is 0 and the program file doesn't exist.
 */
int
run_program(prog, args, must_exist)
    char *prog;
    char **args;
    int must_exist;
{
    char *cmdline;
    int status = 0;

    cmdline = calloc(1024, 1);
    if (cmdline == 0)
    	return -1;

    while (*args) {
    	strcat( cmdline, *args++ );
    	strcat( cmdline, " ");
    }
//	syslog(LOG_INFO, "running script: %s", cmdline);
//	status = system( cmdline );
    free( cmdline );
    return status;
}

/*
 * log_packet - format a packet and log it.
 */

char line[256];			/* line to be logged accumulated here */
char *linep;

void
log_packet(p, len, prefix, level)
    u_char *p;
    int len;
    char *prefix;
    int level;
{
    strcpy(line, prefix);
    linep = line + strlen(line);
    format_packet(p, len, pr_log, NULL);
    if (linep != line)
	syslog(level, "%s", line);
}

/*
 * format_packet - make a readable representation of a packet,
 * calling `printer(arg, format, ...)' to output it.
 */
void
format_packet(p, len, printer, arg)
    u_char *p;
    int len;
    void (*printer) __P((void *, char *, ...));
    void *arg;
{
    int i, n;
    u_short proto;
    u_char x;
    struct protent *protp;

    if (len >= PPP_HDRLEN && p[0] == PPP_ALLSTATIONS && p[1] == PPP_UI) {
	p += 2;
	GETSHORT(proto, p);
	len -= PPP_HDRLEN;
	for (i = 0; (protp = protocols[i]) != NULL; ++i)
	    if (proto == protp->protocol)
		break;
	if (protp != NULL) {
	    printer(arg, "[%s", protp->name);
	    n = (*protp->printpkt)(p, len, printer, arg);
	    printer(arg, "]");
	    p += n;
	    len -= n;
	} else {
	    printer(arg, "[proto=0x%x]", proto);
	}
    }

    for (; len > 0; --len) {
	GETCHAR(x, p);
	printer(arg, " %.2x", x);
    }
}

static void
pr_log __V((void *arg, char *fmt, ...))
{
    int n;
    va_list pvar;
    char buf[256];

#if __STDC__
    va_start(pvar, fmt);
#else
    void *arg;
    char *fmt;
    va_start(pvar);
    arg = va_arg(pvar, void *);
    fmt = va_arg(pvar, char *);
#endif

    n = vfmtmsg(buf, sizeof(buf), fmt, pvar);
    va_end(pvar);

    if (linep + n + 1 > line + sizeof(line)) {
	syslog(LOG_DEBUG, "%s", line);
	linep = line;
    }
    strcpy(linep, buf);
    linep += n;
}

/*
 * print_string - print a readable representation of a string using
 * printer.
 */
void
print_string(p, len, printer, arg)
    char *p;
    int len;
    void (*printer) __P((void *, char *, ...));
    void *arg;
{
    int c;

    printer(arg, "\"");
    for (; len > 0; --len) {
	c = *p++;
	if (' ' <= c && c <= '~') {
	    if (c == '\\' || c == '"')
		printer(arg, "\\");
	    printer(arg, "%c", c);
	} else {
	    switch (c) {
	    case '\n':
		printer(arg, "\\n");
		break;
	    case '\r':
		printer(arg, "\\r");
		break;
	    case '\t':
		printer(arg, "\\t");
		break;
	    default:
		printer(arg, "\\%.3o", c);
	    }
	}
    }
    printer(arg, "\"");
}

/*
 * novm - log an error message saying we ran out of memory, and die.
 */
void
novm(msg)
    char *msg;
{
    syslog(LOG_ERR, "Virtual memory exhausted allocating %s\n", msg);
    die(1);
}

/*
 * fmtmsg - format a message into a buffer.  Like sprintf except we
 * also specify the length of the output buffer, and we handle
 * %r (recursive format), %m (error message) and %I (IP address) formats.
 * Doesn't do floating-point formats.
 * Returns the number of chars put into buf.
 */
int
fmtmsg __V((char *buf, int buflen, char *fmt, ...))
{
    va_list args;
    int n;

#if __STDC__
    va_start(args, fmt);
#else
    char *buf;
    int buflen;
    char *fmt;
    va_start(args);
    buf = va_arg(args, char *);
    buflen = va_arg(args, int);
    fmt = va_arg(args, char *);
#endif
    n = vfmtmsg(buf, buflen, fmt, args);
    va_end(args);
    return n;
}

/*
 * vfmtmsg - like fmtmsg, takes a va_list instead of a list of args.
 */
#define OUTCHAR(c)	(buflen > 0? (--buflen, *buf++ = (c)): 0)

int
vfmtmsg(buf, buflen, fmt, args)
    char *buf;
    int buflen;
    char *fmt;
    va_list args;
{
    int c, i, n;
    int width, prec, fillch;
    int base, len, neg, quoted;
    unsigned long val = 0;
    char *str, *f, *buf0;
    unsigned char *p;
    char num[32];
    time_t t;
    static char hexchars[] = "0123456789abcdef";

    buf0 = buf;
    --buflen;
    while (buflen > 0) {
	for (f = fmt; *f != '%' && *f != 0; ++f)
	    ;
	if (f > fmt) {
	    len = (int)(f - fmt);
	    if (len > buflen)
		len = buflen;
	    memcpy(buf, fmt, len);
	    buf += len;
	    buflen -= len;
	    fmt = f;
	}
	if (*fmt == 0)
	    break;
	c = *++fmt;
	width = prec = 0;
	fillch = ' ';
	if (c == '0') {
	    fillch = '0';
	    c = *++fmt;
	}
	if (c == '*') {
	    width = va_arg(args, int);
	    c = *++fmt;
	} else {
	    while (isdigit(c)) {
		width = width * 10 + c - '0';
		c = *++fmt;
	    }
	}
	if (c == '.') {
	    c = *++fmt;
	    if (c == '*') {
		prec = va_arg(args, int);
		c = *++fmt;
	    } else {
		while (isdigit(c)) {
		    prec = prec * 10 + c - '0';
		    c = *++fmt;
		}
	    }
	}
	str = 0;
	base = 0;
	neg = 0;
	++fmt;
	switch (c) {
	case 'd':
	    i = va_arg(args, int);
	    if (i < 0) {
		neg = 1;
		val = -i;
	    } else
		val = i;
	    base = 10;
	    break;
	case 'o':
	    val = va_arg(args, unsigned int);
	    base = 8;
	    break;
	case 'x':
	    val = va_arg(args, unsigned int);
	    base = 16;
	    break;
	case 'p':
#ifdef _ORG_FREEBSD_
	    val = (unsigned long) va_arg(args, void *);
#else
	    p = va_arg(args, void *);
#endif
	    val = (unsigned long) (p - (unsigned char*)0);
	    base = 16;
	    neg = 2;
	    break;
	case 's':
	    str = va_arg(args, char *);
	    break;
	case 'c':
	    num[0] = va_arg(args, int);
	    num[1] = 0;
	    str = num;
	    break;
	case 'm':
	    str = strerror(errno);
	    break;
	case 'I':
	    str = ip_ntoa(va_arg(args, u_int32_t));
	    break;
	case 'r':
	    f = va_arg(args, char *);
#ifndef __powerpc__
	    n = vfmtmsg(buf, buflen + 1, f, va_arg(args, va_list));
#else
	    /* On the powerpc, a va_list is an array of 1 structure */
	    n = vfmtmsg(buf, buflen + 1, f, va_arg(args, void *));
#endif
	    buf += n;
	    buflen -= n;
	    continue;
	case 't':
	    time(&t);
	    str = ctime(&t);
	    str += 4;		/* chop off the day name */
	    str[15] = 0;	/* chop off year and newline */
	    break;
	case 'v':		/* "visible" string */
	case 'q':		/* quoted string */
	    quoted = c == 'q';
	    p = va_arg(args, unsigned char *);
	    if (fillch == '0' && prec > 0) {
		n = prec;
	    } else {
		n = (int)strlen((char *)p);
		if (prec > 0 && prec < n)
		    n = prec;
	    }
	    while (n > 0 && buflen > 0) {
		c = *p++;
		--n;
		if (!quoted && c >= 0x80) {
		    OUTCHAR('M');
		    OUTCHAR('-');
		    c -= 0x80;
		}
		if (quoted && (c == '"' || c == '\\'))
		    OUTCHAR('\\');
		if (c < 0x20 || (0x7f <= c && c < 0xa0)) {
		    if (quoted) {
			OUTCHAR('\\');
			switch (c) {
			case '\t':	OUTCHAR('t');	break;
			case '\n':	OUTCHAR('n');	break;
			case '\b':	OUTCHAR('b');	break;
			case '\f':	OUTCHAR('f');	break;
			default:
			    OUTCHAR('x');
			    OUTCHAR(hexchars[c >> 4]);
			    OUTCHAR(hexchars[c & 0xf]);
			}
		    } else {
			if (c == '\t')
			    OUTCHAR(c);
			else {
			    OUTCHAR('^');
			    OUTCHAR(c ^ 0x40);
			}
		    }
		} else
		    OUTCHAR(c);
	    }
	    continue;
	default:
	    *buf++ = '%';
	    if (c != '%')
		--fmt;		/* so %z outputs %z etc. */
	    --buflen;
	    continue;
	}
	if (base != 0) {
	    str = num + sizeof(num);
	    *--str = 0;
	    while (str > num + neg) {
		*--str = hexchars[val % base];
		val = val / base;
		if (--prec <= 0 && val == 0)
		    break;
	    }
	    switch (neg) {
	    case 1:
		*--str = '-';
		break;
	    case 2:
		*--str = 'x';
		*--str = '0';
		break;
	    }
	    len = (int)(num + sizeof(num) - 1 - str);
	} else {
	    len = (int)strlen(str);
	    if (prec > 0 && len > prec)
		len = prec;
	}
	if (width > 0) {
	    if (width > buflen)
		width = buflen;
	    if ((n = width - len) > 0) {
		buflen -= n;
		for (; n > 0; --n)
		    *buf++ = fillch;
	    }
	}
	if (len > buflen)
	    len = buflen;
	memcpy(buf, str, len);
	buf += len;
	buflen -= len;
    }
    *buf = 0;
    return (int)(buf - buf0);
}

/*
 * script_setenv - set an environment variable value to be used
 * for scripts that we run (e.g. ip-up, auth-up, etc.)
 */
void
script_setenv(var, value)
    char *var, *value;
{
    int vl = (int)strlen(var);
    int i;
    char *p, *newstring;

    newstring = (char *) malloc(vl + strlen(value) + 2);
    if (newstring == 0)
	return;
    strcpy(newstring, var);
    newstring[vl] = '=';
    strcpy(newstring+vl+1, value);

    /* check if this variable is already set */
    if (script_env != 0) {
	for (i = 0; (p = script_env[i]) != 0; ++i) {
	    if (strncmp(p, var, vl) == 0 && p[vl] == '=') {
		free(p);
		script_env[i] = newstring;
		return;
	    }
	}
    } else {
	i = 0;
	script_env = (char **) malloc(16 * sizeof(char *));
	if (script_env == 0)
	    return;
	s_env_nalloc = 16;
    }

    /* reallocate script_env with more space if needed */
    if (i + 1 >= s_env_nalloc) {
	int new_n = i + 17;
	char **newenv = (char **) realloc((void *)script_env,
					  new_n * sizeof(char *));
	if (newenv == 0)
	    return;
	script_env = newenv;
	s_env_nalloc = new_n;
    }

    script_env[i] = newstring;
    script_env[i+1] = 0;
}

/*
 * script_unsetenv - remove a variable from the environment
 * for scripts.
 */
void
script_unsetenv(var)
    char *var;
{
    int vl = (int)strlen(var);
    int i;
    char *p;

    if (script_env == 0)
	return;
    for (i = 0; (p = script_env[i]) != 0; ++i) {
	if (strncmp(p, var, vl) == 0 && p[vl] == '=') {
	    free(p);
	    while ((script_env[i] = script_env[i+1]) != 0)
		++i;
	    break;
	}
    }
}
