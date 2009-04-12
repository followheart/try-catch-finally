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
#ifndef blockfile_h
#define blockfile_h

#include "yase.h"
#include "ystdio.h"
#include "list.h"

typedef struct ys_blockfile_t ys_blockfile_t;
typedef struct ys_block_t ys_block_t;

struct ys_blockfile_t {
	ys_file_t *file;
	size_t lastblock;
	ys_list_t blocks;
};

extern ys_blockfile_t *
ys_blockfile_open( const char *name, const char *mode );

extern ys_block_t *
ys_blockfile_get( ys_blockfile_t *file, ys_blocknum_t blocknum );

extern int
ys_blockfile_put( ys_blockfile_t *file, ys_block_t *block );

extern ys_block_t *
ys_blockfile_new( ys_blockfile_t *file );

extern int 
ys_blockfile_close( ys_blockfile_t *file );

#endif
