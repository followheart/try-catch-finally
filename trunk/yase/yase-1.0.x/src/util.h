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

#ifndef UTIL_H
#define UTIL_H

#define YS_ROUND_TO_MULTIPLE_OF(x,y)      ((((x)+((y)-1))/(y))*(y))
#define YS_DIVIDE_AND_ROUNDUP(x,y)	  (((x)+((y)-1))/(y))

typedef struct ys_string {
	char *buf;
	size_t allocated;
	size_t used;
} ys_string;

extern int
ys_string_addch( ys_string *sb, char ch );

extern int 
ys_string_addstr( ys_string *sb, const char *str );

extern void
ys_string_init( ys_string *sb );

extern void
ys_string_reset( ys_string *sb );

extern void 
ys_string_nullterminate( ys_string *sb );

extern void
ys_string_alltrim( ys_string *sb );

extern void
ys_string_destroy( ys_string *sb );

#define ys_string_length(sb) ((sb)->used)

extern void *
ys_cstring_init_memory(void);

extern const char *
ys_cstring_copy(void *handle, const char *s);

extern void
ys_cstring_release_memory(void *handle);

extern int
ys_url_decode_char(
	const char *buf);

extern const char *
ys_url_decode_string(
	const char *instr,
	char *outstr,
	int outlen);

enum {
	YS_URLX_ESCAPE_PERCENT = 1,
	YS_URLX_SPACE_TO_PLUS = 2,
	YS_URLX_SPACE_TO_HEX = 4
};

extern const char *
ys_url_encode_string(
	const char *inp, 		/* input value */
	char *buf, 			/* buffer to hold output */
	size_t len, 			/* length of buffer */
	int flags); 			/* flags */

extern void 
ys_print_as_bits(FILE *fp, ys_uintmax_t n);

extern int 
ys_most_significant_bit(ys_uintmax_t n);

extern double
ys_calculate_elapsed_time(const struct timeval *t0, 
	  const struct timeval *t1);

#ifdef WIN32

struct dirent {
	const char *d_name;
};

typedef struct {
	HANDLE handle;
	WIN32_FIND_DATA find_data;
	int is_first;
	struct dirent entry;
} DIR;

extern DIR *opendir(const char *pathname);
extern struct dirent *readdir(DIR *dir);
extern int closedir(DIR *dir);

struct timezone;

extern void gettimeofday(struct timeval *val, struct timezone *zone);

#endif

#endif
