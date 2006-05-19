/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   um_plusio: io wrappers (second part)
 *   
 *   Copyright 2005 Renzo Davoli University of Bologna - Italy
 *   Modified 2005 Ludovico Gardenghi
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   $Id$
 *
 */   
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <asm/ptrace.h>
#include <asm/unistd.h>
#include <linux/net.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <utime.h>
#include "defs.h"
#include "gdebug.h"
#include "umproc.h"
#include "services.h"
#include "um_services.h"
#include "sctab.h"
#include "scmap.h"
#include "utils.h"

#define umNULL ((int) NULL)

static int filecopy(service_t sercode,const char *from, const char *to)
{
	char buf[BUFSIZ];
	int fdf,fdt;
	int n;
	if ((fdf=service_syscall(sercode,uscno(__NR_open))(from,O_RDONLY,0)) < 0)
		return -errno;
	if ((fdt=open(to,O_CREAT|O_TRUNC|O_WRONLY,0600)) < 0)
		return -errno;
	while ((n=service_syscall(sercode,uscno(__NR_read))(fdf,buf,BUFSIZ)) > 0)
		write (fdt,buf,n);
	service_syscall(sercode,uscno(__NR_close))(fdf);
	fchmod (fdt,0700); /* permissions? */
	close (fdt);
	return 0;
}


#define CHUNKSIZE 16
static char **getparms(struct pcb *pc,long laddr) {
	long *paddr=NULL;
	char **parms;
	int size=0;
	int n=0;
	int i;
	do {
		int rv;
		if (n >= size) {
			size+=CHUNKSIZE;
			paddr=realloc(paddr,size*sizeof(long));
			assert(paddr);
		}
		rv=umoven(pc,laddr,sizeof(char *),&(paddr[n]));
		assert(rv=4);
		laddr+= sizeof(char *);
		n++;
	} while (paddr[n-1] != 0);
	parms=malloc(n*sizeof(char *));
	assert(parms);
	parms[n-1]=NULL;
	for (i=0;i<n-1;i++) {
		char tmparg[PATH_MAX+1];
		tmparg[PATH_MAX]=0;
		umovestr(pc,paddr[i],PATH_MAX,tmparg);
		parms[i]=strdup(tmparg);
	}
	free(paddr);
	return parms;
}

static void freeparms(char **parms)
{
	char **scan=parms;
	while (*scan != 0) {
		free(*scan);
		scan++;
	}
	free(parms);
}

/*
static void printparms(char *what,char **parms)
{
	fprint2("%s\n",what);
	while (*parms != 0) {
		fprint2("--> %s\n",*parms);
		parms++;
	}
}
*/

#define UMBINWRAP (INSTALLDIR "/umbinwrap")
int wrap_in_execve(int sc_number,struct pcb *pc,struct pcb_ext *pcdata,
		service_t sercode,intfun um_syscall)
{
	struct binfmt_req req={(char *)pcdata->path,NULL,0};
	epoch_t nestepoch=um_x_setepoch(0);
	service_t binfmtser;
	um_x_setepoch(nestepoch+1);
	binfmtser=service_check(CHECKBINMFT,&req,0);
	um_x_setepoch(nestepoch);
	if (binfmtser != UM_NONE) {
		char *umbinfmtarg0;
		char sep;
		long largv=getargn(1,pc);
		long larg0;
		char oldarg0[PATH_MAX+1];
		int rv;
		int filenamelen;
		int arg0len;
		long sp=getsp(pc);
		rv=umoven(pc,largv,sizeof(char *),&(larg0));
		assert(rv);
		if (req.flags & BINFMT_KEEP_ARG0) {
			oldarg0[PATH_MAX]=0;
			umovestr(pc,larg0,PATH_MAX,oldarg0);
		} else
			oldarg0[0]=0;
		for (sep=1;sep<255 && 
				(strchr((char *)pcdata->path,sep)!=NULL ||
				 strchr(req.interp,sep)!=NULL ||
				 strchr(oldarg0,sep)!=NULL);
				sep++)
			;
		if (req.flags & BINFMT_KEEP_ARG0) 
			asprintf(&umbinfmtarg0,"%c%s%c%s%c%s",sep,req.interp,sep,oldarg0,sep,(char *)pcdata->path);
		else 
			asprintf(&umbinfmtarg0,"%c%s%c%s",sep,req.interp,sep,(char *)pcdata->path);
		filenamelen=WORDALIGN(strlen(UMBINWRAP));
		arg0len=WORDALIGN(strlen(umbinfmtarg0));
		pc->retval=0;
		ustorestr(pc,sp-filenamelen,filenamelen,UMBINWRAP);
		putargn(0,sp-filenamelen,pc);
		ustorestr(pc,sp-filenamelen-arg0len,arg0len,umbinfmtarg0);
		larg0=sp-filenamelen-arg0len;
		ustoren(pc,largv,sizeof(char *),&larg0);
		free(umbinfmtarg0);
		if (req.flags & BINFMT_MODULE_ALLOC)
			free(req.interp);
		return SC_CALLONXIT;
	}
	else if (sercode != UM_NONE) {
		pc->retval=ERESTARTSYS;
		if (! isnosys(um_syscall)) {
			long largv=getargn(1,pc);
			long lenv=getargn(2,pc);
			char **argv=getparms(pc,largv);
			char **env=getparms(pc,lenv);
			/*printparms("ARGV",argv);
				printparms("ENV",env);*/
			pc->retval=um_syscall(pcdata->path,argv,env);
			pc->erno=errno;
			freeparms(argv);
			freeparms(env);
		}
		if (pc->retval==ERESTARTSYS){
			char *filename=strdup(um_proc_tmpname());
			//fprint2("wrap_in_execve! %s %p %d\n",(char *)pcdata->path,um_syscall,isnosys(um_syscall));

			/* argv and env should be downloaded then
			 * pc->retval = um_syscall(pcdata->path, argv, env); */
			if ((pc->retval=filecopy(sercode,pcdata->path,filename))>=0) {
				long sp=getsp(pc);
				int filenamelen=WORDALIGN(strlen(filename));
				pc->retval=0;
				pcdata->tmpfile2unlink_n_free=filename;
				ustorestr(pc,sp-filenamelen,filenamelen,filename);
				putargn(0,sp-filenamelen,pc);
				return SC_CALLONXIT;
			} else {
				free(filename);
				pc->erno= -(pc->retval);
				pc->retval= -1;
				return SC_FAKE;
			}
		} else 
			return SC_FAKE;
	} else 
		return STD_BEHAVIOR;
}


int wrap_out_execve(int sc_number,struct pcb *pc,struct pcb_ext *pcdata) 
{ 
	//fprint2("wrap_out_execve %d\n",pc->retval);
	/* The tmp file gets automagically deleted (see sctab.c) */
	if (pc->retval < 0) {
		putrv(pc->retval,pc);
		puterrno(pc->erno,pc);
	}
	return STD_BEHAVIOR;
}
