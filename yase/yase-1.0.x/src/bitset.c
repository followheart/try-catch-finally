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

#include "bitset.h"
#include "util.h"

#define NBYTES(n)	(sizeof(unsigned)*n)
#define ESIZE		(sizeof(unsigned)*8)

/**
 * Allocate a new bitset.
 */
ys_bitset_t *
ys_bs_alloc( unsigned size )
{
	ys_bitset_t *bs = (ys_bitset_t *)calloc(1, sizeof(ys_bitset_t));
	if (bs == 0) {
		fprintf(stdout, "Error allocating memory\n");
		exit(1);
	}
        bs->n = YS_DIVIDE_AND_ROUNDUP(size, ESIZE);

        bs->data = (unsigned *)calloc(bs->n, sizeof(unsigned));
	if (bs->data == 0) {
		fprintf(stdout, "Error allocating memory\n");
		exit(1);
	}
        bs->size = size;
        return bs;
}

/**
 * Test membership
 */
ys_bool_t
ys_bs_ismember( ys_bitset_t *bs, unsigned bit )
{
	unsigned n;
        if ( bit >= bs->size )
        	return BOOL_FALSE;
        n = bit / ESIZE;
        bit = bit % ESIZE;
        return bs->data[n] & (1 << bit);
}

/**
 * Add a new member.
 */
unsigned
ys_bs_addmember( ys_bitset_t *bs, unsigned bit )
{
	unsigned n;
        unsigned oldbit = 0;
        if ( bit >= bs->size )
        	return oldbit;
        n = bit / ESIZE;
        bit = bit % ESIZE;
        oldbit = bs->data[n] & (1 << bit);
        bs->data[n] |= (1 << bit);
        return oldbit;
}

/**
 * Turn on all the bits in the set.
 */
void
ys_bs_setall( ys_bitset_t *bs )
{
	unsigned i;
        unsigned bit = ~0;
        for (i = 0; i < bs->n; i++) {
        	bs->data[i] = bit;
        }
}

/**
 * Turn them all off.
 */
void
ys_bs_clear( ys_bitset_t *bs )
{
	unsigned i;
        unsigned bit = 0;
        for (i = 0; i < bs->n; i++) {
        	bs->data[i] = bit;
        }
}

/**
 * Reverse each bit in the set.
 */
void
ys_bs_complement( ys_bitset_t *bs )
{
	unsigned i;
        for (i = 0; i < bs->n; i++) {
        	bs->data[i] = ~(bs->data[i]);
        }
}

/**
 * Removed a member from the set.
 */
void
ys_bs_delmember( ys_bitset_t *bs, unsigned bit )
{
	unsigned n;
        if ( bit >= bs->size )
        	return;
        n = bit / ESIZE;
        bit = bit % ESIZE;
        bs->data[n] &= ~(1 << bit);
}

/**
 * Union bs1 and bs2 and store the result in bs1.
 */
ys_bitset_t *
ys_bs_union( ys_bitset_t *bs1, ys_bitset_t *bs2 )
{
        unsigned i;

        assert(bs1->size == bs2->size);
        for (i = 0; i < bs1->n; i++) {
        	bs1->data[i] |= bs2->data[i];
	}
        return bs1;
}

/**
 * Union bs1 and bs2 and store the result in bs3.
 */
ys_bitset_t *
ys_bs_union2( ys_bitset_t *bs1, ys_bitset_t *bs2, ys_bitset_t *bs3 )
{
        unsigned i;

        assert(bs1->size == bs2->size);
        if (bs3 == 0)
        	bs3 = ys_bs_alloc(bs1->size);

        for (i = 0; i < bs1->n; i++) {
        	bs3->data[i] = bs1->data[i] | bs2->data[i];
	}
        return bs3;
}

/**
 * Intersect bs1 and bs2 and store the result in bs1.
 */
ys_bitset_t *
ys_bs_intersect( ys_bitset_t *bs1, ys_bitset_t *bs2 )
{
	unsigned i;

        assert(bs1->size == bs2->size);
        for (i = 0; i < bs1->n; i++) {
        	bs1->data[i] &= bs2->data[i];
        }
        return bs1;
}

/**
 * Intersect bs1 and bs2 and store the result in bs3.
 */
ys_bitset_t *
ys_bs_intersect2( ys_bitset_t *bs1, ys_bitset_t *bs2, ys_bitset_t *bs3 )
{
	unsigned i;

        assert(bs1->size == bs2->size);
        if (bs3 == 0)
        	bs3 = ys_bs_alloc(bs1->size);

        for (i = 0; i < bs1->n; i++) {
        	bs3->data[i] = bs1->data[i] & bs2->data[i];
        }
        return bs1;
}

/**
 * Subtract bitset bs2 from bs1 and store result in bs1.
 */
ys_bitset_t *
ys_bs_minus( ys_bitset_t *bs1, ys_bitset_t *bs2 )
{
	unsigned i;

        assert(bs1->size == bs2->size);
        for (i = 0; i < bs1->n; i++) {
        	bs1->data[i] = bs1->data[i] & ~(bs2->data[i]);
        }
        return bs1;
}

/**
 * Subtract bitset bs2 from bs1 and store result in bs3.
 */
ys_bitset_t *
ys_bs_minus2( ys_bitset_t *bs1, ys_bitset_t *bs2, ys_bitset_t *bs3 )
{
	unsigned i;

        assert(bs1->size == bs2->size);
        if (bs3 == 0)
        	bs3 = ys_bs_alloc(bs1->size);

        for (i = 0; i < bs1->n; i++) {
        	bs3->data[i] = bs1->data[i] & ~(bs2->data[i]);
        }
        return bs1;
}

/** 
 * Make a copy of a bitset.
 */
ys_bitset_t *
ys_bs_clone( ys_bitset_t *bs1, ys_bitset_t *bs2 )
{
	unsigned i;

        if (bs1 == 0)
        	bs1 = ys_bs_alloc(bs2->size);
        for (i = 0; i < bs1->n; i++) {
        	bs1->data[i] = bs2->data[i];
        }
        return bs1;
}

/**
 * Slower version of iterate()
 */
void
ys_bs_iterate1( ys_bitset_t *bs, int flag, ys_bool_t (*f)( void *, unsigned, ys_bool_t ), void *arg )
{
        unsigned i;
        for (i = 0; i < bs->size; i++) {
		unsigned n, bit;
                ys_bool_t is_set;

	        n = i / ESIZE;
	        bit = i % ESIZE;
        	is_set = (bs->data[n] & (1 << bit));
                if ( is_set ) {
	                if ( !f(arg, i, BOOL_TRUE) )
        	        	break;
                }
                else if ( flag == YS_BS_ITER_ALL ) {
	                if ( !f(arg, i, BOOL_FALSE) )
        	        	break;
                }
        }
}

/**
 * Iterate through the bits.
 * If flag is YS_BS_ITER_ALL, evaluate supplied function f for all bits,
 * otherwise only evaluate it for members.
 */
void
ys_bs_iterate( ys_bitset_t *bs, int flag, ys_bool_t (*f)( void *, unsigned, ys_bool_t ), void *arg )
{
        unsigned i, j;
        unsigned bit = 0;
        for (i = 0; i < bs->n && bit < bs->size; i++) {
	        unsigned mask = 1;
		for (j = 0; j < ESIZE && bit < bs->size; j++) {
                	ys_bool_t is_set;

	        	is_set = (bs->data[i] & mask) != 0;

	                if ( is_set ) {
		                if ( !f(arg, bit, BOOL_TRUE) )
	        	        	return;
	                }
	                else if ( flag == YS_BS_ITER_ALL ) {
		                if ( !f(arg, bit, BOOL_FALSE) )
	        	        	return;
	                }
                        if (++bit == bs->size)
                        	return;
                        mask <<= 1;
                }
        }
}

/**
 * Count number of members in the bitset.
 */
unsigned ys_bs_count( ys_bitset_t *bs )
{
	/* This array was generated using bt.c */
static unsigned bitcount[] = {
0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,
4,5,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,
5,5,6,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,
4,5,5,6,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,
6,5,6,6,7,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,
4,5,4,5,5,6,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,
5,5,6,5,6,6,7,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,
4,5,5,6,5,6,6,7,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,
7,5,6,6,7,6,7,7,8};

	unsigned n;
	unsigned char *cp;
	unsigned u;
	unsigned count = 0;
	unsigned left;
	unsigned last;
	unsigned mask;

	n = bs->n-1;
	cp = (unsigned char *)bs->data;
	for (u = 0, count = 0; u < NBYTES(n); u++) {
		count += bitcount[*cp];
		cp++;
	}
	left = bs->size - (n * ESIZE);
	last = bs->data[n];
	for (u = 0, mask = 1; u < left; u++) {
		if (mask & last)
			count++;
		mask <<= 1;
	}
	return count;
}	

/**
 * Free memory allocated to a bitset.
 */
void
ys_bs_destroy( ys_bitset_t *bs )
{
	free(bs->data);
	bs->data = 0;
	free(bs);
}

ys_bool_t
printset( void *arg, unsigned bit, ys_bool_t is_set )
{
	putc(is_set ? '1' : '0', stdout);
	return BOOL_TRUE;
}

int testbitset(void)
{
	ys_bitset_t *bs1, *bs2, *bs3;

        bs1 = ys_bs_alloc(100);
        ys_bs_addmember(bs1, 0);
        assert(ys_bs_ismember(bs1, 0));
        ys_bs_addmember(bs1, 32);
        assert(ys_bs_ismember(bs1, 32));
	assert(ys_bs_count(bs1) == 2);

        bs2 = ys_bs_alloc(100);
        ys_bs_addmember(bs2, 10);
        assert(ys_bs_ismember(bs2, 10));
        ys_bs_addmember(bs2, 99);
        assert(ys_bs_ismember(bs2, 99));
        ys_bs_addmember(bs2, 32);
        assert(ys_bs_ismember(bs2, 32));
        ys_bs_delmember(bs2, 32);
        assert(!ys_bs_ismember(bs2, 32));
	assert(ys_bs_count(bs2) == 2);

        bs3 = ys_bs_alloc(100);

        ys_bs_iterate(bs1, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);
        ys_bs_iterate(bs2, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);

        ys_bs_union2(bs1, bs2, bs3);
	assert(ys_bs_count(bs3) == 4);
        ys_bs_iterate(bs3, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);
        ys_bs_intersect2(bs1, bs2, bs3);
	assert(ys_bs_count(bs3) == 0);
        ys_bs_iterate(bs3, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);
        ys_bs_minus2(bs1, bs2, bs3);
	assert(ys_bs_count(bs3) == 2);
        ys_bs_iterate(bs3, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);

        ys_bs_complement(bs1);
	assert(ys_bs_count(bs1) == 98);
        ys_bs_iterate(bs1, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);

        ys_bs_setall(bs1);
	assert(ys_bs_count(bs1) == 100);
        ys_bs_iterate(bs1, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);

        ys_bs_clear(bs1);
	assert(ys_bs_count(bs1) == 0);
        ys_bs_iterate(bs1, YS_BS_ITER_ALL, printset, 0 ); putc('\n', stdout);

        return 0;
}

