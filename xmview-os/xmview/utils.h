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
#ifndef _UTILS_H_
#define _UTILS_H_

char **getparms(struct pcb *pc,long laddr);
void freeparms(char **parms);

#ifdef _VIEWOS_KM
extern int kmviewfd;
#	include <string.h>
#	include <sys/ioctl.h>
#	include <kmview.h>

#	define WORDLEN sizeof(int *)
#	define WORDALIGN(X) (((X) + WORDLEN) & ~(WORDLEN-1))

/* Moves len bytes from address 'addr' in the address space of the process
 * whose pid is 'pid' to local address '_laddr'. */
static inline int umoven(struct pcb *pc, long addr, int len, void *_laddr) {
	struct kmview_ioctl_data data={pc->kmpid,addr,len,_laddr};
	return (ioctl(kmviewfd,KMVIEW_READDATA,&data) < 0);
}

/* Moves bytes from address 'addr' in the address space of the process whose
 * pid is 'pid' to local address '_laddr', until it doesn't find a '\0' */
static inline int umovestr(struct pcb *pc, long addr, int len, void *_laddr) {
	struct kmview_ioctl_data data={pc->kmpid,addr,len,_laddr};
	return (ioctl(kmviewfd,KMVIEW_READSTRINGDATA,&data) < 0);
}

/* Moves len bytes from local address '_laddr' in our address space to address
 * 'addr' in the address space of the process whose pid is 'pid'. */
static inline int ustoren(struct pcb *pc, long addr, int len, void *_laddr) {
	struct kmview_ioctl_data data={pc->kmpid,addr,len,_laddr};
	return (ioctl(kmviewfd,KMVIEW_WRITEDATA,&data) < 0);
}

/* Moves bytes from local address '_laddr' in our address space to address
 * 'addr' in the address space of the process whose pid is 'pid', until it
 * doesn't find a '\0' */
static inline int ustorestr(struct pcb *pc, long addr, int len, void *_laddr) {
	struct kmview_ioctl_data data={pc->kmpid,addr,
		strnlen((char *)_laddr,len)+1, /* +1: final '\0' must be included */
		_laddr}; 
	return (ioctl(kmviewfd,KMVIEW_WRITEDATA,&data) < 0);
}

static inline int addfd(struct pcb *pc, int fd) {
	struct kmview_fd kmfd={pc->kmpid,fd};
	//printk("FD ADD pid %d fd %d\n",pc->pid,fd);
	return (ioctl(kmviewfd,KMVIEW_ADDFD,&kmfd));
}

static inline int delfd(struct pcb *pc, int fd) {
	struct kmview_fd kmfd={pc->kmpid,fd};
	//printk("FD DEL pid %d fd %d\n",pc->pid,fd);
	return (ioctl(kmviewfd,KMVIEW_DELFD,&kmfd));
}
#endif

#ifdef _VIEWOS_UM
#ifdef _PROCESS_VM_RW
#include <unistd.h>
#include <sys/syscall.h>   
#include <sys/uio.h>

#if !defined(__NR_process_vm_readv)
# if defined(I386)
#  define __NR_process_vm_readv  347
# elif defined(X86_64)
#  define __NR_process_vm_readv  310
# elif defined(POWERPC)
#  define __NR_process_vm_readv  351
# endif
#endif
#if !defined(__NR_process_vm_writev)
# if defined(I386)
#  define __NR_process_vm_writev  348
# elif defined(X86_64)
#  define __NR_process_vm_writev  311
# elif defined(POWERPC)
#  define __NR_process_vm_writev  352
# endif
#endif

#if defined(__NR_process_vm_readv) && !defined(HAVE_PROCESS_VM_READV)
static inline ssize_t process_vm_readv(pid_t pid,
		const struct iovec *lvec,
		unsigned long liovcnt,
		const struct iovec *rvec,
		unsigned long riovcnt,
		unsigned long flags)
{
	  return syscall(__NR_process_vm_readv, (long)pid, lvec, liovcnt, rvec, riovcnt, flags);
}
#endif

#if defined(__NR_process_vm_readv) && !defined(HAVE_PROCESS_VM_READV)
static inline ssize_t process_vm_writev(pid_t pid,
		const struct iovec *lvec,
		unsigned long liovcnt,
		const struct iovec *rvec,
		unsigned long riovcnt,
		unsigned long flags)
{
	  return syscall(__NR_process_vm_writev, (long)pid, lvec, liovcnt, rvec, riovcnt, flags);
}
#endif

#endif

/* Moves len bytes from address 'addr' in the address space of the process
 * whose pid is 'pid' to local address '_laddr'. */
extern int (*umoven) (struct pcb *pc, long addr, int len, void *_laddr);
/* Moves bytes from address 'addr' in the address space of the process whose
 * pid is 'pid' to local address '_laddr', until it doesn't find a '\0' */
extern int (*ustoren) (struct pcb *pc, long addr, int len, void *_laddr);
/* Moves len bytes from local address '_laddr' in our address space to address
 * 'addr' in the address space of the process whose pid is 'pid'. */
extern int (*umovestr) (struct pcb *pc, long addr, int len, void *_laddr);
/* Moves bytes from local address '_laddr' in our address space to address
 * 'addr' in the address space of the process whose pid is 'pid', until it
 * doesn't find a '\0' */
extern int (*ustorestr) (struct pcb *pc, long addr, int len, void *_laddr);

/* ptrace std */
int umoven_std(struct pcb *pc, long addr, int len, void *_laddr);
int umovestr_std(struct pcb *pc, long addr, int len, void *_laddr);
int ustoren_std(struct pcb *pc, long addr, int len, void *_laddr);
int ustorestr_std(struct pcb *pc, long addr, int len, void *_laddr);

/* ptrace multi */
int umoven_multi(struct pcb *pc, long addr, int len, void *_laddr);
int umovestr_multi(struct pcb *pc, long addr, int len, void *_laddr);
int ustoren_multi(struct pcb *pc, long addr, int len, void *_laddr);
int ustorestr_multi(struct pcb *pc, long addr, int len, void *_laddr);

#ifdef _PROCESS_VM_RW
/* process_vm_readv/writew  */
int umoven_process_rw(struct pcb *pc, long addr, int len, void *_laddr);
int umovestr_process_rw(struct pcb *pc, long addr, int len, void *_laddr);
int ustoren_process_rw(struct pcb *pc, long addr, int len, void *_laddr);
int ustorestr_process_rw(struct pcb *pc, long addr, int len, void *_laddr);
#endif

static inline int addfd(struct pcb *pc, int fd) {
	return 0;
}

static inline int delfd(struct pcb *pc, int fd) {
	return 0;
}

#endif

#endif
