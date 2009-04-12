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

#include "blockfile.h"
#include "btree.h"

ys_blockfile_t *
ys_blockfile_open( const char *name, const char *mode )
{
	struct stat statbuf;
	ys_blockfile_t *file;
	int i;

	file = (ys_blockfile_t*) calloc(1, sizeof(ys_blockfile_t));
	if (file == NULL) {
		perror("calloc");	
		fprintf(stderr, "Unable to allocate a blockfile\n");
		return NULL;
	}
	file->file = ys_file_open(name, mode, YS_FILE_ABORT_ON_ERROR);
	if (file->file == NULL) {
		fprintf(stderr, "Unable to open file %s\n", name);
		free(file);
		return NULL;
	}
	if (stat(name, &statbuf) < 0) {
		perror("stat");
		fprintf(stderr, "Unable to get information about %s\n",
			name);
		ys_file_close(file->file);
		free(file);
		return NULL;
	}
	file->lastblock = statbuf.st_size/sizeof(ys_blockdata_t);
	assert(file->lastblock*sizeof(ys_blockdata_t) == statbuf.st_size);

	ys_list_init( &file->blocks );
	for (i = 0; i < 25; i++) {
		ys_block_t *block;
		block = (ys_block_t *)calloc(1, sizeof(ys_block_t));
		if (block == NULL) {
			perror("calloc");
			fprintf(stderr, "Unable to allocate a block\n");
			ys_blockfile_close(file);
			return NULL;
		}
		block->blocknum = 0;
		block->dirty = BOOL_FALSE;
		ys_list_append( &file->blocks, block );
	}

	return file;
}

static ys_block_t *
ys_blockfile_find( ys_blockfile_t *file, ys_blocknum_t blocknum )
{
	ys_block_t *block;

	for (block = (ys_block_t *)ys_list_first( &file->blocks ); 
	     block != NULL;
	     block = (ys_block_t *)ys_list_next( &file->blocks, block )) {
		if (block->blocknum == blocknum) {
			return block;
		}
	}
	return NULL;
}

static int
ys_blockfile_write( ys_blockfile_t *file, ys_block_t *block )
{
	ys_filepos_t pos;
	if ( !block->dirty )
		return 0;
	pos = (ys_filepos_t)(block->blocknum-1) * sizeof(ys_blockdata_t);
	if ( ys_file_setpos( file->file, &pos ) < 0 ) {
		fprintf(stderr, "Unable to seek to pos %lu block %lu\n",
			pos, block->blocknum);
		return -1;
	}
	if ( ys_file_write( &block->bd, 1, sizeof block->bd, file->file )
		!= sizeof block->bd ) {
		fprintf(stderr, "Unable to write block %lu\n",
			block->blocknum);
		return -1;
	}
	block->dirty = BOOL_FALSE;
	return 0;
}	

static int
ys_blockfile_read( ys_blockfile_t *file, ys_block_t *block )
{
	ys_filepos_t pos;
	pos = (ys_filepos_t)(block->blocknum-1) * sizeof(ys_blockdata_t);
	if ( ys_file_setpos( file->file, &pos ) < 0 ) {
		fprintf(stderr, "Unable to seek to pos %lu block %lu\n",
			pos, block->blocknum);
		return -1;
	}
	if ( ys_file_read( &block->bd, 1, sizeof block->bd, file->file )
		!= sizeof block->bd ) {
		fprintf(stderr, "Unable to read block %lu\n",
			block->blocknum);
		return -1;
	}
	block->dirty = BOOL_FALSE;
	return 0;
}	

static ys_block_t *
ys_blockfile_getempty( ys_blockfile_t *file )
{
	ys_block_t *block;

	block = ys_blockfile_find( file, 0 );
	if (block != NULL)
		return block;
	for (block = (ys_block_t *)ys_list_last( &file->blocks ); 
	     block != NULL;
	     block = (ys_block_t *)ys_list_prev( &file->blocks, block )) {
		if (block->ref == 0) {
			break;
		}
	}
	if (block == NULL)
		return NULL;
	if (ys_blockfile_write( file, block ) == 0 ) {
		return block;
	}
	return NULL;
}

static void
ys_blockfile_make_lru( ys_blockfile_t *file, ys_block_t *block )
{
	ys_block_t *firstblock;

	firstblock = (struct ys_block_t *) ys_list_first( &file->blocks );
	if (block != firstblock) {	
		ys_list_remove( &file->blocks, block );
		ys_list_insert_before( &file->blocks, 
			firstblock,
			block );
	}
}

ys_block_t *
ys_blockfile_get( ys_blockfile_t *file, ys_blocknum_t blocknum )
{
	ys_block_t *block;

	block = ys_blockfile_find( file, blocknum );
	if (block != NULL) {
		block->ref++;
		ys_blockfile_make_lru( file, block );
		return block;
	}
	block = ys_blockfile_getempty( file );
	if (block == NULL)
		return NULL;
	block->blocknum = blocknum;
	block->dirty = BOOL_FALSE;
	block->ref = 1;
	if (ys_blockfile_read( file, block ) == 0) {
		ys_blockfile_make_lru( file, block );
		return block;
	}
	return NULL;
}

int
ys_blockfile_put( ys_blockfile_t *file, ys_block_t *block )
{
	assert( block->ref > 0 );
	block->ref--;
	return 0;
}

ys_block_t *
ys_blockfile_new( ys_blockfile_t *file )
{
	ys_block_t *block;

	block = ys_blockfile_getempty( file );
	if (block == NULL)
		return NULL;
	block->blocknum = ++file->lastblock;
	block->dirty = BOOL_FALSE;
	block->ref = 1;
	memset(&block->bd, 0, sizeof block->bd);
	ys_blockfile_make_lru( file, block );
	return block;
}

int 
ys_blockfile_close( ys_blockfile_t *file )
{
	ys_block_t *block;
	int rc = 0;
	int count = 0;

	while ( (block = (ys_block_t *)ys_list_first( &file->blocks )) != NULL ) {
		if (block->ref != 0)
			count++;
		if (block->blocknum != 0) {
			if (ys_blockfile_write( file, block ) != 0) {
				rc = -1;
			}
		}
		ys_list_remove( &file->blocks, block );
		free(block);
	}
	if (ys_file_close(file->file) != 0) {
		rc = -1;
		fprintf(stderr, "Error closing block file\n");
	}
	free(file);
	if (count)
		fprintf(stderr, "warning: %d blocks were still in use\n",
			count);
	
	return rc;
}
