/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   ptrace_multi_test.c : Test if this kernel has the ptrace_multi patch
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

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include "ptrace2.h"
#include <asm/unistd.h>
#include <errno.h>
#include <config.h>
#define r_waitpid(p,s,o) (syscall(__NR_wait4,(p),(s),(o),NULL))

/* test thread code. This thread is started only to test 
 * which features are provided by the linux kernel */
static int child(void *arg)
{
  if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0){
    perror("ptrace test_ptracemulti");
  }
  kill(getpid(), SIGSTOP);
  return 0;
}

/* kernel feature test:
 * exit value =1 means that there is ptrace multi support
 * vm_mask and viewos_mask are masks of supported features of
 * PTRACE_SYSVM and PTRACE_VIEWOS tags, respectively*/
unsigned int test_ptracemulti(unsigned int *vm_mask, unsigned int *viewos_mask) {
  int pid, status, rv;
  static char stack[1024];

  if((pid = clone(child, &stack[1020], SIGCHLD, NULL)) < 0){
    perror("clone");
    return 0;
  }
  if((pid = r_waitpid(pid, &status, WUNTRACED)) < 0){
	  perror("Waiting for stop");
	  return 0;
  }
 if (ptrace(PTRACE_MULTI, pid, stack, 0) < 0) 
	  rv=0;
  else
	  rv=1;
  errno=0;
  *vm_mask=ptrace(PTRACE_SYSVM, pid, PTRACE_VM_TEST, 0);
  if (errno != 0)
	  *vm_mask=0;
  errno=0;
  *viewos_mask=ptrace(PTRACE_VIEWOS, pid, PT_VIEWOS_TEST, 0);
  if (errno != 0)
	  *viewos_mask=0;
  ptrace(PTRACE_KILL,pid,0,0);
  if((pid = r_waitpid(pid, &status, WUNTRACED)) < 0){
	  perror("Waiting for stop");
	  return 0;
  }
  return rv;
}
