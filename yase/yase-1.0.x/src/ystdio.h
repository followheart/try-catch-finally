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
/**
 * The purpose of this wrapper is two-fold:
 * a) To make it easy to support 64-bit IO as well as multiple operating
 *    systems.
 * b) Ease of error handling.
 */

#ifndef YSTDIO_H
#define YSTDIO_H

#include "yase.h"

typedef struct ys_file_t ys_file_t;

enum {
	YS_FILE_ABORT_ON_ERROR = 1,
	YS_FILE_STATUS_OK = 2,
	YS_FILE_STATUS_FAILED = 3
};

extern ys_file_t *
ys_file_open( const char *filename, const char *mode, int flags );

extern int
ys_file_close( ys_file_t *file );

extern int 
ys_file_getpos( ys_file_t *file, ys_filepos_t *pos );

extern int 
ys_file_setpos( ys_file_t *file, const ys_filepos_t *pos );

extern int
ys_file_seek( ys_file_t *file, ys_filepos_t offset, int whence );

extern int
ys_file_printf( ys_file_t *file, const char *format, ... );

extern int
ys_file_vprintf( ys_file_t *file, const char *format, va_list args );

extern const char *
ys_file_gets( char *buf, size_t size, ys_file_t *file );

extern size_t
ys_file_read( void* buf, size_t size, size_t items, ys_file_t *file );

extern size_t
ys_file_write( const void* buf, size_t size, size_t items, ys_file_t *file );

extern int
ys_file_getc( ys_file_t *file );

extern int
ys_file_putc( int ch, ys_file_t *file );

extern int
ys_file_error( ys_file_t *file );

extern int
ys_file_eof( ys_file_t *file );

extern void
ys_file_rewind( ys_file_t *file );

extern ys_file_t *
ys_pipe_open( const char *command, const char *mode, int flags );

extern int
ys_pipe_close( ys_file_t *file );

#endif
