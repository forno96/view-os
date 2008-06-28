/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   
 *
 *   Copyright 2005 Renzo Davoli University of Bologna - Italy
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2, as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 *   $Id$
 *
 */
#ifndef _UM_VIEW_MODULE_H
#define _UM_VIEW_MODULE_H
#include <sys/syscall.h>
#include <unistd.h>
#include <stdarg.h>
//#include <sys/socket.h>
/* VIRTUAL SYSCALLS */
#define VIRSYS_UMSERVICE 1
#define VIRSYS_MSOCKET 2
#define __NR_msocket VIRSYS_MSOCKET

extern int _umview_version;

typedef long (*sysfun)();
typedef long long epoch_t;
struct treepoch;
struct timestamp {
	epoch_t epoch;
	struct treepoch *treepoch;
};

extern epoch_t tst_matchingepoch(struct timestamp *service_tst);
extern struct timestamp tst_timestamp();
extern epoch_t get_epoch();

extern epoch_t um_setepoch(epoch_t epoch);

typedef epoch_t (*epochfun)();
typedef unsigned char service_t;

typedef unsigned long c_set;

extern int msocket (char *path, int domain, int type, int protocol);

#define MC_USER 1

#define MC_CORECTLCLASS(x) ((x) << 1)
#define MC_CORECTLOPT(x) ((x) << 6)
#define MC_USERCTL(sercode, ctl) (MC_USER | (sercode << 1) | (ctl << 9))

/* To be tested. Bits are fun!  */
#define MC_USERCTL_SERCODE(x) (((x) >> 1) & ((1L << (sizeof(service_t) * 8)) - 1))
#define MC_USERCTL_CTL(x) (((x) >> 9) & ((1L << (sizeof(long) * 8 - 9)) - 1))

#define MC_PROC			MC_CORECTLCLASS(0)
#define MC_MODULE		MC_CORECTLCLASS(1)
#define MC_MOUNT		MC_CORECTLCLASS(2)

#define MC_ADD			MC_CORECTLOPT(0)
#define MC_REM			MC_CORECTLOPT(1)

#define MC_ALLSERVICES	((1 << (sizeof(service_t) * 8)) - 1)

#define MCH_SET(c, set)		*(set) |= (1 << c)
#define MCH_CLR(c, set)		*(set) &= ~(1 << c)
#define MCH_ISSET(c, set)	(*(set) & (1 << c))
#define MCH_ZERO(set)		*(set) = 0;

#define CHECKNOCHECK  0
#define CHECKPATH     1
#define CHECKSOCKET   2
#define CHECKFSTYPE   3
#define CHECKSC 5
#define CHECKBINFMT 6

// for IOCTL mgmt
#define CHECKIOCTLPARMS   0x40000000
#define IOCTLLENMASK      0x07ffffff
#define IOCTL_R           0x10000000
#define IOCTL_W           0x20000000
struct ioctl_len_req {
	int fd;
	int req;
};

#define BINFMT_MODULE_ALLOC 1
#define BINFMT_KEEP_ARG0 2
struct binfmt_req {
	char *path;
	char *interp;
	int flags;
};


struct service {
	char *name;
	service_t code;

	/* handle to service data. It is used by um_service.c to store
	 * dynamic lib handle (see dlopen (3))*/
	void *dlhandle;

	/* Generic notification/callback function. The first parameter identifies
	 * the type of command. The lower 5 bits identify the class, the remaining
	 * ones identify the command.
	 *
	 * Valid classes:
	 *
	 * MC_PROC (process birth/dead)
	 * MC_MODULE (module insertion/removal)
	 * MC_MOUNT (mount/umount)
	 *
	 * Valid commands:
	 *
	 * MC_ADD
	 * MC_REM
	 *
	 * Parameters 2 to n depend on the command type.
	 *
	 * MC_MODULE | MC_ADD:
	 *
	 *
	 * MC_MODULE | MC_REM:
	 *
	 *
	 * MC_PROC | MC_ADD: int umpid, int pumpid, int numprocs
	 * numprocs is the current max number of processes: service implementation can use it
	 * to realloc their internal structures. UMPID is an internal id, *not*
	 * the pid! id is in the range 0,...,numprocs-1 it is never reassigned during
	 * the life of a process, can be used as an index for internal data
	 * pumpid is the similar id for the parent process, -1 if it does not exist
	 *
	 * MC_PROC | MC_REM: int id
	 * is the garbage collection function for the data that addproc may have created
	 *
	 * MC_MOUNT | MC_ADD:
	 *
	 * MC_MOUNT | MC_REM:
	 *
	 */
	long (*ctl)(int, va_list);

	/* Mask of ctl classes for which the module want synthetized
	 * notifications. For example, at module loading time, it may want one
	 * ctl(MC_PROC | MC_ADD) for each currently running process.
	 * (hs stands for history set)
	 */
	c_set ctlhs;

	/* choice function: returns TRUE if this path must be managed by this module
	 * FALSE otherwise.
	 * Nesting modules: returns the epoch of best match (0 if non found).
	 *
	 * checkfun functions has the following args:
	 *  (int type, void *arg) or
	 *  type is defined by CHECK... constants above
	 */
	epochfun checkfun;

	/* proactive management of select/poll system call. The module provides this function
	 * to activate a callback when an event occurs.
	 * it has the followin args:
	 * (void (* cb)(), void *arg, int fd, int events)    
	 * cb: the callback function (if NULL, it means that a previous registration for callback
	 *     must be deleted).
	 * arg: argument passed to the callback function
	 * fd: fd (i.e. sfd, the fd as seen by the service module)
	 * events: as defined in poll(2)
	 */
	sysfun event_subscribe;

	/* the syscall table, the arguments are the same of the "real world" syscalls,*/
	sysfun *syscall;

	/* the socket call table, the arguments are the same of the "real world" syscalls,*/
	sysfun *socket;

  /* the virtual call table, the arguments are the same of the "real world" syscalls,*/
	sysfun *virsc;
};

/* 
 * #define ESCNO_SOCKET is defined 0x40000000 or 0x0
 * depending on the presence of the single socketcall system call
 * or one syscall for each socket call*/
#define ESCNO_VIRSC 0x80000000
#define ESCNO_MASK  0x3fffffff
#define ESCNO_MAP   0xC0000000

extern int _lwip_version;
extern int scmap_scmapsize;
extern int scmap_sockmapsize;
extern int scmap_virscmapsize;

extern int um_mod_getpid(void);
extern int um_mod_umoven(long addr, int len, void *_laddr);
extern int um_mod_umovestr(long addr, int len, void *_laddr);
extern int um_mod_ustoren(long addr, int len, void *_laddr);
extern int um_mod_ustorestr(long addr, int len, void *_laddr);
extern int um_mod_getsyscallno(void);
extern int um_mod_getumpid(void);
extern long* um_mod_getargs(void);
extern struct stat64 *um_mod_getpathstat(void);
char *um_mod_getpath(void);
extern int um_mod_getsyscalltype(int escno);
int um_mod_event_subscribe(void (* cb)(), void *arg, int fd, int how);
int um_mod_nrsyscalls(void);

extern int uscno(int scno);
extern int add_service(struct service *);
// XXX: should modules have visibility of this function?
// extern void service_ctl(unsigned long type, service_t code, int skip, ...);
extern void service_userctl(unsigned long type, service_t sender, service_t recipient, ...);

extern void *openmodule(const char *modname, int flag);

extern int fprint2(const char *fmt, ...);
extern int vfprint2(const char *fmt, va_list ap);

#define __NR_doesnotexist -1

#if defined(__x86_64__)
#define __NR_socketcall __NR_doesnotexist
#define __NR__newselect __NR_doesnotexist
#define __NR_umount __NR_doesnotexist
#define __NR_stat64 __NR_stat
#define __NR_lstat64 __NR_lstat
#define __NR_fstat64 __NR_fstat
#define __NR_statfs64 __NR_statfs
#define __NR_fstatfs64 __NR_fstatfs
//#define __NR_chown32 __NR_chown
//#define __NR_lchown32 __NR_lchown
//#define __NR_fchown32 __NR_fchown
#define __NR_fcntl64 __NR_fcntl
#define __NR__llseek __NR_doesnotexist
#define __NR_send __NR_doesnotexist
#define __NR_recv __NR_doesnotexist
#endif

#if (__NR_socketcall != __NR_doesnotexist)
#define __NR_socket     SYS_SOCKET
#define __NR_bind       SYS_BIND
#define __NR_connect    SYS_CONNECT
#define __NR_listen     SYS_LISTEN
#define __NR_accept     SYS_ACCEPT
#define __NR_getsockname        SYS_GETSOCKNAME
#define __NR_getpeername        SYS_GETPEERNAME
#define __NR_socketpair SYS_SOCKETPAIR
#define __NR_send       SYS_SEND
#define __NR_recv       SYS_RECV
#define __NR_sendto     SYS_SENDTO
#define __NR_recvfrom   SYS_RECVFROM
#define __NR_shutdown   SYS_SHUTDOWN
#define __NR_setsockopt SYS_SETSOCKOPT
#define __NR_getsockopt SYS_GETSOCKOPT
#define __NR_sendmsg    SYS_SENDMSG
#define __NR_recvmsg    SYS_RECVMSG
#define ESCNO_SOCKET  0x40000000
#else
#define ESCNO_SOCKET  0x00000000
#endif

#define __NR_msocket	  VIRSYS_MSOCKET

#define INTERNAL_MAKE_NAME(a, b) a ## b
#define MAKE_NAME(a, b) INTERNAL_MAKE_NAME(a, b)


/* GEN stands for "generic" */
#define GENSERVICESYSCALL(s, scno, sfun, type) ((s).syscall[uscno(MAKE_NAME(__NR_, scno))] = (type) (sfun))
#define GETSERVICESYSCALL(s, scno) ((s).syscall[uscno(MAKE_NAME(__NR_, scno))])

#if (__NR_socketcall == __NR_doesnotexist)
#	define GENSERVICESOCKET(s, scno, sfun, type) ((s).syscall[uscno(MAKE_NAME(__NR_, scno))] = (type) (sfun))
#	define GETSERVICESOCKET(s, scno) ((s).syscall[uscno(MAKE_NAME(__NR_, scno))])
#else
#	define GENSERVICESOCKET(s, scno, sfun, type) ((s).socket[MAKE_NAME(__NR_, scno)] = (type) (sfun))
#	define GETSERVICESOCKET(s, scno) ((s).socket[MAKE_NAME(__NR_, scno)])
#endif

#define SERVICESYSCALL(s, scno, sfun) GENSERVICESYSCALL(s, scno, sfun, sysfun)
#define SERVICESOCKET(s, scno, sfun) GENSERVICESOCKET(s, scno, sfun, sysfun)

#define SERVICEVIRSYSCALL(s, scno, sfun) ((s).virsc[MAKE_NAME(__NR_, scno)] = (sysfun) (sfun))

#endif
