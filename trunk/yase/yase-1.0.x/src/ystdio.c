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
/*
 * 16-nov-01: Fixed portability problems in ys_file_setpos() and ys_file_getpos().
 * 16-nov-01: Fixed incorrect use of va_start() in ys_file_printf().
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "yase.h"
#include "ystdio.h"

struct ys_file_t {
	FILE *file;
	int error;
	int flags;
	char filename[1024];
};

/**
 * Reports an IO error and if YS_FILE_ABORT_ON_ERROR flag is set, the application
 * is terminated.
 */
static void
report_error( ys_file_t *file, const char *operation )
{
	file->error = errno;
	fprintf(stderr, "Operation %s on '%s' failed: %s\n",
		operation, file->filename, strerror(file->error));
	if (file->flags & YS_FILE_ABORT_ON_ERROR) {
		fprintf(stderr, "Execution terminated\n");
		assert(0);
		exit(1);
	}
}

/**
 * Opens a file.
 */
ys_file_t *
ys_file_open( const char *filename, const char *mode, int flags )
{
	ys_file_t * file = (ys_file_t*) calloc(1, sizeof(ys_file_t));
	char mymode[10];
	if (file == 0) {
		fprintf(stderr, "Failed to allocate memory\n");	
		return 0;
	}
#ifdef WIN32
	snprintf(mymode, sizeof mymode, "%sb", mode);
#else
	strncpy(mymode, mode, sizeof mymode);
#endif
	snprintf( file->filename, sizeof file->filename,
		"%s", filename );
	file->flags = 0;	/* we set the flag after opening the file */
	file->file = fopen( filename, mymode );
	if ( file->file == 0 ) {
		report_error( file, "OPEN" );
		free(file);
		return 0;
	}
	file->flags = flags;
	return file;
}

/**
 * fclose() clone.
 */
int
ys_file_close( ys_file_t *file )
{
	int rc = fclose( file->file );
	if (rc != 0)
		report_error( file, "CLOSE" );
	free(file);
	return rc;
}

/**
 * Get the current file position. Note that in future this will need to
 * handle 64 bit pointers.
 */
int 
ys_file_getpos( ys_file_t *file, ys_filepos_t *pos )
{
	long int _pos;
	int rc = 0;

	errno = 0;
	_pos = ftell( file->file );
	if (_pos == -1L && errno != 0) {
		report_error( file, "GETPOS" );
		rc = -1;
	}
	else
		*pos = (ys_filepos_t) _pos;
	return rc;
}

/**
 * Sets the current file position. Note that in future this will need to
 * handle 64 bit pointers.
 */
int 
ys_file_setpos( ys_file_t *file, const ys_filepos_t *pos )
{
	return ys_file_seek( file, *pos, SEEK_SET );
}

/**
 * Sets the current file position. Note that in future this will need to
 * handle 64 bit pointers.
 */
int
ys_file_seek( ys_file_t *file, ys_filepos_t offset, int whence )
{
	int rc;
	long int _offset = (long int) offset;
	assert((ys_filepos_t)_offset == offset);
	rc = fseek( file->file, _offset, whence );
	if (rc != 0)
		report_error( file, "SEEK" );
	return rc;
}

/**
 * fprintf() clone.
 */
int
ys_file_printf( ys_file_t *file, const char *format, ... )
{
	int rc;
	va_list args;

	va_start( args, format );
	rc = vfprintf( file->file, format, args );
	if (rc < 0)
		report_error( file, "PRINTF" );
	va_end(args);
	return rc;
}

/**
 * fvprintf() clone.
 */
int
ys_file_vprintf( ys_file_t *file, const char *format, va_list args )
{
	int rc;
	rc = vfprintf( file->file, format, args );
	if (rc < 0)
		report_error( file, "VPRINTF" );
	return rc;
}

/**
 * fgets() clone.
 */
const char *
ys_file_gets( char *buf, size_t size, ys_file_t *file )
{
	const char *rbuf;

	rbuf = fgets( buf, size, file->file );
	if ( ferror(file->file) ) {
		report_error( file, "GETS" );
	}
	return rbuf;
}

/**
 * fread() clone.
 */
size_t
ys_file_read( void* buf, size_t size, size_t items, ys_file_t *file )
{
	size_t rc;

	rc = fread( buf, size, items, file->file );
	if ( ferror(file->file) )
		report_error( file, "READ" );
	return rc;
}

/**
 * fwrite() clone.
 */
size_t
ys_file_write( const void* buf, size_t size, size_t items, ys_file_t *file )
{
	size_t rc;

	rc = fwrite( buf, size, items, file->file );
	if ( ferror(file->file) )
		report_error( file, "WRITE" );
	return rc;
}

/**
 * popen() clone.
 */
ys_file_t *
ys_pipe_open( const char *command, const char *mode, int flags )
{
	ys_file_t * file = (ys_file_t*) calloc(1, sizeof(ys_file_t));
	if (file == 0) {
		fprintf(stderr, "Failed to allocate memory\n");	
		return 0;
	}
	snprintf( file->filename, sizeof file->filename,
		"%s", command );
	file->flags = 0;	/* we set the flag after opening the file */
	file->file = popen( command, mode );
	if ( file->file == 0 ) {
		report_error( file, "POPEN" );
		free(file);
		return 0;
	}
	file->flags = flags;
	return file;
}

/**
 * pclose() clone.
 */
int
ys_pipe_close( ys_file_t *file )
{
	int rc, prc;

	prc = rc = pclose(file->file);

	if ( rc < 0 ) {
		report_error( file, "PCLOSE" );
	}
#ifndef WIN32 
	else if (WIFEXITED(rc)) {
		rc = WEXITSTATUS(rc);
		if (rc != 0) {
			fprintf(stderr, "Command '%s' failed with rc=%d\n",
				file->filename, rc);
		}
	}
	else if (WIFSTOPPED(rc)) {
		rc = WSTOPSIG(rc);
		fprintf(stderr, "Command '%s' stopped by signal=%d\n",
			file->filename, rc);
	}
	else {
		rc = WTERMSIG(rc);
		fprintf(stderr, "Command '%s' terminated by signal=%d\n",
			file->filename, rc);
	}
#endif
	free( file );
	return prc;
}

/**
 * fgetc() clone.
 */
int
ys_file_getc( ys_file_t *file )
{
	int ch;

	ch = fgetc( file->file );
	if ( ferror(file->file) )
		report_error( file, "GETC" );
	return ch;
}

/**
 * fputc() clone.
 */
int
ys_file_putc( int ch, ys_file_t *file )
{
	int rc;
	rc = fputc( ch, file->file );
	if ( ferror(file->file) )
		report_error( file, "PUTC" );
	return rc;
}

/**
 * ferror() clone.
 */
int
ys_file_error( ys_file_t *file )
{
	return ferror(file->file);
}

/**
 * feof() clone.
 */
int 
ys_file_eof( ys_file_t *file )
{
	return feof(file->file);
}

/**
 * rewind() clone.
 */
void
ys_file_rewind( ys_file_t *file )
{
	rewind(file->file);
	if ( ferror(file->file) )
		report_error( file, "REWIND" );
}

#ifdef _TEST_YSTDIO

int main(int argc, char *argv[])
{
	ys_file_t *file;
	char buf[256];

	if (argc < 3) {
		fprintf(stderr, "usage: %s [fc] <arg>\n", argv[0]);
		return 1;
	}

	if (argv[1][0] == 'f') {
		file = ys_file_open(argv[2], "r", YS_FILE_ABORT_ON_ERROR);
		ys_file_gets(buf, sizeof buf, file);
		ys_file_close(file);
	}
	else {
		file = ys_pipe_open(argv[2], "r", YS_FILE_ABORT_ON_ERROR);
		ys_file_gets(buf, sizeof buf, file);
		ys_pipe_close(file);
	}

	return 0;
}

#endif
