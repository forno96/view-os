/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   defs.h: interfaces to system call arguments (architecture dependant)
 *           needed for capture_um
 *   
 *   Copyright 2005 Renzo Davoli University of Bologna - Italy
 *   Modified 2005 Mattia Belletti, Ludovico Gardenghi, Andrea Gasparini
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

#ifndef _DEFS_X86_64
#define _DEFS_X86_64

#define _KERNEL_NSIG   64
#define _KERNEL_SIGSET_SIZE _KERNEL_NSIG/8

#define __NR_socketcall	__NR_doesnotexist

#define __NR__newselect __NR_doesnotexist
#define __NR_umount __NR_doesnotexist
#define __NR_stat64 __NR_doesnotexist
#define __NR_lstat64 __NR_doesnotexist
#define __NR_fstat64 __NR_doesnotexist
#undef __NR_chown32
#define __NR_chown32 __NR_doesnotexist
#undef __NR_lchown32
#define __NR_lchown32 __NR_doesnotexist
#undef __NR_fchown32
#define __NR_fchown32 __NR_doesnotexist
#define __NR_fcntl64 __NR_doesnotexist
#define __NR__llseek __NR_doesnotexist
#define __NR_truncate64 __NR_doesnotexist
#define __NR_ftruncate64 __NR_doesnotexist
#define __NR_send __NR_doesnotexist
#define __NR_recv __NR_doesnotexist
#define __NR_statfs64 __NR_doesnotexist
#define __NR_fstatfs64 __NR_doesnotexist
#define __NR_nice __NR_doesnotexist
#define __NR_mmap2 __NR_doesnotexist

/* XXX: should we find a more elegant solution? */
#define wrap_in_statfs64 NULL
#define wrap_in_fstatfs64 NULL

#define wrap_in_stat wrap_in_stat64
#define wrap_in_fstat wrap_in_fstat64

#define __NR_setpgrp __NR_doesnotexist

#define LITTLEENDIAN
#define LONG_LONG(_l,_h) \
	    ((long long)((unsigned long long)(unsigned)(_l) | ((unsigned long long)(_h)<<32)))

#define MAXERR 4096

#endif // _DEFS_X86_64
