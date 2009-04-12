/***
*    YASE (Yet Another Search Engine) 
*    Copyright (C) 2000-2002    Dibyendu Majumdar.
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
#ifndef compress_h
#define compress_h

#include "yase.h"
#include "bitfile.h"

extern int ys_unary_encode(ys_bitfile_t *bf, ys_uintmax_t x);
extern int ys_unary_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr);
extern int ys_gamma_encode(ys_bitfile_t *bf, ys_uintmax_t x);
#ifndef _INLINE_MAX
extern int ys_gamma_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr);
#endif
extern int ys_binary_encode(ys_bitfile_t *bf, ys_uintmax_t x, ys_uintmax_t b);
extern int ys_binary_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr, ys_uintmax_t b);
extern int ys_delta_encode(ys_bitfile_t *bf, ys_uintmax_t x);
extern int ys_delta_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr);

#ifdef _INLINE_MAX
static inline int 
ys_gamma_decode(ys_bitfile_t *bf, ys_uintmax_t *x_ptr)
{
	ys_uintmax_t x = 1;
	register ys_intmax_t nbits = 0;
	ys_bit_t bit;

	for (;;) {
		if (ys_bgetbit(bf, &bit) <= EOF)
			return -1;
		if (bit == 0)
			nbits++;
		else
			break;
	}
	while ( nbits-- > 0 ) {
		if (ys_bgetbit(bf, &bit) <= EOF)
			return -1;
		x += x + bit;
	}
	*x_ptr = x;
	return 0;
}
#endif

#endif
