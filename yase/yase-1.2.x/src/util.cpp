/***
*    YASE (Yet Another Search Engine)
*    Copyright (C) 2000-2003  Dibyendu Majumdar.
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

/* 
 * 6 Mar 2002 - split from xmlparser.c
 * 6 Mar 2002 - moved bits from queryin.c queryout.c and getword.c
 */
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yase.h"
#include "alloc.h"
#include "util.h"

enum { CHUNK_SIZE = 32 };
#define BYTESIZE 8
#define LEFTMOST_BIT(n)	(sizeof(n)*BYTESIZE-1)

int
ys_string_growby(ys_string *sb, size_t size) 
{
	char *newbuf = (char *) realloc( sb->buf, sb->allocated + size );
	if ( newbuf != 0 ) {
		sb->buf = newbuf;
		memset( sb->buf + sb->allocated, 0, size );
		sb->allocated += size;
		return 0;
	}
	fprintf( stderr, "Failed to allocate memory\n" );
	exit( EXIT_FAILURE );
	return -1;
}

int
ys_string_grow( ys_string *sb ) 
{
	return ys_string_growby( sb, CHUNK_SIZE );
}
		
int
ys_string_addch( ys_string *sb, char ch )
{
	if ( sb->used == sb->allocated )
		ys_string_grow( sb );
	sb->buf[ sb->used++ ] = ch;
	return 0;
}

int 
ys_string_addstr( ys_string *sb, const char *str )
{
	size_t len = strlen( str ) + 1;
	if ( sb->used + len >= sb->allocated )
		ys_string_growby( sb, len );
	strcpy( sb->buf + sb->used, str );
	sb->used += len - 1;
	return 0;
}

void
ys_string_init( ys_string *sb )
{
	sb->buf = 0;
	sb->allocated = 0;
	sb->used = 0;
}

void
ys_string_reset( ys_string *sb )
{
	sb->used = 0;
	if ( sb->allocated > 0 )
		sb->buf[0] = 0;
}

void
ys_string_alltrim( ys_string *sb )
{
	int len;
	char *cp;
	
	if (sb->used == 0)
		return;
	cp = sb->buf;
	while (*cp == ' ')
		cp++;
	len = strlen(cp);
	while (len > 0 && cp[len-1] == ' ') {
		cp[len-1] = 0;
		len--;		
	}
	memmove(sb->buf, cp, len);
	sb->used = len;
	ys_string_nullterminate(sb);
}

void 
ys_string_nullterminate( ys_string *sb )
{
	assert( sb->used <= sb->allocated );
	if ( sb->used == sb->allocated )
		ys_string_grow( sb );
	sb->buf[ sb->used ] = 0;
}

void
ys_string_destroy( ys_string *sb )
{
	if ( sb->allocated > 0 )
		free( sb->buf );
	ys_string_init( sb );
}

void *
ys_cstring_init_memory(void)
{
	return ys_new_allocator(0, 100);
}

const char *
ys_cstring_copy(void *a, const char *s)
{
	char *copy;
	ys_allocator_t *string_allocator = (ys_allocator_t *)a;

	if (string_allocator == 0) {
		assert(0);
		return 0;
	}
	if (s != 0) {
		copy = (char *)ys_allocate(string_allocator, strlen(s)+1);
		strcpy(copy, s);
	}
	else {
		copy = (char *)ys_allocate(string_allocator, 1);
		*copy = 0;
	}
	return copy;
}

void
ys_cstring_release_memory(void *a)
{
	ys_allocator_t *string_allocator = (ys_allocator_t *)a;
	if (string_allocator != 0)
		ys_destroy_allocator(string_allocator);
}	

/**
 * Decode an hex character sequence.
 */
int 
ys_url_decode_char(const char *buf) 
{
	int a,b;

	assert(buf[0] == '%' && isxdigit(buf[1]) && isxdigit(buf[2]));

	buf++;	
	a = isdigit(*buf) ? *buf-'0' : tolower(*buf)-'a'+10;
	buf++;
	b = isdigit(*buf) ? *buf-'0' : tolower(*buf)-'a'+10;
	return a * 16 + b;
}

const char *
ys_url_decode_string(
	const char *instr,
	char *outstr,
	int outlen)
{
	register const char *s = instr;
	register char *d = outstr;

	while (*s && outlen > 1) {
		if (s[0] == '%' && isxdigit(s[1]) && isxdigit(s[2])) {
			*d = ys_url_decode_char(s);
			s += 3;
		}
		else {
			*d = *s++;
		}	
		d++;
		outlen--;
	}
	*d = 0;
	return d;
}

/**
 * 15-feb 2002
 * This function encodes HTML unsafe characters into hex sequences
 * which begin with a % sign. If escape_percent flag is set, then the
 * % character is escaped by preceding it with another %.
 */
#define hichar(x)	((((x)/16) > 9) ? (((x)/16)-10+'a') : (((x)/16)+'0'))
#define lochar(x)	((((x)%16) > 9) ? (((x)%16)-10+'a') : (((x)%16)+'0'))
const char *
ys_url_encode_string(
	const char *inp, 		/* input value */
	char *buf, 			/* buffer to hold output */
	size_t len, 			/* length of buffer */
	int flags) 			/* flag to escape % */
{
	register const char *cp = inp;
	register char *bufptr = buf;
	ys_bool_t escape_percent = (flags & YS_URLX_ESCAPE_PERCENT);
	int ch;

	*bufptr = 0;
	while (*cp && len > 1) {
		switch (ch = *cp++) {
		case '<': case '>': case '"':
		case '%': case '{': case '}': case '|':
		case '\\': case '^': case '[': case ']':
		case '`': 
			{
				/* we need at least 3 characters + 1 for zero and
			 	* another if escaping %.
			 	*/
				if (len < 3+(escape_percent?1:0)+1) 	
					goto end;;
				if (escape_percent) {
					*bufptr++ = '%';
					len--;
				}
				*bufptr++ = '%';
				*bufptr++ = hichar(ch);
				*bufptr++ = lochar(ch);
				len -= 3;
				break;
			}
		default: 
			{
				if (ch == ' ' && (flags & YS_URLX_SPACE_TO_PLUS)) {
					ch = '+';
				}
				if (ch == ' ' && (flags & YS_URLX_SPACE_TO_HEX)) {
					if (len < 4)
						goto end;
					*bufptr++ = '%';
					*bufptr++ = hichar(ch);
					*bufptr++ = lochar(ch);
					len -= 3;
				}
				else {
					/* we need at least 2 characters */
					if (len < 2)
						goto end;
					*bufptr++ = ch;
					len--;
				}
				break;
			}
		}
	}
end:
	assert(len > 0);
	*bufptr = 0;

	return buf;
}

/**
 * Output a number as bits.
 */
void
ys_print_as_bits(FILE *fp, ys_uintmax_t n)
{
	ys_uintmax_t mask;
	int i = LEFTMOST_BIT(n);

	for (mask = (1 << i); i >= 0; i--, mask >>= 1)
		fprintf(fp, "%c", (n & mask) ? '1' : '0');
}

/**
 * Calculate the most significant bit that is turned on.
 */
int
ys_most_significant_bit(ys_uintmax_t n)
{
	ys_uintmax_t mask;
	int i = LEFTMOST_BIT(n);

	for (mask = (1 << i); i >= 0; i--, mask >>= 1)
		if (n & mask) return i;
	return -1;
}

#ifndef WIN32

static void
tvsub(struct timeval *tdiff, 
      const struct timeval *t1, 
      const struct timeval *t0)
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

double
ys_calculate_elapsed_time(const struct timeval *t0, 
	  const struct timeval *t1)
{
	struct timeval td;
	double s;

	tvsub(&td, t1, t0);
	s = td.tv_sec + (td.tv_usec / 1000000.);
	return s;
}

#else

void
gettimeofday(
	struct timeval *val,
	struct timezone *zone)
{
	val->tv_sec = (long) GetTickCount();
}

static void
tvsub(
	struct timeval *tdiff, 
	const struct timeval *t1, 
	const struct timeval *t0)
{
	if ((DWORD)t1->tv_sec < (DWORD)t0->tv_sec)
		tdiff->tv_sec = (long)((~(DWORD)0 - (DWORD)t0->tv_sec) + (DWORD)t1->tv_sec);
	else
		tdiff->tv_sec = (long)((DWORD)t1->tv_sec - (DWORD)t0->tv_sec);
}

double
ys_calculate_elapsed_time(
	const struct timeval *t0, 
	const struct timeval *t1)
{
	struct timeval td;
	double s;

	tvsub(&td, t1, t0);
	s = (DWORD)td.tv_sec / 1000.;
	return s;
}

#endif

#ifdef WIN32
DIR *
opendir(const char *pathname)
{
	DIR *dir;
	char mypath[PATH_MAX];
	const char *lastchar;

	if (pathname == 0 || *pathname == 0)
		return 0;

	lastchar = pathname+strlen(pathname)-1;
	if (*lastchar == '/' || *lastchar == '\\')
		snprintf(mypath, sizeof mypath,	"%s*", pathname);
	else
		snprintf(mypath, sizeof mypath,	"%s/*", pathname);

	dir = (DIR *) calloc(1, sizeof(DIR));
	if (0 == dir)
		return 0;

	dir->handle = FindFirstFile(mypath, &dir->find_data);

	if (dir->handle == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Unable to open directory '%s': Windows Error code %d\n", 
			pathname, GetLastError());
		free(dir);
		return 0;
	}
	
	dir->is_first = 1;
	return dir;
}

struct dirent *
readdir(DIR *dir)
{
	if (!dir->is_first) {
		if (!FindNextFile(dir->handle, &dir->find_data))
			return 0;
	}
	dir->is_first = 0;
	dir->entry.d_name = dir->find_data.cFileName;
	return &dir->entry;
}

int 
closedir(DIR *dir)
{
	FindClose(dir->handle);
	free(dir);
	return 0;
}

#endif
