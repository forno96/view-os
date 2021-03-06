/*   This is part of ViewOS
 *   umview-kmview -- A Process with a View
 *
 *   canonicalize.c: recursively canonicalize filenames
 *   
 *   Copyright 2009 Renzo Davoli University of Bologna - Italy
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
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <alloca.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "services.h"
#include "sctab.h"
 
#define PERMIT_NONEXISTENT_LEAF
#define DOTDOT 1
#define ROOT 2

/* canonstruct: this struct contains the values that must be shared during the
	 whole recursive scan:
	 .xpc: opaque for um_x_lstat64, um_xaccess, um_readlink, um_getroot
	 .ebuf: source for the relative to absolute path translation.
	   it is allocated on the stack.
		 if the path to translate begins by '/':
		   it contains the root dir followed by the path to translate, 
		 otherwise
		   it contains the current working dir followed by the path to translate
	 .start, .end: pointers on ebuf, the boundaries of the current component
	 .resolved: the user provided buffer where the result must be stored
	 .rootlen: the len of the root component (it is not possible to generate
	   shorter pathnames to force the root cage)
	 .num_links: counter of symlink to avoid infinite loops (ELOOP)
	 .statbuf: lstat64 of the last component (of the file at the end)
	 .dontfollowlonks: flag, if the entire path is a link do not follow it,
	   this flag is for l-system calls like lstat, lchmod, lchown etc...
*/
	        
struct canonstruct {
	void *xpc;
	char *ebuf;
	char *start;
	char *end;
	char *resolved;
	short rootlen;
	short num_links;
	struct stat64 *statbuf;
	int dontfollowlink;
};

/* recursive construction of canonical absolute form of a filename.
	 This function gets called recursively for each component of the resolved path.
	 dest is pointer to a char of the "resolved" string (the first char
	 of the new component).
   return value: 0 successful canonicalize
                 DOTDOT (..) return to the previous level
                 ROOT, canonicalize stepped onto an absolute symlink
                       the translation process must return back to the root 
											 (or chroot rootcage) 
								 -1 error
 */

static int rec_realpath(struct canonstruct *cdata, char *dest)
{
	char *newdest;
	int recoutput=ROOT; /* force the first lstat */
	/* LOOP (***) This loop manages '.'
		 '..' (DOTDOT) from an inner call
		 ROOT if this is the root dir layer */
	while (1) {
		*dest=0;
		/*printk("looprealpath %s -> %s\n",cdata->ebuf,cdata->resolved);*/
		/* delete multiple slashes / */
		while (*cdata->start == '/')
			cdata->start++;
		/* find the next component */
		for (cdata->end = cdata->start; *cdata->end && *cdata->end != '/'; ++cdata->end)
			;
		{ /* scope of lastlen, a smart compiler should not save this
				 on the stack */
			register int lastlen=cdata->end-cdata->start;
			/* '.': continue with the next component of the path, forget this */
			if (lastlen == 1 && cdata->start[0] == '.') {
				cdata->start=cdata->end;
				continue; /* CONTINUE: NEXT ITERATION OF THE LOOP (***) */
			}
			/* '..' */
			if (lastlen == 2 && cdata->start[0] == '.' && cdata->start[1] == '.') {
				cdata->start=cdata->end;
				/* return DOTDOT only if this does not goes outside the current root */
				if (dest > cdata->resolved+cdata->rootlen)
					return DOTDOT;
				else
					continue; /* CONTINUE: NEXT ITERATION OF THE LOOP (***) */
			}
			if (recoutput != 0) /* ROOT or DOTDOT */
				um_x_lstat64(cdata->resolved,cdata->statbuf,cdata->xpc,recoutput==DOTDOT);
			/* nothing more to do */
			if (lastlen == 0) 
				return 0;
			/* overflow check */
			if (dest + lastlen > cdata->resolved + PATH_MAX) {
				um_set_errno(cdata->xpc,ENAMETOOLONG);
				return -1;
			}
			/* add the new component */
			newdest=dest;
			if (newdest[-1] != '/')
				*newdest++='/';
			newdest=mempcpy(newdest,cdata->start,lastlen);
			*newdest=0;
		}
		/* does the file exist? */
		if (um_x_lstat64(cdata->resolved,cdata->statbuf,cdata->xpc,0) < 0) {
			cdata->statbuf->st_mode=0;
#ifdef PERMIT_NONEXISTENT_LEAF
			if (errno != ENOENT || *cdata->end == '/') {
				um_set_errno(cdata->xpc,ENOENT);
				return -1;
			} else
				return 0;
#else
			um_set_errno(cdata->xpc,ENOENT);
			return -1;
#endif
		}
		/* Symlink case */
		if (S_ISLNK(cdata->statbuf->st_mode) &&
				((*cdata->end == '/') || !cdata->dontfollowlink))
		{
			/* root dir must be already canonicalized.
				 symlinks navigating inside the root link are errors */
			if (dest <= cdata->resolved+cdata->rootlen) {
				um_set_errno(cdata->xpc,ENOENT);
				return -1;
			} else
			{
				char buf[PATH_MAX];
				int len,n;
				/* test for symlink loops */
				if (++cdata->num_links > MAXSYMLINKS) {
					um_set_errno(cdata->xpc,ELOOP);
					return -1;
				}
				/* read the link */
				n = um_x_readlink(cdata->resolved, buf, PATH_MAX-1, cdata->xpc);
				if (n<0)  {
					um_set_errno(cdata->xpc,errno);
					return -1;
				}
				buf[n]=0;
				/* overflow check */
				len=strlen(cdata->end);
				if (n+len >= PATH_MAX) {
					um_set_errno(cdata->xpc,ENAMETOOLONG);
					return -1;
				}
				/* append symlink and remaining part of the path,
					 the latter part is moved inside ebuf itself */
				memmove(cdata->ebuf+n,cdata->end,len+1);
				cdata->end = memcpy(cdata->ebuf,buf,n);
				/* if the symlink is absolute the scan must return
					 back to the current root otherwise from the
				   same dir of the symlink */
				if (*buf == '/') {
					cdata->start=cdata->ebuf;
					return ROOT;
				} else {
					cdata->start=cdata->end;
					continue; /* CONTINUE: NEXT ITERATION OF THE LOOP (***) */
				}
			}
		}
		/* consistency checks on dirs:
			 all the components of the path but the last one must be
			 directories and must have 'x' permission */
		if (*cdata->end == '/') {
		 	if (!S_ISDIR(cdata->statbuf->st_mode)) {
				um_set_errno(cdata->xpc,ENOTDIR);
				return -1;
			} else if (um_x_access(cdata->resolved,X_OK,cdata->xpc,cdata->statbuf) < 0) {
				um_set_errno(cdata->xpc,errno);
				return -1;
			}
		}
		/* okay: recursive call for the next component */
		cdata->start=cdata->end;
		switch(recoutput=rec_realpath(cdata,newdest)) {
			/* success. close recursion */
			case 0 : return 0;
			/* DOTDOT: cycle at this layer */
			case DOTDOT: 
							 continue; /* CONTINUE: NEXT ITERATION OF THE LOOP (***) */
			/* ROOT: close recursive calls up the root */
			case ROOT: 
							 if (dest > cdata->resolved+cdata->rootlen) 
								 return ROOT;
							 else
								 continue; /* CONTINUE: NEXT ITERATION OF THE LOOP (***) */
			/* Error */
			default: return -1;
		}
	}
}

static inline void cancel_trailing_slash(char *path,int len)
{
	if (len > 1 && path[len-1] == '/')
		path[len-1]=0;
}

/* realpath: 
name: path to be canonicalized,
root: current root (chroot), must already be in canonical form
cwd: current working directory, must already be in canonical form
resolved: a buffer of PATH_MAX chars for the result
return resolved or NULL on failures.
errno is set consistently */
char *um_realpath(const char *name, const char *cwd, char *resolved, 
		struct stat64 *pst, int dontfollowlink, void *xpc)
{
	char *root=um_getroot(xpc);
	struct canonstruct cdata= {
		.ebuf=alloca(PATH_MAX),
		.resolved=resolved,
		.rootlen=strlen(root),
		.statbuf=pst,
		.dontfollowlink=dontfollowlink,
		.xpc=xpc,
		.num_links=0
	};
	/* arg consistency check */
	if (name==NULL) {
		um_set_errno(xpc,EINVAL);
		return NULL;
	}
	if (*name==0) {
		um_set_errno(xpc,ENOENT);
		return NULL;
	}
	/* absolute path: 
	   append 'name' to the current root */
	if (*name=='/') {
		int namelen=strlen(name);
		memcpy(cdata.ebuf,root,cdata.rootlen);
		if (cdata.ebuf[cdata.rootlen-1] != '/') {
			cdata.ebuf[cdata.rootlen]='/';
			cdata.rootlen++;
		}
		/* overflow check */
		if (cdata.rootlen + namelen >= PATH_MAX) {
			um_set_errno(xpc,ENAMETOOLONG);
			return NULL;
		}
		memcpy(cdata.ebuf+cdata.rootlen,name+1,namelen);
		cancel_trailing_slash(cdata.ebuf,cdata.rootlen + namelen - 1);
	} else {
		/* relative path 
		   append 'name' to the cwd */
		/* cwd == NULL (unlikely) means relative filenames forbidden */
		if (__builtin_expect((cwd==NULL),0)) {
			um_set_errno(xpc,EINVAL);
			return NULL;
		} else {
			int namelen=strlen(name);
			int cwdlen=strlen(cwd);
			memcpy(cdata.ebuf,cwd,cwdlen);
			if (cdata.ebuf[cwdlen-1] != '/') {
				cdata.ebuf[cwdlen]='/';
				cwdlen++;
			}
			/* cwd inside the current root:
				 set the immutable part of the path (inside the chroot cage) */
			if (strncmp(cdata.ebuf,root,cdata.rootlen)==0 &&
					(root[cdata.rootlen-1]=='/' || cdata.ebuf[cdata.rootlen]=='/')) {
				if (root[cdata.rootlen-1]!='/')
					cdata.rootlen++;
			} else
				cdata.rootlen=1;
			/* overflow check */
			if (cwdlen + namelen>= PATH_MAX) {
				um_set_errno(xpc,ENAMETOOLONG);
				return NULL;
			}
			memcpy(cdata.ebuf+cwdlen,name,namelen+1);
			cancel_trailing_slash(cdata.ebuf,cwdlen + namelen);
		}
	}
	/* printk("PATH! %s (inside %s)\n",cdata.ebuf,cdata.ebuf+cdata.rootlen);*/
	resolved[0]='/';
	cdata.start=cdata.ebuf+1;
	pst->st_mode=0;
	/* start the recursive canonicalization function */
	if (rec_realpath(&cdata,resolved+1) < 0) {
		/*printk("PATH! %s ERR\n",name);*/
		*resolved=0;
		return NULL;
	} else {
		um_set_errno(xpc,0);
		/*printk("PATH! %s (resolved %s)\n",name,resolved);*/
		return resolved;
	}
}
