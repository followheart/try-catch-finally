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

/*
 * Dibyendu Majumdar
 * 14 April 2001
 */
#ifndef BITSET_H
#define BITSET_H

#include "yase.h"

typedef struct {
	unsigned *data;
        unsigned size;		/* no of bits in this set */
        unsigned n;		/* data[n] */
} ys_bitset_t;

enum {
	YS_BS_ITER_ALL,
        YS_BS_ITER_MEMBERS
};

extern ys_bitset_t * ys_bs_alloc( unsigned size );
extern void ys_bs_destroy( ys_bitset_t *bs );
extern ys_bool_t ys_bs_ismember( ys_bitset_t *bs, unsigned bit );
extern unsigned ys_bs_addmember( ys_bitset_t *bs, unsigned bit );
extern void ys_bs_setall( ys_bitset_t *bs );
extern void ys_bs_clear( ys_bitset_t *bs );
extern void ys_bs_complement( ys_bitset_t *bs );
extern void ys_bs_delmember( ys_bitset_t *bs, unsigned bit );
extern ys_bitset_t * ys_bs_union( ys_bitset_t *bs1, ys_bitset_t *bs2 );
extern ys_bitset_t * ys_bs_union2( ys_bitset_t *bs1, ys_bitset_t *bs2, ys_bitset_t *bs3 );
extern ys_bitset_t * ys_bs_intersect( ys_bitset_t *bs1, ys_bitset_t *bs2 );
extern ys_bitset_t * ys_bs_intersect2( ys_bitset_t *bs1, ys_bitset_t *bs2, ys_bitset_t *bs3 );
extern ys_bitset_t * ys_bs_minus( ys_bitset_t *bs1, ys_bitset_t *bs2 );
extern ys_bitset_t * ys_bs_minus2( ys_bitset_t *bs1, ys_bitset_t *bs2, ys_bitset_t *bs3 );
extern ys_bitset_t * ys_bs_clone( ys_bitset_t *bs1, ys_bitset_t *bs2 );
extern void ys_bs_iterate( ys_bitset_t *bs, int flag, ys_bool_t (*f)( void *, unsigned, ys_bool_t ), void *arg );
extern unsigned ys_bs_count( ys_bitset_t *bs );

#endif
