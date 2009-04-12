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
/* dibyendu majumdar */
/* created 31 dec 1999 */
/* modified 1 jan 2000 */

#ifndef bitfile_h
#define bitfile_h

#include "yase.h"
#include "ystdio.h"

typedef ys_uintmax_t ys_bits_t;
typedef ys_uintmax_t ys_bit_t;

typedef struct {
	void *fp;
	void *v;
	ys_bits_t pch;
	ys_bits_t gch;
	ys_bits_t pmask;
	ys_bits_t gmask;
	ys_filepos_t ppos;
	ys_filepos_t gpos;			
	int pnbit;					/* Number of bits to put */
	int gnbit;					/* Number of bits to get */
} ys_bitfile_t;

extern ys_bitfile_t * ys_bopen( const char *name, const char *mode );
extern int ys_bflush( ys_bitfile_t *bf );
extern int ys_bfill( ys_bitfile_t *bf );
extern int ys_bgeof( ys_bitfile_t *bf );
#ifndef _INLINE_MAX
extern int ys_bputbit( ys_bitfile_t *, ys_bit_t bit );
extern int ys_bgetbit( ys_bitfile_t *, ys_bit_t *bit );
#endif
extern int ys_bgetbits( ys_bitfile_t *, ys_bits_t *n, int nbits );
extern int ys_bputbits( ys_bitfile_t *, ys_bits_t n, int nbits );
extern int ys_bsetgpos( ys_bitfile_t *, ys_filepos_t * );
extern int ys_bgetgpos( ys_bitfile_t *, ys_filepos_t * );
extern int ys_bsetppos( ys_bitfile_t *, ys_filepos_t * );
extern int ys_bgetppos( ys_bitfile_t *, ys_filepos_t * );
extern int ys_bclose( ys_bitfile_t * );

#ifdef _INLINE_MAX
static inline int 
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

static inline int 
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

#endif
