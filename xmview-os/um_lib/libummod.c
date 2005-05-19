/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   
 *
 *   Copyright 2005 Renzo Davoli University of Bologna - Italy
 *   Modified 2005 Andrea Seraghiti
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
#include <unistd.h>
#include <linux/types.h>
#include <sys/types.h>
#include <linux/dirent.h>
#include <linux/unistd.h>
#include <errno.h>


int getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
{
	return syscall(__NR_getdents, fd, dirp, count);
}

int getdents64(unsigned int fd, struct dirent *dirp, unsigned int count)
{
	return syscall(__NR_getdents64, fd, dirp, count);
}

int fcntl32(int fd, int cmd, long arg)
{
	return syscall(__NR_fcntl, fd, cmd, arg);
}

int fcntl64(int fd, int cmd, long arg)
{
	return syscall(__NR_fcntl64, fd, cmd, arg);
}
 
int _llseek(unsigned int fd, unsigned long offset_high,  unsigned  long
		       offset_low, loff_t *result, unsigned int whence)
{
	return syscall(__NR__llseek, fd, offset_high, offset_low, result, whence);
}

