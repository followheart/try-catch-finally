/***
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*    Author : Dibyendu Majumdar
*    Email  : dibyendu@mazumdar.demon.co.uk
*/
#ifndef dm1_port_h
#define dm1_port_h

#if defined(WIN32)

#	pragma warning(disable: 4290)

#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <process.h>
#	define snprintf 	_snprintf
#	ifndef ETIMEDOUT
#		define ETIMEDOUT 10060     /* This is the value in winsock.h. */
#	endif
#	ifndef DM1_USE_PTHREAD
#		define DM1_USE_PTHREAD 0
#	endif
	inline int sleep(unsigned int secs)      { Sleep(secs*1000); return 0; }
	inline int msleep(unsigned int millsecs) { Sleep(millsecs); return 0; }

#else

#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	define DM1_USE_PTHREAD 1
	inline int msleep(unsigned int millsecs) { return usleep(millsecs*1000); }

#endif

#if DM1_USE_PTHREAD
#	include <pthread.h>
#endif

#endif
