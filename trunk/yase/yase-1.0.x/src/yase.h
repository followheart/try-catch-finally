/*** 
*    YASE (Yet Another Search Engine) 
*    Copyright (C) 2000-2002  Dibyendu Majumdar.  
* 
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
*    Website: www.mazumdar.demon.co.uk/yase_index.html 
*/ 
#ifndef yase_h
#define yase_h

#ifdef WIN32
#include "win32config.h"
#else
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/wait.h>
#endif
#ifndef assert
#include <assert.h>
#define CHECK assert
#endif
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <limits.h>
#ifndef WIN32
#include <dirent.h>
#endif

enum {
	BOOL_TRUE = 1,
	BOOL_FALSE = 0
};

typedef unsigned char ys_bool_t;
typedef unsigned char ys_uchar_t;
typedef unsigned char ys_uint8_t;
typedef char ys_int8_t;
typedef unsigned short ys_uint16_t;
typedef short ys_int16_t;
typedef unsigned long ys_uint32_t;
typedef long int ys_int32_t;
// typedef unsigned long long ys_uint64_t;
// typedef long long ys_int64_t;
typedef ys_int32_t ys_intmax_t;
typedef ys_uint32_t ys_uintmax_t;

#define _USE_COMPRESSION 1
#define _USE_GAMMA 1
#define _XML_HTML_SUPPORTED 1
#define _DUMP_MERGE 0
#define _DUMP_CALCWEIGHT 0
#define _DUMP_POSTINGS 0
#define _DUMP_RANKING 0
#define _TEST_FREQ_IN_TREE 0
#define _DUMP_TREE_READ_WRITE 0
#define _NEWSTYLE_DOCWT 1
#define _INLINE_MAX 1

/* All possible candidates for 64 bit values are listed here. Currently
 * uintmax_t is set to 32 bit integer - changing it to 64 bit should
 * work but doesn't. TODO: Fix the bugs.
 */
typedef ys_uintmax_t ys_docnum_t;	
typedef ys_intmax_t ys_filepos_t;
typedef ys_uintmax_t ys_blocknum_t;
typedef ys_uintmax_t ys_doccnt_t;

enum { YS_TERM_LEN = 255 };	/* Maximum length of a word that can
				 * indexed by YASE. 
				 * NOTE: This cannot exceed YS_MAXKEYSIZE
				 * in btree.h
				 */
#define YS_QUERY_MAXTERMS 50

extern int Ys_debug;

#ifndef __GNUC__
#define inline
#define __func__ "unknown"
#endif

#if defined(WIN32)
#include <windows.h>
#define sleep(n) 	Sleep(n*1000)
#define usleep(n)	Sleep(n)
#define snprintf 	_snprintf
#define popen		_popen
#define pclose		_pclose
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
enum { PATH_MAX = 1024 };
#undef inline
#define inline __inline
#endif

#endif
