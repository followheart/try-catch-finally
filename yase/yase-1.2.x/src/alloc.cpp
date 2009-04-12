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
* alloc.c - a fast memory allocator 
*
* dibyendu majumdar                 
* adapted from code in C++ STL lib 
*
* This file implements a fast fixed size memory 
* allocator. It is very useful for situations where 
* you need to allocate and free fixed size memory chunks
* fairly frequently.
*/

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "yase.h"
#include "alloc.h"
#include "util.h"

typedef long int ys_max_align_t;
enum {
	YS_CHUNKSIZE =      32,
	YS_BLOCKSIZE =      256,
	YS_MAXALLOCATORS =  25,
	YS_OBJECTS_PER_ALLOCATOR = 100
};

static ys_allocator_t *FixedSizedAllocators[YS_MAXALLOCATORS];
unsigned long Maxmem;		/* TODO: Get rid of this */

static void *
safe_malloc(size_t n) 
{
	void *ptr = calloc(1, n);
	if (ptr == 0) {
		perror("calloc");
		fprintf(stderr, "Failed to allocate memory\n");
		exit(1);
	}
	return ptr;
}

/**
 * Create new allocator.
 * If size is 0, then the allocator can be used to allocate
 * variable chunks of memory - but these chunks cannot be deallocated.
 * If size is supplied, then the allocator can be used to
 * allocate/deallocate fixed size chunks of memory.
 */

ys_allocator_t *
ys_new_allocator(
	size_t size, 	/* Size of individual objects */
	size_t n)	/* Number of objects to get memory for at a time */
{
	ys_allocator_t *a;
	size_t nbytes;
	size_t osize = size, on = n;

	assert(n > 0);

	if (size > 0 && size < sizeof(ys_buflink_t))
		/* Object must be at least as big as ys_buflink_t */
		size = sizeof(ys_buflink_t);

	if (size > 0) {
		/* Make sure that memory is allocated meets alignment
		 * requirements.
		 */
		size = YS_ROUND_TO_MULTIPLE_OF(size, sizeof(ys_max_align_t));

		/* Try to allocate chunks that are multiples of YS_BLOCKSIZE */
		nbytes = size*n;
		nbytes = YS_ROUND_TO_MULTIPLE_OF(nbytes, YS_BLOCKSIZE);
		n = nbytes/size;
	}
	else {
		nbytes = YS_BLOCKSIZE*YS_OBJECTS_PER_ALLOCATOR;
	}


	a = (ys_allocator_t *) safe_malloc(sizeof(ys_allocator_t));
	a->buffer_list = 0;
	a->free_list = 0;
	a->next_avail = 0;
	a->last = 0;
	a->size = size;
	a->n = n;
	a->nbytes = nbytes;

	if (Ys_debug >= 1) { 
		printf("%s:%s: Created allocator %p: requested(%d,%d), "
			"actual(%d,%d), nbytes(%d)\n",
			__FILE__, __func__,
			a, osize, on, size, n, nbytes);
	}

	return a;
}

/* 
 * Acquire more memory for the allocator 
 */
void 
ys_grow_allocator(ys_allocator_t *a)
{
	ys_buftype_t *tmp;

	tmp = (ys_buftype_t *) safe_malloc(sizeof(ys_buftype_t));
	tmp->buffer = (char *) safe_malloc(a->nbytes);
	tmp->next_buffer = a->buffer_list;
	a->buffer_list = tmp;
	a->next_avail = a->buffer_list->buffer;
	a->last = a->next_avail + a->nbytes;

	if (Ys_debug >= 2) {
		printf("%s:%s: Enlarged allocator %p(%d) by %d bytes\n", 
			__FILE__, __func__, a, a->size, a->nbytes);
	}

	Maxmem += a->nbytes;
}

static void
ys_validate_ptr(ys_allocator_t *a, void *ptr)
{
	ys_buftype_t *pool;
	char *p = (char *)ptr;

	for (pool = a->buffer_list; pool != 0; pool = pool->next_buffer) {
		if (p >= pool->buffer && p < pool->buffer+a->nbytes)
			return;
	}
	fprintf(stderr, "Error: Ptr %p does not belong to allocator %p\n", ptr, a);
	exit(1);
}

/**
 * Allocate memory using an allocator.
 * The size argument is valid only for variable size allocators.
 */
void *
ys_allocate(ys_allocator_t *a, size_t size)
{
	if (a->free_list) {
		ys_buflink_t *buflink = a->free_list;	
		/* If an object is on the free list, reuse it */
		if (Ys_debug >= 4) {
			ys_validate_ptr(a, buflink);
		}
		a->free_list = buflink->next;
		return (void *) buflink;
	} else {
		/* We ignore the size argument for fixed size allocators */
		if (a->size != 0)
			size = a->size;
		if (a->next_avail+size > a->last) {
			ys_grow_allocator(a);
		} 
		{
			void *new_chunk = (void *) a->next_avail;
			a->next_avail += size;
			assert(a->next_avail <= a->last);
			return new_chunk;
		}
	}
}

/**
 * Deallocate a chunk. This merely puts the chunk onto the free list
 * of the allocator.
 */
void 
ys_deallocate(ys_allocator_t * a, void *p)
{
	if (Ys_debug >= 1) {
		ys_validate_ptr(a, p);
	}

	if (a->size == 0) 
		/* We cannot deallocate variable sized chunks */
		return;

	((ys_buflink_t *)p)->next = a->free_list;
	a->free_list = (ys_buflink_t *) p;
}

/**
 * Free up all memory allocated to the allocator.
 */
void 
ys_destroy_allocator(ys_allocator_t * a)
{
	while (a->buffer_list) {
		ys_buftype_t *buflink = a->buffer_list;
		a->buffer_list = a->buffer_list->next_buffer;
		free(buflink->buffer);
		free(buflink);

		Maxmem -= a->nbytes;
	}
	free(a);
}

/**
 * The allocator above supports fixed or variable size objects,
 * but it does not allow an object to be resized.
 * YASE requires the ability to allocate memory that is subsequently
 * resized. Using malloc and realloc would slow down YASE, hence the
 * following implementation provides a method by which YASE can 
 * allocate/resize objects using the allocator framework. 
 * This is done as follows:
 * YASE maintains allocators of various sizes.
 * When a request for memory comes, it is allocated using the allocator
 * that is the best match for the object size.
 * If the object is re-sized, then if necessary, it is moved to another
 * allocator.
 */

static inline unsigned int
ys_find_allocator(size_t size, size_t *a_size)
{
	unsigned int a;

	*a_size = YS_ROUND_TO_MULTIPLE_OF(size, YS_CHUNKSIZE);
	a = (*a_size/YS_CHUNKSIZE)-1;

	return a;
}

/**
 * A malloc replacement.
 */
void *
ys_allocmem(size_t size)
{
	unsigned int a;
	size_t a_size;

	/* 
	 * First determine the allocator that is the best match
	 * for the object size.
	 */
	a = ys_find_allocator(size, &a_size);

	if (a >= YS_MAXALLOCATORS) {
		/* If the size of the object is greater than the
		 * largest allocator, then we simply use malloc.
		 */
		void *p = safe_malloc(a_size);
		Maxmem += a_size;
		if (Ys_debug >= 1) {
			fprintf(stderr,"Allocating memory %p req(%d), alloc(%d)\n", p, size, a_size);
		}
		return p;
	}

	if (FixedSizedAllocators[a] == 0) {
		/* The allocator does not exist */
		FixedSizedAllocators[a] = 
			ys_new_allocator(a_size, YS_OBJECTS_PER_ALLOCATOR);
	}

	return ys_allocate(FixedSizedAllocators[a], 0);
}

/**
 * Replacement for free().
 * We need to know the size of the object so that we can
 * determine the correct allocator.
 */
void
ys_freemem(void *p, size_t size)
{
	unsigned int a;
	size_t a_size;

	if (p == NULL)
		return;

	a = ys_find_allocator(size, &a_size);
	if (a >= YS_MAXALLOCATORS) {
		Maxmem -= a_size;
		if (Ys_debug >= 1) {
			fprintf(stderr,"Freeing memory %p req(%d), alloc(%d)\n", p, size, a_size);
		}
		free(p);
		return;
	}

	assert(FixedSizedAllocators[a] != 0);
	assert(FixedSizedAllocators[a]->size == a_size);

	ys_deallocate(FixedSizedAllocators[a], p);
}

/**
 * Replacement for realloc().
 * Resizes a piece of memory.
 */
void *
ys_reallocmem(void *p, size_t old_size, size_t new_size)
{
	void *np;
	unsigned int a_old, a_new;
	size_t a_old_size, a_new_size;

	assert(old_size < new_size);

	if (p == 0)
		return ys_allocmem(new_size);

	/* First check if we really need to reallocate */
	a_old = ys_find_allocator(old_size, &a_old_size);
	a_new = ys_find_allocator(new_size, &a_new_size);

	if (a_old == a_new) {
		/* No reallocation needed */
		return p;
	}

	np = ys_allocmem(new_size);
	memcpy(np, p, old_size);
	ys_freemem(p, old_size);
	return np;
}

/**
 * Destroy all allocators and release all memory.
 */
void
ys_destroyallmem(void)
{
	unsigned int a;
	for (a = 0; a < YS_MAXALLOCATORS; a++) {
		if (FixedSizedAllocators[a] == 0) 
			continue;
		ys_destroy_allocator(FixedSizedAllocators[a]);
		FixedSizedAllocators[a] = 0;
	}
}


#ifdef TEST_ALLOC

int Ys_debug = 1;

int main(int argc, const char *argv[]) 
{
	int i, j;
	void *ptr = 0;
	unsigned int old_size, new_size;

	for (j = 0; j < 1000; j++) {
		old_size = 0;
		new_size = 0;
		ptr = 0;
		for (i = 0; i < 1000; i++) {
			old_size = new_size;
			new_size += 4; 
			ptr = ys_reallocmem(ptr, old_size, new_size);
			*((char *)ptr+new_size-1) = 1;
		}
	}
	return 0;
}

#endif
