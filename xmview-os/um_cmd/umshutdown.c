/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   umviewname.c 
 *   uname extension to view-os (umview)
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
 *   $Id: vuname.c 364 2007-06-11 08:56:36Z rd235 $
 *
 */   
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <config.h>
#include <um_lib.h>

void usage()
{
	fprintf(stderr, 
			"Usage: umshutdown [grace]\n"
		  "Shutdown of xmview.\n"
			"\n");
}

void termhandler(int signo)
{
}

main(int argc, char *argv[])
{
	int grace;
	daemon(1,1);
	signal(SIGTERM,termhandler);
	if (argc > 1)
		grace=atoi(argv[1]);
	else
		grace=30;
	um_killall(SIGTERM);
	if (grace>0)
		sleep(grace);
	um_killall(SIGKILL);
}
