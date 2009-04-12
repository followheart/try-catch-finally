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

#ifndef alloc_h
#define alloc_h

struct ys_buftype_t {
	struct ys_buftype_t *next_buffer;
	char                *buffer;
};
typedef struct ys_buftype_t ys_buftype_t;

struct ys_buflink_t {
	struct ys_buflink_t *next;
};
typedef struct ys_buflink_t ys_buflink_t;

struct ys_allocator_t {
	ys_buftype_t  *buffer_list;
	ys_buflink_t  *free_list;
	char          *next_avail;
	char          *last;
	size_t        size;
	size_t        n;
	size_t        nbytes;
};
typedef struct ys_allocator_t ys_allocator_t;

extern ys_allocator_t *
ys_new_allocator(size_t size, size_t n);

extern void 
ys_grow_allocator(ys_allocator_t *a);

extern void *
ys_allocate(ys_allocator_t *a, size_t size);

extern void 
ys_deallocate(ys_allocator_t *a, void *n);

extern void 
ys_destroy_allocator(ys_allocator_t *a);

extern void *
ys_allocmem(size_t size);

extern void
ys_freemem(void *p, size_t size);

extern void *
ys_reallocmem(void *p, size_t osize, size_t nsize);

extern void
ys_destroyallmem(void);

extern unsigned long Maxmem;

#endif
