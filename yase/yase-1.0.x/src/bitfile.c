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
/* bitfile.h */
/* dibyendu majumdar */
/* created 31 dec 1999 */
/* modified 1 jan 2000 */

#include "bitfile.h"
#include "util.h"

#define BYTESIZE	8
#define BITS(n)		(sizeof(n)*BYTESIZE)
#define LEFTMOST_BIT(n)	(BITS(n)-1)

typedef int readfunc_t(void *, size_t, size_t, void *);
typedef int writefunc_t(void *, size_t, size_t, void *);
typedef int getposfunc_t(void *, ys_filepos_t *);
typedef int setposfunc_t(void *, ys_filepos_t *);
typedef int eoffunc_t(void *);
typedef int closefunc_t(void *);
typedef int errorfunc_t(void *);

typedef struct {
	readfunc_t *read;
	writefunc_t *write;
	getposfunc_t *getpos;
	setposfunc_t *setpos;
	eoffunc_t *eof;
	closefunc_t *close;
	errorfunc_t *error;
} vtable_t;

vtable_t Filetab = {
	(readfunc_t *)ys_file_read,
	(writefunc_t *)ys_file_write,
	(getposfunc_t *)ys_file_getpos,
	(setposfunc_t *)ys_file_setpos,
	(eoffunc_t *)ys_file_eof,
	(closefunc_t *)ys_file_close,
	(errorfunc_t *)ys_file_error
};

enum { RESET_GET = 1, RESET_PUT = 2 };
static int ys_breset( ys_bitfile_t *bf, int flags );
static int ys_bcheck_error( ys_bitfile_t *bf, int error_rc, const char *msg );

ys_bitfile_t * 
ys_bopen( const char *name, const char *mode )
{
	const char *omode = "rb+";
	ys_bitfile_t *bf = calloc(1, sizeof(ys_bitfile_t));
	if ( bf == 0 ) {
		perror("calloc");
		fprintf(stderr, "Error allocating memory\n");
		return 0;
	}
	if ( mode != 0 ) omode = mode;
	bf->fp = ys_file_open( name, omode, mode == 0 ? 0 : YS_FILE_ABORT_ON_ERROR );
	if ( bf->fp == 0 && mode == 0 ) {
		bf->fp = ys_file_open( name, "wb+", 
			mode == 0 ? 0 : YS_FILE_ABORT_ON_ERROR );
	}
	if ( bf->fp == 0 ) {
		fprintf(stderr, "Error opening file %s\n", name);
		free( bf );
		return 0;
	}
	ys_breset( bf, RESET_GET|RESET_PUT );
	bf->gnbit = 0;			/* force read */
	bf->gpos = 0;
	bf->ppos = 0;
	bf->v = &Filetab;
	return bf;
}

static int 
ys_breset( ys_bitfile_t *bf, int flags )
{
	if (flags & RESET_PUT) {
		bf->pch = 0;
		bf->pnbit = BITS(bf->pch);
		bf->pmask = 1 << LEFTMOST_BIT(bf->pnbit);
	}
	if (flags & RESET_GET) {
		bf->gch = 0;
		bf->gnbit = BITS(bf->gch);
		bf->gmask = 1 << LEFTMOST_BIT(bf->gnbit);
	}
	return 0;
}

static int 
ys_bcheck_error( ys_bitfile_t *bf, int error_rc, const char *msg )
{
	vtable_t *vt = (vtable_t *)bf->v;
	if ( vt->error( bf->fp ) ) {
		fprintf(stderr, msg);
		return error_rc;
	}
	return 0;
}

int 
ys_bflush( ys_bitfile_t *bf )
{
	vtable_t *vt = (vtable_t *)bf->v;
	if ( bf->pnbit != BITS(bf->pch) ) {
		if (Ys_debug >= 3) {
			printf("%s:%s: Wrote %lu at %lu (", 
				__FILE__, __func__, bf->pch, bf->ppos);
			ys_print_as_bits(stderr, bf->pch); 
			fprintf(stderr, ")\n");
		}
		vt->setpos( bf->fp, &bf->ppos );
		vt->write( &bf->pch, 1, sizeof bf->pch, bf->fp );
		bf->ppos += sizeof bf->pch;
		ys_breset( bf, RESET_PUT );
		return ys_bcheck_error( bf, -1, "Error flushing contents of file\n" );
	}
	return 0;
}

int 
ys_bfill( ys_bitfile_t *bf )
{
	vtable_t *vt = (vtable_t *)bf->v;
	if ( !vt->eof(bf->fp) ) {
		ys_breset( bf, RESET_GET );
		vt->setpos( bf->fp, &bf->gpos );
		vt->read( &bf->gch, 1, sizeof bf->gch, bf->fp );
		bf->gpos += sizeof bf->gch;
		if (Ys_debug >= 3) {
			printf("%s:%s: Read %lu at %lu (", 
				__FILE__, __func__, bf->gch, bf->gpos);
			ys_print_as_bits(stderr, bf->gch);
			fprintf(stderr, ")\n");
		}
		return ys_bcheck_error( bf, EOF-1, "Error reading file\n" );
	}
	return EOF;
}


#ifndef _INLINE_MAX
int 
ys_bputbit( ys_bitfile_t *bf, ys_bit_t bit )
{
	if (bf->pnbit == 0) {
		if ( ys_bflush( bf ) != 0 )
			return -1;
	}
	if (bit)
		bf->pch |= bf->pmask;
	else
		bf->pch &= ~bf->pmask;
	bf->pmask >>= 1;
	bf->pnbit--;
	return 0;
}

int 
ys_bgetbit( ys_bitfile_t *bf, ys_bit_t *bit_ptr )
{
	int bit;
	if ( bf->gnbit == 0 ) {
		if (ys_bfill( bf ) == EOF-1)
			return EOF-1;
		if (bf->gnbit == 0) {
			return EOF;
		}
	}
	bit = (bf->gch & bf->gmask) ? 1 : 0;
	bf->gmask >>= 1;
	bf->gnbit--;
	*bit_ptr = bit;
	return 0;
}
#endif

int 
ys_bclose( ys_bitfile_t * bf ) 
{
	vtable_t *vt = (vtable_t *)bf->v;
	int rc;
	ys_bflush( bf );
	rc = vt->close(bf->fp);
	if (rc != 0) {
		fprintf(stderr, "Error closing file\n");
	}
	free(bf);
	return rc;
}

int 
ys_bgeof( ys_bitfile_t * bf )
{
	vtable_t *vt = (vtable_t *)bf->v;
	return (bf->gnbit == 0 && vt->eof(bf->fp));
}

int 
ys_bgetbits( ys_bitfile_t *bf, ys_bits_t *bits_ptr, int nbits ) 
{
	ys_bits_t bits = 0;
	int bitnum = nbits-1;
	ys_bits_t mask = 1 << bitnum;
	for (; bitnum >= 0; bitnum--, mask >>= 1) {
		ys_bit_t bit;
		if (ys_bgetbit( bf, &bit ) != 0)
			return -1;
		if (bit)
			bits |= mask;
	}
	*bits_ptr = bits;
	return 0;
}

int 
ys_bputbits( ys_bitfile_t *bf, ys_bits_t n, int nbits ) 
{
	int bitnum = nbits-1;
	ys_bits_t mask = 1 << bitnum;
	for (; bitnum >= 0; bitnum--, mask >>= 1)
		if (ys_bputbit( bf, (n&mask) ? 1 : 0 ) != 0)
			return -1;
	return 0;
}

int 
ys_bsetgpos( ys_bitfile_t *bf, ys_filepos_t *pos )
{
	bf->gpos = *pos;
	ys_breset( bf, RESET_GET );
	bf->gnbit = 0;			/* force read */
	return 0;
}

int 
ys_bgetgpos( ys_bitfile_t *bf, ys_filepos_t *pos )
{
	*pos = bf->gpos;
	return 0;
}

int 
ys_bsetppos( ys_bitfile_t *bf, ys_filepos_t *pos )
{
	bf->ppos = *pos;
	ys_breset( bf, RESET_PUT );
	return 0;
}

int 
ys_bgetppos( ys_bitfile_t *bf, ys_filepos_t *pos )
{
	*pos = bf->ppos;
	return 0;
}

#ifdef TEST_BITFILE

int Ys_debug = 3;

int main()
{
	ys_bitfile_t *bf;
	int i, t;
	char obits[] = "10101010101010101010101010110101010101010101010101010"
		      "11111111111111000000000000001111100001111111111100000"
		      "00000000001111111111111111110101010111010101100010101";
	char ibits[sizeof obits] = {0};
	ys_bits_t bits;
	ys_filepos_t pos = 0;
	const char *tmpnm = "tmp.testfile";
	ys_uintmax_t ll[5];

	remove(tmpnm);
	for (t = 0; t < 2; t++) {

		bf = ys_bopen( tmpnm, 0 );
		for (i = 0; i < sizeof obits-1; i++) {
			ys_bputbit( bf, obits[i] == '1' ? 1 : 0 );
		}
		ys_bflush( bf );
		pos = 0;
		ys_bsetgpos( bf, &pos );
		for (i = 0; i < sizeof ibits-1; i++) {
			ys_bit_t bit;
			ys_bgetbit( bf, &bit );
			ibits[i] = bit ? '1' : '0';
		}
		if (strcmp(obits, ibits) != 0) {
			fprintf(stderr, "test 1 failed\n");
			fprintf(stderr, "obits=%s\n", obits);
			fprintf(stderr, "ibits=%s\n", ibits);
			exit(1);
		}
		ys_bclose( bf );
		bf = ys_bopen( tmpnm, 0 );
		pos = 0;
		ys_bsetgpos( bf, &pos );
		for (i = 0; i < 5; i++) {
			ys_bgetbits(bf, &bits, BITS(bits));
			if (t == 0)
				ll[i] = bits;
			else if (ll[i] != bits) {
				fprintf(stderr, "test 2 failed\n");
				fprintf(stderr, "read=%lu\n", bits);
				fprintf(stderr, "expected=%lu\n", ll[i]);
				exit(1);
			}
		}
		ys_bclose( bf );
	}
	remove(tmpnm);

	for (t = 0; t < 2; t++) {

		bf = ys_bopen( tmpnm, 0 );
		for (i = 0; i < 1000; i++) {
			ys_bits_t bits = i;
			ys_filepos_t pos = i*sizeof(ys_bits_t);
			ys_bsetppos( bf, &pos );
			ys_bputbits( bf, bits, BITS(bits) );
			ys_bflush( bf );
		}

		for (i = 0; i < 1000; i++) {
			ys_bits_t bits;
			ys_filepos_t pos = i*sizeof(ys_bits_t);
		
			ys_bsetgpos( bf, &pos );
			ys_bgetbits( bf, &bits, BITS(bits) );
			if (bits != i) {
				fprintf(stderr, "test 3 failed\n");
				fprintf(stderr, "expected %lu\n", (ys_bits_t)i);
				fprintf(stderr, "read %lu\n", bits);
				break;
			}
		}
		ys_bclose( bf );
	}
	remove(tmpnm);
	fprintf(stdout, "Test completed OK\n");
		
	return 0;
}

#endif
