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
#ifndef btree_h
#define btree_h

#include "yase.h"
#include "list.h"
#include "blockfile.h"

/* ys_blocknum_t, ys_filepos_t and ys_doccnt_t are defined in yase.h */
typedef ys_uint16_t ys_keycnt_t;
typedef ys_uint16_t ys_keyoffset_t;
typedef ys_uint8_t  ys_len_t;
typedef ys_uint16_t ys_elen_t;
typedef ys_uint16_t ys_flag_t;

#define YS_MAXKEYSIZE  254
#define YS_BLOCKSIZE   8192

enum {
    YS_BTREENODE_ISLEAF = 1,
    YS_BTREENODE_ISROOT = 2
};

typedef struct {
        ys_flag_t      flags;		/* YS_BTREENODE_ISLEAF | YS_BTREENODE_ISROOT */
	ys_keyoffset_t freespace;
	ys_keycnt_t    keycount;
	ys_blocknum_t  lowernode;
#define YS_BTREE_MAXKEYSPACE (YS_BLOCKSIZE-sizeof(ys_flag_t)-sizeof(ys_keyoffset_t)-sizeof(ys_keycnt_t)-sizeof(ys_blocknum_t))
	ys_uchar_t     keyspace[YS_BTREE_MAXKEYSPACE];	
} ys_blockdata_t;

/*
* Each key/value (entry) pair is stored as follows:
* key {
*    unsigned dupcnt : 8;
*    unsigned len    : 8;
*    unsigned ptrsiz : 4;
*    unsigned valsiz : 4;
*    unsigned cntsiz : 4;            -- 0 if cnt <= 0x0f else > 0
*    unsigned cnt    : 4;            -- unused if cnt > 0x0f 
*    unsigned char key[len];
*    unsigned char ptr[ptrsiz];
*    unsigned char val[valsiz];
*    unsigned char doccnt[cntsiz];   -- only if cnt > 0x0f
* }
*/

#define YS_BTREE_MINENTRYSIZE	5 
#define YS_BTREE_MAXKEYCOUNT 	((YS_BTREE_MAXKEYSPACE)/YS_BTREE_MINENTRYSIZE)
enum {
	YS_BTREENODE_INVALID = 1,
	YS_BTREENODE_VALID = 2
};

struct ys_block_t {
	ys_link_t      link;
	ys_bool_t        dirty;
	ys_blocknum_t  blocknum;
	ys_int16_t     ref;

	ys_flag_t      status;	/* YS_BTREENODE_INVALID or YS_BTREENODE_VALID */

	/* curknum can range from 1 to bd.keycount+1 */
	ys_keycnt_t    curknum; 	

	/* following fields are for performance. we keep an uncompressed
	 * version of the current entry.
	 */
	ys_filepos_t   curvalue;
	ys_doccnt_t    curdoccnt;
	ys_blocknum_t  curptr;
	ys_uchar_t     curkey[YS_MAXKEYSIZE+1]; 
	ys_len_t       curprefix_len;
	ys_len_t       curkey_len;
	ys_len_t       curvalue_size;
	ys_len_t       curdoccnt_size;
	ys_len_t       curptr_size;

	/* To allow random access to keys we maintain their offsets
	 * from the beginning of bd.keyspace[]. This allows us to
	 * do binary searches for example. The array of offsets
	 * is populated when a block is read. It is also updated
	 * when keys are added, modified or deleted.
	 */
	ys_keyoffset_t offsets[YS_BTREE_MAXKEYCOUNT]; 
	ys_blockdata_t bd;
};

typedef struct {
	ys_blocknum_t root;
	ys_blocknum_t lastblock;
} ys_header_t;

typedef struct {
	ys_blocknum_t root;
	ys_block_t *headerblock;
	ys_header_t *header;
	ys_blockfile_t *file;
} ys_btree_t;

extern int
ys_btree_create_empty( const char *treename );

extern int
ys_btree_create( const char *treename, const char *inputfile );

extern void
ys_btree_dump_blocks( const char *treename );

extern ys_btree_t *
ys_btree_open( const char *treename );

extern ys_btree_t *
ys_btree_open_mode( const char *treename, const char *mode );

extern int
ys_btree_close( ys_btree_t *tree );

typedef ys_bool_t ys_pfn_btree_iterator_t(ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t value, 
	ys_doccnt_t doccnt, void *arg);
extern int
ys_btree_iterate( ys_btree_t *tree, ys_uchar_t *key, ys_pfn_btree_iterator_t *pfunc, void *arg );

#endif
