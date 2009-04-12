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
/***
 * Dibyendu Majumdar
 * DM 26-12-99 Created
 * DM 28-12-99 completed support for insertion of keys
 * DM 10-01-00 changed all integer types to a few well defined types
 * DM 16-01-00 Added arg to ys_pfn_btree_iterator_t.
 * DM 16-05-00 Added doccnt to the index. Now the pfn_index contains term,
 *             term freq (doccnt) and posting offset. 
 * DM 06-07-00 Fixed bugs in ys_btree_iterate().
 * DM 06-07-00 Re-introduced ys_block_insert() and ys_btree_insert() and
 *             fixed a few bugs.
 * DM 06-07-00 Added test harness to allow easy regression testing of the
 *             Btree code.
 * DM 07-07-00 Improved test harness - added new target testbtree to Makefile.
 * DM 07-07-00 Added prototypes for local functions and declared them static.
 * DM 07-07-00 Created a few new functions and moved some repeated code to
 *             these functions.
 * DM 07-12-02 Fixed bug in ys_block_curkey().
 */
/***
 * To test the code here in standalone mode, try:
 * make testbtree
 */

#include "btree.h"
#include "blockfile.h"
#include "util.h"

typedef struct {
	ys_uchar_t key[YS_MAXKEYSIZE+1];
	ys_filepos_t vl;
	ys_doccnt_t doccnt;
	ys_blocknum_t bn;
} ys_entry_t;

typedef struct {
	int prefix_len;
	int word_len;
	char word[256];
	ys_filepos_t posting_offset;
	ys_doccnt_t doccnt;
	ys_blocknum_t blockptr;
} ys_rec_t;

#ifdef TEST_BTREE 

static ys_keycnt_t Max_key_count = YS_BTREE_MAXKEYCOUNT/100;

static int Num_records = 0;
static int Keys_added = 0;
static int Keys_inserted = 0;
static int Keys_rewritten = 0;
static int Nodes_split = 0;
static int Keys_counted = 0;

#else

/* We use a variable so that we can change this (mostly for debugging) */
static ys_keycnt_t Max_key_count = YS_BTREE_MAXKEYCOUNT;

#endif

static inline ys_bool_t ys_block_isleaf(ys_block_t *block);
static inline ys_bool_t ys_block_iseof(ys_block_t *block);
static void ys_block_init_curkey(ys_block_t *block);
static void ys_block_set_curkey(ys_block_t *block, ys_uchar_t *key, ys_len_t prefix_len,
	ys_len_t key_len, ys_bool_t fullkey, ys_blocknum_t bn, ys_len_t bnsize, 
	ys_filepos_t vl, ys_len_t vlsize, ys_doccnt_t ct, ys_len_t ctsize);
static void ys_block_init(ys_block_t *block, ys_flag_t flags);
static inline size_t ys_calc_prefix_len(ys_uchar_t *s1, ys_uchar_t *s2);
static inline int ys_calc_bytesize(ys_uintmax_t n);
static void ys_block_calc_offsets(ys_block_t *block);
static ys_uchar_t * ys_block_curentry(ys_block_t *block);
static void ys_block_read_curkey(ys_block_t *block);
static inline int ys_block_curentry_len(ys_block_t *block);
static int ys_block_goto(ys_block_t *block, ys_keycnt_t k);
static inline void ys_block_gotop(ys_block_t *block);
static inline void ys_block_gobottom(ys_block_t *block);
static ys_uchar_t* ys_block_write_entry(ys_uchar_t *cp, ys_uchar_t *key, ys_len_t prefix_len,
	ys_len_t key_len, ys_blocknum_t bn, ys_len_t bnsize,
	ys_filepos_t vl, ys_len_t vlsize, ys_doccnt_t ct, ys_len_t ctsize);
static ys_block_t * ys_block_new(ys_blockfile_t *file, ys_bool_t isleaf);
static void ys_block_dump(ys_block_t *block);
static void ys_block_check_integrity(ys_block_t *block);
static ys_rec_t * ys_get_record(ys_file_t *fp, ys_bool_t yaseformat);
static ys_block_t * ys_block_getblock(ys_blockfile_t *file, ys_blocknum_t bn);
static inline int ys_key_compare(ys_uchar_t *key1, ys_uchar_t *key2);
static ys_bool_t ys_block_search(ys_block_t *block, ys_uchar_t *key, ys_filepos_t *valueptr,
	ys_doccnt_t *doccntptr, ys_blocknum_t *blockptr);
static int ys_block_insert(ys_block_t *block, ys_uchar_t *key, ys_blocknum_t bn, 
	ys_filepos_t vl, ys_doccnt_t ct);

/**
 * Tests whether a block is a leaf block.
 */
static inline ys_bool_t
ys_block_isleaf(ys_block_t *block)
{
	return (block->bd.flags & YS_BTREENODE_ISLEAF);
}

/**
 * Following exploits the fact that curknum is always keycount+1
 * when at EOF.
 */
static inline ys_bool_t
ys_block_iseof(ys_block_t *block)
{
	return (block->curknum > block->bd.keycount);
}

/**
 * Initialise current entry.
 */
static void
ys_block_init_curkey(ys_block_t *block)
{
	block->curvalue = 0;
	block->curptr = 0;
	block->curdoccnt = 0;
	block->curkey[0] = 0; 
	block->curprefix_len = 0;
	block->curkey_len = 0;
	block->curvalue_size = 0;
	block->curptr_size = 0;
	block->curdoccnt_size = 0;
}

/**
 * Set the current entry.
 */
static void
ys_block_set_curkey(ys_block_t *block, ys_uchar_t *key, ys_len_t prefix_len,
	ys_len_t key_len, ys_bool_t fullkey, ys_blocknum_t bn, ys_len_t bnsize, 
	ys_filepos_t vl, ys_len_t vlsize, ys_doccnt_t ct, ys_len_t ctsize)
{
	if (fullkey) {
		/* memcpy(block->curkey+1, key+1, block->curkey[0]); */
		memcpy(block->curkey+1, key+1, key[0]);
		block->curkey[0] = key[0];
	}
	else {
		memcpy(block->curkey+1+prefix_len, key, key_len); 
		block->curkey[0] = prefix_len+key_len; 
	}
	block->curptr = bn;
	block->curvalue = vl;
	block->curdoccnt = ct;
	block->curprefix_len = prefix_len;
	block->curkey_len = key_len;
	block->curptr_size = bnsize;
	block->curvalue_size = vlsize;
	block->curdoccnt_size = ctsize;
}

/**
 * Initialise a block.
 */
static void
ys_block_init(ys_block_t *block, ys_flag_t flags)
{
	block->curknum = 1;
	ys_block_init_curkey(block);
	/* block->offsets */

	block->bd.flags |= flags;
	block->bd.freespace = sizeof block->bd.keyspace;
	block->bd.keycount = 0;
	block->bd.lowernode = 0;
	/* block->bd.keyspace */
}

/**
 * Following assumes that each string has its length
 * specified in the first byte.
 */
static inline size_t
ys_calc_prefix_len(ys_uchar_t *s1, ys_uchar_t *s2)
{
	size_t n = 0;
	size_t len1 = *s1++, len2 = *s2++;
	size_t complen = len1 < len2 ? len1 : len2;
	for (n = 0; *s1 == *s2 && complen > 0; complen--, n++, s1++, s2++)
		;
	return n;
}

/**
 * Calculate the number of bytes would take to represent a number.
 */
static inline int
ys_calc_bytesize(ys_uintmax_t n)
{
	enum { BYTESIZE = 8 };
	return YS_DIVIDE_AND_ROUNDUP(ys_most_significant_bit(n)+1,BYTESIZE);
}

/* Following macros assist in writing/reading entries. 
 * To avoid inroducing problems where these macros are used, 
 * the code is enclosed in dummy do { } while(0) loops.
 * Care is also taken to ensure that apart from cp, none of the other
 * args change. cp is always incremented so that it is past the
 * area being processed.
 */

#define PROLOGUE_LEN	4
#define ENTRY_LEN(key_len,bnsize,vlsize,cntsize) \
	(PROLOGUE_LEN+key_len+bnsize+vlsize+cntsize)
#define WRITE_ENTRY_PROLOGUE(cp, pl, kl, bs, vs, cs, cnt) \
	do { \
		*cp++ = pl; \
		*cp++ = kl; \
		*cp++ = (ys_uchar_t) (0xf0&(bs<<4))|(0x0f&vs); \
		if (cnt <= 0x0f) \
			*cp++ = (ys_uchar_t) (0x0f&cnt); \
		else \
			*cp++ = (ys_uchar_t) (0xf0&(cs<<4)); \
	} while(0)
#define READ_ENTRY_PROLOGUE(cp, pl, kl, bs, vs, cs, cnt) \
	do { \
		pl = *cp++; \
		kl = *cp++; \
		bs = (*cp)>>4; \
		vs = (*cp++)&0x0f; \
		cs = (*cp)>>4; \
		if (cs == 0) \
			cnt = (*cp)&0x0f; \
		else \
			cnt = 0; \
		cp++; \
	} while(0)

#define WRITE_VAL_OR_PTR(cp, n, len) \
	do { \
		int __1__; \
		for (__1__ = 0; __1__ < len; __1__++, cp++) { \
			*cp = 0xff&(n>>(8*__1__)); \
		} \
	} while(0)

#define READ_VAL_OR_PTR(cp, n, len) \
	do { \
		int __1__; \
		for (n = 0, __1__ = 0; __1__ < len; __1__++, cp++) { \
			n |= ((*cp)<<(8*__1__)); \
		} \
	} while(0)

#define WRITE_KEY(cp, key, prefix_len, key_len) \
	do { \
		ys_uchar_t *__1__ = key+1+prefix_len; \
		int __2__; \
		for (__2__ = key_len; __2__ > 0; __2__--) \
			*cp++ = *__1__++; \
	} while(0)

/**
 * Calculate the offset of each entry (key/value pair) in the block.
 */
static void
ys_block_calc_offsets(ys_block_t *block)
{
	ys_uchar_t *cp, *ks;
	ys_keycnt_t k;
	
	assert(block->bd.keycount <= Max_key_count);

	ks = cp = block->bd.keyspace;
	for (k = 0; k < block->bd.keycount; k++) {
		ys_len_t prefix_len, key_len;
		ys_len_t bnsize, vlsize, ctsize;
		ys_doccnt_t ct;

		block->offsets[k] = cp - ks;
		READ_ENTRY_PROLOGUE(cp, prefix_len, key_len, bnsize, vlsize,
			ctsize, ct);
		cp += (key_len+bnsize+vlsize+ctsize);
	}
}

/**
 * Return a pointer to the current entry.
 */
static ys_uchar_t *
ys_block_curentry(ys_block_t *block)
{
	ys_uchar_t *cp;
	assert(block->curknum > 0 && block->curknum <= block->bd.keycount+1);

	if (block->curknum == block->bd.keycount+1)
		/* Beyond last key */
		cp = &block->bd.keyspace
			[sizeof block->bd.keyspace - block->bd.freespace];
	else
		cp = &block->bd.keyspace
			[block->offsets[block->curknum-1]];
	return cp;
}

/**
 * Read the current entry.
 */
static void
ys_block_read_curkey(ys_block_t *block)
{
	ys_uchar_t *cp, *key;
	ys_len_t prefix_len, key_len;
	ys_len_t bnsize, vlsize, ctsize;
	ys_blocknum_t bn;
	ys_filepos_t vl;
	ys_doccnt_t ct;
	
	assert(block->curknum > 0);
	assert(block->curknum <= block->bd.keycount+1);

	if (block->bd.keycount == 0 || ys_block_iseof(block)) {
		/* empty block or EOF */
		ys_block_init_curkey(block);
		return;
	}

	cp = ys_block_curentry(block);
	READ_ENTRY_PROLOGUE(cp, prefix_len, key_len, bnsize, vlsize,
		ctsize, ct);
	key = cp;
	cp += key_len;
	READ_VAL_OR_PTR(cp, bn, bnsize);
	READ_VAL_OR_PTR(cp, vl, vlsize);
	if (ctsize > 0)
		READ_VAL_OR_PTR(cp, ct, ctsize);
	ys_block_set_curkey(block, key, prefix_len, key_len, BOOL_FALSE,
		bn, bnsize, vl, vlsize, ct, ctsize);	
#if 0
	printf("curkey = '%.*s', dupcnt = %d, keylen = %d\n", 
		block->curkey[0], block->curkey+1, prefix_len, key_len);
#endif
}

/**
 * Calculate the length of the current entry.
 */
static inline int
ys_block_curentry_len(ys_block_t *block)
{
	if (ys_block_iseof(block))
		return 0;
	return ENTRY_LEN(block->curkey_len,block->curptr_size,
		block->curvalue_size,block->curdoccnt_size);
}

/**
 * Goto a particular entry in the block.
 * There are keycount entries - starting from 1.
 */
static int
ys_block_goto(ys_block_t *block, ys_keycnt_t k)
{
	assert(k <= block->bd.keycount+1);

	if (k == 0) {
		/* This can only be requested when the block is
		 * empty.
		 */
		assert(block->bd.keycount == 0);
		block->curknum = k = 1;
	}

	/* We need to scan keys from the beginning because keys
	 * are stored in a compressed form. However, we can optimise
	 * if the key to be read is further down, else we start
	 * scanning from 1.
	 */
	if (k < block->curknum)
		block->curknum = 1;
	if (block->curknum == 1)
		/* This could be the first request so we read the key anyway */
		ys_block_read_curkey(block);
	if (k == block->curknum) {
		return 0;
	}

	while (block->curknum < k) {
		block->curknum++;
		ys_block_read_curkey(block);
	}
	return 0;
}
	
/**
 * Goto the first entry in the block.
 */
static inline void
ys_block_gotop(ys_block_t *block)
{
	ys_block_goto(block, 1);
}

/**
 * Goto the last entry in the block.
 */
static inline void
ys_block_gobottom(ys_block_t *block)
{
	/* if keycount is 0, ys_block_goto() will adjust automatically */
	ys_block_goto(block, block->bd.keycount);
}

/**
 * Write an entry to the block.
 */
static ys_uchar_t *
ys_block_write_entry(ys_uchar_t *cp, ys_uchar_t *key, ys_len_t prefix_len,
	ys_len_t key_len, ys_blocknum_t bn, ys_len_t bnsize,
	ys_filepos_t vl, ys_len_t vlsize, ys_doccnt_t ct, ys_len_t ctsize)
{
	WRITE_ENTRY_PROLOGUE(cp, prefix_len, key_len, bnsize, vlsize, 
		ctsize, ct);
	WRITE_KEY(cp, key, prefix_len, key_len);
	WRITE_VAL_OR_PTR(cp, bn, bnsize);
	WRITE_VAL_OR_PTR(cp, vl, vlsize);
	if (ct > 0x0f)
		WRITE_VAL_OR_PTR(cp, ct, ctsize);
	return cp;
}

/**
 * Add an entry to the block.
 */
static int 
block_addentry(ys_block_t *block, ys_uchar_t *key, ys_blocknum_t bn, ys_filepos_t vl,
	ys_doccnt_t ct)
{
	ys_elen_t entry_len;
	ys_len_t prefix_len, key_len;
	ys_len_t bnsize, vlsize, ctsize;
	ys_uchar_t *cp;
	
	if (block->bd.keycount == Max_key_count)
		return -1;

	/* position at EOF */
	ys_block_gobottom(block);

	/* calculate various sizes */
	prefix_len = ys_calc_prefix_len(key, block->curkey);
	key_len = *key-prefix_len;
	if (bn == 0) bnsize = 0;
	else bnsize = ys_calc_bytesize(bn);
	if (vl == 0) vlsize = 0;
	else vlsize = ys_calc_bytesize(vl);
	if (ct <= 0x0f) ctsize = 0;
	else ctsize = ys_calc_bytesize(ct);
	entry_len = ENTRY_LEN(key_len,bnsize,vlsize,ctsize);

	/* is there space for the new entry ? */
	if (entry_len > block->bd.freespace)
		return -1;

	/* go past the current entry - if we are already at EOF,
	 * ys_block_curentry_len() will return 0.
	 */
	cp = ys_block_curentry(block);
	cp += ys_block_curentry_len(block);

	if (block->bd.keycount > 0)
		/* when keycount == 0, curknum is already 1 */
		block->curknum++;
	block->bd.keycount++;
	block->offsets[block->curknum-1] = cp - block->bd.keyspace;
	block->bd.freespace -= entry_len;

	ys_block_set_curkey(block, key, prefix_len, key_len, BOOL_TRUE,
		bn, bnsize, vl, vlsize, ct, ctsize);
	ys_block_write_entry(cp, key, prefix_len, key_len,
		bn, bnsize, vl, vlsize, ct, ctsize);	

#if _DUMP_TREE_READ_WRITE
	printf("block# %lu: wrote '%.*s(%d,%d)',%ld(%d),%ld(%d),%ld(%d)\n", 
		block->blocknum, block->curkey[0], block->curkey+1, block->curprefix_len,
		block->curkey_len, block->curvalue, block->curvalue_size,
		block->curptr, block->curptr_size, block->curdoccnt, block->curdoccnt_size);
#endif

	return 0;
}

/**
 * Create a new block.
 */
static ys_block_t *
ys_block_new(ys_blockfile_t *file, ys_bool_t isleaf)
{
	ys_block_t *block;

	block = ys_blockfile_new( file );
	if (block == NULL)
		return NULL;
	ys_block_init( block, isleaf ? YS_BTREENODE_ISLEAF : 0 );
	return block;
}

/**
 * Dump a block to stdout (mostly for debugging).
 */
static void
ys_block_dump(ys_block_t *block)
{
	ys_keycnt_t i, knum;
	ys_keyoffset_t space_used = 0;	/* internal checking */

	knum = block->curknum;
	printf("Block number = %ld\n", block->blocknum);
	printf("      keycount = %d\n", block->bd.keycount);
	printf("      lowernode = %ld\n", block->bd.lowernode);
	for (i = 1; i <= block->bd.keycount; i++) {
		ys_block_goto(block, i);
#if _DUMP_TREE_READ_WRITE
		printf("block# %lu: read '%.*s(%d,%d)',%ld(%d),%ld(%d),%ld(%d)\n", 
		block->blocknum, block->curkey[0], block->curkey+1, block->curprefix_len,
		block->curkey_len, block->curvalue, block->curvalue_size,
		block->curptr, block->curptr_size, block->curdoccnt, block->curdoccnt_size);
#endif

		space_used += ys_block_curentry_len(block);
		printf("      key # %d = '%.*s'", i, block->curkey[0], block->curkey+1);
		printf(", value = %ld", block->curvalue);
		printf(", doccnt = %ld\n", block->curdoccnt);
		printf(", ptr = %ld\n", block->curptr);
	}
	printf("free space calculated = %d\n", 
		sizeof block->bd.keyspace-space_used);
	printf("free space reported = %d\n", block->bd.freespace);
	assert((sizeof block->bd.keyspace)-space_used == block->bd.freespace);
	ys_block_goto(block, knum);
}

/**
 * Check a block's integrity.
 * This doesn't always work correctly.
 */
static void
ys_block_check_integrity(ys_block_t *block)
{
	ys_keycnt_t i, knum;
	ys_keyoffset_t space_used = 0;

	knum = block->curknum;
	for (i = 1; i <= block->bd.keycount; i++) {
		ys_block_goto(block, i);
		space_used += ys_block_curentry_len(block);
	}
	if ((sizeof block->bd.keyspace)-space_used != block->bd.freespace) {
		printf("Block number = %ld\n", block->blocknum);
		printf("      keycount = %d\n", block->bd.keycount);
		printf("      lowernode = %ld\n", block->bd.lowernode);
		printf("      total key space = %d\n", sizeof block->bd.keyspace);
		printf("      free space calculated = %d\n", 
 			sizeof block->bd.keyspace-space_used);
		printf("      free space reported = %d\n", block->bd.freespace);
		space_used = 0;
		for (i = 1; i <= block->bd.keycount; i++) {
			int len;
			ys_block_goto(block, i);
			len = ys_block_curentry_len(block);
			printf("      key # %d = '%.*s'", i, block->curkey[0], block->curkey+1);
			printf(", value = %ld", block->curvalue);
			printf(", doccnt = %ld", block->curdoccnt);
			printf(", ptr = %ld", block->curptr);
			printf(", offset = %d", block->offsets[i-1]);
			printf(", len = %d", len);
			printf(", calculated offset = %d\n", space_used);
			space_used += len;
		}
	}
	assert((sizeof block->bd.keyspace)-space_used == block->bd.freespace);
	ys_block_goto(block, knum);
}

/**
 * Read a record from either the yase.words file (yaseformat) or the
 * intermediate file created when building a tree.
 */
static ys_rec_t *
ys_get_record(ys_file_t *fp, ys_bool_t yaseformat)
{
	static ys_rec_t record;
	if (fp == NULL || ys_file_eof(fp))
		return NULL;
	if (yaseformat) {
		record.prefix_len = ys_file_getc(fp);
		if (ys_file_eof(fp)) return NULL;
		record.word_len = ys_file_getc(fp);
		ys_file_read(record.word+record.prefix_len, 1, 
			record.word_len, fp);
		record.word[record.prefix_len+record.word_len] = 0;
		ys_file_read(&record.posting_offset, 1, 
			sizeof record.posting_offset, fp);
		ys_file_read(&record.doccnt, 1, 
			sizeof record.doccnt, fp);
		record.blockptr = 0;
#ifdef TEST_BTREE
		Num_records++;
#endif
	}
	else {
		char buffer[1024];
		*buffer = 0;
		if (ys_file_gets(buffer, sizeof buffer, fp)) {
			strncpy(record.word, strtok(buffer, ":"), 
				sizeof record.word);
			record.posting_offset = atoi(strtok(NULL, ":"));
			record.doccnt = atoi(strtok(NULL, ":"));
			record.blockptr = atoi(strtok(NULL, ":\n"));
		}
		else
			return NULL;
	}
	if (ys_file_eof(fp)) return NULL;
	
	return &record;
}
	
/***
* Build a tree from the bottom up.
* First build the leaf nodes. Build a list of keys that should
* go to the parent nodes and save these to a temporary file.
* Once all leaf nodes are built, repeat the process, only this time
* use the temporary file as the source for keys, building the parent 
* nodes, while creating another temporary file to store grand-parent
* keys. Repeat until no more keys to process.
* The process progressively rises higher up the tree until the
* root node is built.
*/
int
ys_btree_create( const char *treename, const char *inputname )
{
	ys_file_t *fp = NULL;
	ys_file_t *tempfp = NULL;
	char tempfile[1024];
	int tempfnum = 0;
	ys_block_t *block = NULL;
	ys_blocknum_t rootblock = 0;
	ys_blocknum_t firstblocknum = 0;
	ys_blocknum_t lastblocknum = 0;
	ys_bool_t leaflevel = BOOL_TRUE;
	ys_blockfile_t *file;
	ys_block_t *headerblock = NULL;
	ys_header_t *header = NULL;

	file = ys_blockfile_open( treename, "w+" );
	if (file == NULL)
		return -1;

	fp = ys_file_open( inputname, "r", YS_FILE_ABORT_ON_ERROR );
	if (fp == NULL) {
		fprintf(stderr, "Failed to open file %s\n",
			inputname );
		return -1;
	}
	headerblock = ys_blockfile_new( file );

	while ( fp != NULL ) {
		ys_rec_t *rec;
		ys_file_rewind(fp);
		while ( (rec = ys_get_record(fp, leaflevel)) != NULL ) {

			ys_entry_t e;
			int status;

			e.key[0] = strlen(rec->word);
			memcpy(e.key+1, rec->word, e.key[0]+1);
			e.vl = rec->posting_offset;
			if (leaflevel) e.bn = 0;
			else e.bn = rec->blockptr;
			e.doccnt = rec->doccnt;

#ifdef TEST_BTREE
			if (leaflevel)
				Keys_added++;
#endif
				
			if (block == NULL) {
				block = ys_block_new(file, leaflevel);
				assert(block->ref == 1);
				lastblocknum = block->blocknum;
				block->bd.lowernode = firstblocknum;
				rootblock = block->blocknum;
			}

#ifdef TEST_BTREE
			/* When testing we force Max_key_count
			 * to be a small so that we can trigger
			 * node splits more often.
			 */
			if (block->bd.keycount == Max_key_count)
				status = -1;
			else
#endif
			status = block_addentry( block, e.key, e.bn, 
				e.vl, e.doccnt);
			if (status != 0) {
				if (tempfp == NULL) {
					sprintf(tempfile, "tmp.%d.btree", 
						++tempfnum);
					tempfp = ys_file_open(tempfile, "w+",
						YS_FILE_ABORT_ON_ERROR);
					firstblocknum = block->blocknum;
				}
				block->dirty = BOOL_TRUE;
				assert(block->ref == 1);
				ys_blockfile_put( file, block );
				block = ys_block_new( file, leaflevel );
				assert(block->ref == 1);
				lastblocknum = block->blocknum;
				block->bd.lowernode = e.bn;
				e.bn = block->blocknum;
				ys_file_printf(tempfp, "%s:%ld:%ld:%ld\n", 
					e.key+1, e.vl, e.doccnt, e.bn);
			}
		}
		ys_file_close(fp);
		fp = tempfp;
		tempfp = NULL;
		leaflevel = BOOL_FALSE;
		block->dirty = BOOL_TRUE;
		ys_blockfile_put( file, block );
		block = NULL;
	}
		
	header = (ys_header_t *)&headerblock->bd;
	header->root = rootblock;
	header->lastblock= lastblocknum;
	headerblock->dirty = BOOL_TRUE;
	ys_blockfile_put( file, headerblock );
	ys_blockfile_close( file );

	while (tempfnum > 0) {
		sprintf(tempfile, "tmp.%d.btree", 
			tempfnum--);
		remove(tempfile);
	}

	return 0;
}

/**
 * Create an empty tree.
 */
int
ys_btree_create_empty( const char *treename )
{
	ys_block_t *block = NULL;
	ys_blocknum_t rootblock = 0;
	ys_blocknum_t lastblocknum = 0;
	ys_blockfile_t *file = NULL;
	ys_block_t *headerblock = NULL;
	ys_header_t *header = NULL;

	file = ys_blockfile_open( treename, "w+" );
	if (file == NULL)
		return -1;

	headerblock = ys_blockfile_new( file );

	block = ys_block_new(file, BOOL_TRUE);
	lastblocknum = block->blocknum;
	rootblock = block->blocknum;

	block->bd.lowernode = 0;
	block->dirty = BOOL_TRUE;
	ys_blockfile_put( file, block );
		
	header = (ys_header_t *)&headerblock->bd;
	header->root = rootblock;
	header->lastblock = lastblocknum;
	headerblock->dirty = BOOL_TRUE;
	ys_blockfile_put( file, headerblock );

	ys_blockfile_close( file );

	return 0;
}

/**
 * Read a block from disk.
 */
static ys_block_t *
ys_block_getblock(ys_blockfile_t *file, ys_blocknum_t bn)
{
	ys_block_t *block;

	block = ys_blockfile_get( file, bn );
	block->curknum = 1;
	ys_block_init_curkey(block);
	/* block->offsets */
	ys_block_calc_offsets(block);

	return block;
}

/***
 * Dump the entire contents of a BTree.
 */
void
ys_btree_dump_blocks( const char *treename )
{
	ys_block_t *block = NULL;
	ys_blockfile_t *file;
	ys_block_t *headerblock = NULL;
	ys_header_t *header = NULL;
	ys_blocknum_t blocknum = 0;

	file = ys_blockfile_open( treename, "r" );
	if (file == NULL)
		return;
	headerblock = ys_blockfile_get( file, 1 );
	header = (ys_header_t *)&headerblock->bd;

	/* printf("root block = %lu\n", header->root); */
	for (blocknum = 2; blocknum <= header->lastblock; blocknum++) {
		block = ys_block_getblock( file, blocknum );
		ys_block_dump(block);
		printf("\n");
		ys_blockfile_put( file, block );
	}
	ys_blockfile_close(file);
}

/**
 * Open a BTree.
 */
ys_btree_t *
ys_btree_open_mode( const char *treename, const char *mode )
{
	ys_btree_t *tree;

	tree = calloc(1, sizeof(ys_btree_t));
	if (tree == NULL) {
		perror("calloc");
		fprintf(stderr, "Cannot ys_allocate a btree\n");
		return NULL;
	}
	tree->file = ys_blockfile_open( treename, mode );
	if (tree->file == NULL) {
		free(tree);
		return NULL;
	}
	tree->headerblock = ys_blockfile_get( tree->file, 1 );
	tree->header = (ys_header_t *)&tree->headerblock->bd;
	tree->root = tree->header->root;	
	return tree;
}

/**
 * Open a BTree.
 */
ys_btree_t *
ys_btree_open( const char *treename )
{
	return ys_btree_open_mode(treename, "r+");
}

/**
 * Close a BTree.
 */
int
ys_btree_close( ys_btree_t *tree )
{
	ys_blockfile_put( tree->file, tree->headerblock );	
	ys_blockfile_close( tree->file );
	free( tree );
	return 0;
}

/**
 * Compare two keys - each key must have its length specified in
 * the first byte.
 */
static inline int
ys_key_compare(ys_uchar_t *key1, ys_uchar_t *key2)
{
	ys_len_t len1, len2, len;
	int result;
	len1 = *key1++;
	len2 = *key2++;
	len = len1 < len2 ? len1 : len2;

	result = strncmp((char *)key1, (char *)key2, len);
	if (result == 0) 
		return (int)len1-(int)len2;
	return result;
}

/**
 * Search for a key within a block.
 */
static ys_bool_t
ys_block_search(ys_block_t *block, ys_uchar_t *key, ys_filepos_t *valueptr,
	ys_doccnt_t *doccntptr, ys_blocknum_t *blockptr)
{
	ys_keycnt_t i;
	*blockptr = block->bd.lowernode;
	for (i = 1; i <= block->bd.keycount+1; i++) {
		int result;
		ys_block_goto(block, i);
		if (ys_block_iseof(block))
			break;
		result = ys_key_compare(key, block->curkey);
		if (result == 0) {
			*valueptr = block->curvalue;
			*doccntptr = block->curdoccnt;
			*blockptr = block->blocknum;
			return BOOL_TRUE;
		}
		else if (result > 0) {
			*blockptr = block->curptr;
		}
		else {
			break;
		}
	}
	return BOOL_FALSE;
}

/**
 * Evaluate a user supplied function starting from the matching entry in the
 * BTree until the function returns BOOL_FALSE.
 */
int
ys_btree_iterate( ys_btree_t *tree, ys_uchar_t *key, ys_pfn_btree_iterator_t *pfunc, void *arg )
{
	ys_block_t *block;
	ys_blocknum_t blockptr = 0;
	enum { MAXSTACK = 20 };
	ys_blocknum_t stack[MAXSTACK];
	ys_keycnt_t kstack[MAXSTACK];
	ys_filepos_t dummy_value;
	ys_doccnt_t dummy_doccnt;
	int stack_top = -1;
	int kstack_top = -1;
	ys_bool_t found = BOOL_FALSE;
	ys_uchar_t k[sizeof block->curkey];

#define stack_push(item)  stack[++stack_top] = item
#define stack_is_empty()  (stack_top == -1)
#define stack_pop()       stack[stack_top--]
#define kstack_push(item)  kstack[++kstack_top] = item
#define kstack_is_empty()  (kstack_top == -1)
#define kstack_pop()       kstack[kstack_top--]

	memcpy(k, key, sizeof k);

	/* first do down to the leaf since all keys are inserted
	 * into leafs.
	 */
	block = ys_block_getblock( tree->file, tree->root );
	while (!ys_block_isleaf(block)) {
		found = ys_block_search(block, k, &dummy_value,
			&dummy_doccnt, &blockptr);
		if (found) 
			break;
		assert(blockptr != 0);
		stack_push(block->blocknum);
		kstack_push(block->curknum);
		ys_blockfile_put( tree->file, block );
		block = ys_block_getblock( tree->file, blockptr );
	}
	found = ys_block_search(block, k, &dummy_value, 
		&dummy_doccnt, &blockptr);

	if ( !ys_block_iseof(block) ) {
		if ( !pfunc( k, block->curkey, block->curvalue, 
			block->curdoccnt, arg ) )
			goto done;
	}
	else
		ys_block_gobottom( block );
	for (;;) {
		if ( ys_block_isleaf(block) ) {
			ys_block_goto( block, block->curknum+1 );			
			while ( ys_block_iseof( block ) ) {
				if (!stack_is_empty()) {
					blockptr = stack_pop();
					ys_blockfile_put( tree->file, block );
					block = ys_block_getblock( tree->file, 
						blockptr );
					ys_block_goto( block, kstack_pop() );
				}
				else
					break;
			}
		}
		else {
			stack_push( blockptr );	
			kstack_push( block->curknum+1 );	
			blockptr = block->curptr;
			ys_blockfile_put( tree->file, block );
			block = ys_block_getblock( tree->file, blockptr );
			while ( !ys_block_isleaf(block) ) {
				ys_blockfile_put( tree->file, block );
				stack_push( blockptr );	
				kstack_push( block->curknum );	
				blockptr = block->bd.lowernode;
				block = ys_block_getblock( tree->file, blockptr );
			}
			ys_block_goto(block, 1);
		}
		if ( ys_block_iseof(block) )
			break;
		if ( !pfunc( k, block->curkey, block->curvalue, 
			block->curdoccnt, arg ) )
			break;
	}
done:
	ys_blockfile_put( tree->file, block );

	return 0;
}


/**
 * Insert an entry into a block.
 * Following assumes that block->curknum is correctly positioned.
 */
static int 
ys_block_insert(ys_block_t *block, ys_uchar_t *key, ys_blocknum_t bn, ys_filepos_t vl,
	ys_doccnt_t ct)
{
	ys_elen_t entry_len = 0, newentry_len = 0, oldentry_len = 0;
	ys_len_t prefix_len = 0, prefix_len1 = 0, prefix_len2 = 0;
	ys_len_t key_len = 0, key_len1 = 0, key_len2 = 0;
	ys_blocknum_t bn2 = 0;
	ys_filepos_t vl2 = 0;
	ys_doccnt_t ct2 = 0, ctdummy = 0;
	ys_len_t bnsize = 0, bnsize2 = 0;
	ys_len_t vlsize = 0, vlsize2 = 0;
	ys_len_t ctsize = 0, ctsize2 = 0;
	ys_uchar_t *cp = 0, *ep = 0;
	ys_uchar_t keysaved[sizeof block->curkey] = {0};
	ys_keycnt_t i = 0;
	
	if (block->bd.keycount == Max_key_count)
		return -1;
	
	if (block->curknum == block->bd.keycount+1) {
		/* new entry will be the last one */
#if 0
		printf("inserting %.*s before EOF\n", key[0], key+1);
#endif
		return block_addentry(block, key, bn, vl, ct);
	}
	assert(block->curknum > 0 && block->curknum <= block->bd.keycount);
#if 0
	printf("inserting %.*s before %.*s\n", key[0], key+1,
		block->curkey[0], block->curkey+1);
#endif

#ifdef TEST_BTREE
	Keys_inserted++;
#endif	

	cp = ys_block_curentry(block);
	/* we will have to move current entry to the right.
	 * this may cause its content/size to change due to its
	 * relationship with the new entry. so we save the
	 * old entry now, and calculate its relationship with
         * the new entry. we will later on simply rewrite this
	 * entry.
	 * key_len1 = old key_len
	 * key_len2 = new key_len
	 * prefix_len1 = old prefix_len
	 * prefix_len2 = new prefix_len
	 */
	memcpy(keysaved, block->curkey, sizeof keysaved);
	prefix_len2 = ys_calc_prefix_len(key, block->curkey);
	key_len2 = *keysaved-prefix_len2;
	bn2 = block->curptr;
	vl2 = block->curvalue;
	ct2 = block->curdoccnt;
	READ_ENTRY_PROLOGUE(cp, prefix_len1, key_len1, bnsize2, vlsize2,
		ctsize2, ctdummy);

	/* oldentry_len is the length this entry used to consume.
	 * newentry_len is the new length (may be less or more).
	 */
	oldentry_len = ENTRY_LEN(key_len1,bnsize2,vlsize2,ctsize2);
	newentry_len = ENTRY_LEN(key_len2,bnsize2,vlsize2,ctsize2);

	/* now calculate size requirements for the new key being
	 * inserted. note that it is related to the curknum-1 entry.
	 */
	if (block->curknum == 1) {
		/* new entry will be the first one */
		key_len = *key;
		prefix_len = 0;
	}
	else {
		/*  we need to compare with previous entry */
		ys_block_goto(block, block->curknum-1);
		prefix_len = ys_calc_prefix_len(key, block->curkey);
		key_len = *key-prefix_len;
		ys_block_goto(block, block->curknum+1);
	}
	if (bn == 0) bnsize = 0;
	else bnsize = ys_calc_bytesize(bn);
	if (vl == 0) vlsize = 0;
	else vlsize = ys_calc_bytesize(vl);
	if (ct <= 0x0f) ctsize = 0;
	else ctsize = ys_calc_bytesize(ct);
	entry_len = ENTRY_LEN(key_len,bnsize,vlsize,ctsize);

	if (entry_len > block->bd.freespace)
		return -1;

	/* first we make room for the new entry */
	cp = ys_block_curentry(block);
	ep = &block->bd.keyspace[sizeof block->bd.keyspace - block->bd.freespace];
	memmove(cp+entry_len, cp, ep-cp);
	for (i = block->bd.keycount; i >= block->curknum; i--) {
		block->offsets[i] = block->offsets[i-1];
		block->offsets[i] += entry_len;
	}

	/* now we write the new entry */
	if (block->bd.keycount > 0)
		block->curknum++;
	block->bd.keycount++;
	block->bd.freespace -= entry_len;
	ys_block_set_curkey(block, key, prefix_len, key_len, BOOL_TRUE,
		bn, bnsize, vl, vlsize, ct, ctsize);
	cp = ys_block_write_entry(cp, key, prefix_len, key_len, 
		bn, bnsize, vl, vlsize, ct, ctsize);

	/* we have to rewrite the old entry in case its size or content has
	 * changed.
	 */
	if (newentry_len != oldentry_len || key_len1 != key_len2) {

		assert(block->offsets[block->curknum-1] == cp - block->bd.keyspace);
#ifdef TEST_BTREE
		Keys_rewritten++;
#endif	
#if 0
		printf("rewriting %.*s\n", keysaved[0], keysaved+1);
		printf("oldentry_len = %d, newentry_len = %d\n",
			oldentry_len, newentry_len);
		printf("freespace before = %d\n", block->bd.freespace);
#endif

		ep = &block->bd.keyspace[sizeof block->bd.keyspace - block->bd.freespace];
		if (ep > cp) 
			memmove(cp+newentry_len, cp+oldentry_len, 
				(ep-cp)-oldentry_len);
		ys_block_write_entry(cp, keysaved, prefix_len2, key_len2,
			bn2, bnsize2, vl2, vlsize2, ct2, ctsize2);
		for (i = block->curknum+1; i <= block->bd.keycount; i++) {
			block->offsets[i-1] += (oldentry_len-newentry_len);
		}
		block->bd.freespace += (oldentry_len-newentry_len);
#if 0
		printf("freespace after = %d\n", block->bd.freespace);
#endif
	}

	return 0;
}

/**
 * Insert an entry into the BTree.
 * This is a long and complicated function - I will break it up
 * into smaller functions once I have thoroughly tested it.
 *
 * ALGORITHM: 
 * Search for a key.
 * If not found, insert the key into the leaf block that should have
 * contained it. If successful, we are done. If, however, the leaf block 
 * is full, then split it into two before inserting the key. When 
 * splitting extract the median key.
 * Repeat above step with the parent node of the block/and the median key.
 * The insertion process can lead to a new root being created.
 */
int 
ys_btree_insert(ys_btree_t *tree, ys_uchar_t *key, ys_filepos_t value, ys_doccnt_t doccnt)
{
	/* We need a stack to remember the traversal path */
	enum { MAXSTACK = 20 };
	ys_blocknum_t stack[MAXSTACK] = {0};
	int stack_top = -1;

	ys_blocknum_t blockptr = 0;
	ys_blocknum_t leftchild = 0, rightchild = 0;
	ys_blocknum_t blocknum = 0;
	ys_bool_t found = BOOL_FALSE;
	ys_filepos_t dummy = 0;
	ys_doccnt_t doccnt2 = 0;
	ys_block_t *block = 0;
	ys_block_t *rightblock = 0;
	ys_uchar_t k[sizeof block->curkey] = {0};

#define stack_push(item)  stack[++stack_top] = item
#define stack_is_empty()  (stack_top == -1)
#define stack_pop()       stack[stack_top--]

	memcpy(k, key, sizeof k);

	/* First search for the key - if the key is absent we will end
	 * up at the leaf node that should have contained the key.
	 */

	block = ys_block_getblock( tree->file, tree->root );

	while (!ys_block_isleaf(block)) {
		found = ys_block_search(block, k, &dummy, &doccnt2, &blockptr);
		if (found) {
			ys_blockfile_put( tree->file, block );
			return 0;
		}
		assert(blockptr != 0);
		stack_push(block->blocknum);
		ys_blockfile_put( tree->file, block );
		block = ys_block_getblock( tree->file, blockptr );
	}
	found = ys_block_search(block, k, &dummy, &doccnt2, &blockptr);
	/* at the moment we do not allow duplicate keys */
	if (found) {
		ys_blockfile_put( tree->file, block );
		return 0;
	}

#ifdef TEST_BTREE
	Keys_added++;
#endif

	/* 
	 * Now, we need to insert the key, splitting the node if
	 * necessary. This will continue until the insert is successful,
	 * and can lead to a new root being created.
	 */

	blocknum = 0;
	for (;;) {
		int status = -1;
		ys_keyoffset_t total_space = 0, space_consumed = 0;
		ys_keycnt_t i = 0, m = 0;
		ys_uchar_t *cp = 0, *cp1 = 0, *ep = 0;
		ys_elen_t entry_len = 0;
		ys_len_t prefix_len = 0, key_len = 0;
		ys_len_t bnsize = 0, vlsize = 0, ctsize = 0;
		ys_uchar_t keysaved[sizeof block->curkey] = {0};
		ys_blocknum_t bn = 0;
		ys_filepos_t vl = 0;
		ys_doccnt_t ct = 0;

		status = ys_block_insert(block, k, blocknum, value, doccnt);
		if (status == 0) {
			/* all done */
			block->dirty = BOOL_TRUE;
			ys_blockfile_put( tree->file, block );
			break;
		}
#ifdef TEST_BTREE
		Nodes_split++;
#endif
		/* we will split the current block into two. */
		leftchild = block->blocknum;
		rightblock = ys_block_new( tree->file, ys_block_isleaf(block) );
		tree->header->lastblock = rightblock->blocknum;
		tree->headerblock->dirty = BOOL_TRUE;
		rightchild = rightblock->blocknum;
		
		/* we will try to move approximately half the keys
		 * from this block to the new one. since keys are
		 * variable length we will have to calculate the
		 * number of keys in terms of space used.
		 */
		total_space = sizeof block->bd.keyspace; 
		space_consumed = 0;
		for (i = 1; i <= block->bd.keycount; i++) {
			ys_block_goto(block, i);
			space_consumed += ys_block_curentry_len(block);
			if (space_consumed >= total_space/2)
				break;
		}
#ifdef TEST_BTREE
		if (i > block->bd.keycount)
			i = (block->bd.keycount+1)/2;
#endif
		assert(i <= block->bd.keycount-2);
		m = i++;	/* this key will go up to the parent node*/

		/* now let's calculate the space occupied by
		 * i+1 to keycount entries. the first key will have to
		 * be written in full, so we save relevant information.
		 */
		ys_block_goto(block, i);	
		prefix_len = block->curprefix_len;
		key_len = block->curkey_len;
		bnsize = block->curptr_size;
		vlsize = block->curvalue_size;
		ctsize = block->curdoccnt_size;
		bn = block->curptr;
		vl = block->curvalue;
		ct = block->curdoccnt;
		memcpy(keysaved, block->curkey, sizeof keysaved);
		entry_len = ENTRY_LEN(key_len,bnsize,vlsize,ctsize);
		
		cp = ys_block_curentry(block);
		ep = &block->bd.keyspace[sizeof block->bd.keyspace - 
			block->bd.freespace];
		space_consumed = (ep-cp)-entry_len;
		cp += entry_len;	
		block->bd.freespace += (space_consumed+entry_len);

		/* copy the entries - we need to expand the first key
		 * to its full size.
		 */
		ys_block_goto(rightblock, 1);
		cp1 = ys_block_curentry(rightblock);
		rightblock->bd.keycount = block->bd.keycount-i+1;	
		prefix_len = 0;
		key_len = keysaved[0];
		entry_len = ENTRY_LEN(key_len,bnsize,vlsize,ctsize);
		cp1 = ys_block_write_entry(cp1, keysaved, prefix_len, key_len,
			bn, bnsize, vl, vlsize, ct, ctsize);
		memcpy(cp1, cp, space_consumed);
		rightblock->bd.freespace -= (space_consumed+entry_len);
		ys_block_calc_offsets(rightblock);

		i = m;
		ys_block_goto(block, i);
		rightblock->bd.lowernode = block->curptr;
		rightblock->dirty = BOOL_TRUE;
		if (ys_key_compare(k, block->curkey) < 0) {
			ys_blockfile_put( tree->file, rightblock );
			rightblock = block;
		}

		/* save the median key - we will need to insert it
		 * into the parent node 
		 */
		memcpy(keysaved, block->curkey, sizeof keysaved);	
		vl = block->curvalue;
		ct = block->curdoccnt;
		cp = ys_block_curentry(block);
		block->bd.freespace += ys_block_curentry_len(block);
		block->bd.keycount = m-1;
		memset(cp, 0, ep-cp);	

		/* now that we have split the block, we can insert
		 * the key we were trying to.
		 */
		ys_block_search(rightblock, k, &dummy, &doccnt2, &blockptr);
		status = ys_block_insert(rightblock, k, blocknum, value, doccnt);
		block->dirty = BOOL_TRUE;
		assert(status == 0);

		/* prepare for inserting the median entry to the parent */
		memcpy(k, keysaved, sizeof k);
		blocknum = rightchild;
		value = vl;
		doccnt = ct;
		if (rightblock != block) {
			ys_blockfile_put( tree->file, rightblock );
		}
		ys_blockfile_put( tree->file, block );

		if (!stack_is_empty()) {
			blockptr = stack_pop();
			block = ys_block_getblock(tree->file, blockptr);
		}
		else {
			/* We have to create a new Root Node */
			block = ys_block_new(tree->file, BOOL_FALSE);
			blockptr = block->blocknum;
			block->bd.lowernode = leftchild;
			ys_block_goto(block, 1);
			tree->root = blockptr;
			tree->header->root = tree->root;
			tree->header->lastblock = tree->root;
			tree->headerblock->dirty = BOOL_TRUE;
		}
		ys_block_search(block, k, &dummy, &doccnt2, &blockptr);
	}
	return 0;
}

#ifdef TEST_BTREE

/**
 * This is regression test harness.
 * The test uses three input files.
 * The first file is test.btree.input1 - which is a small file.
 * The second file is test.btree.input2 - this file contains many more
 * entries including those in the first file.
 * The third file is test.btree.input3 - which has quite a few entries,
 * and some are duplicates of those in the previous two files.
 * The test proceeds by using the first file to create a tree. The
 * next two files are processed in insert mode.
 * Finally the entire tree is scanned and the keys counted. We also check
 * that the keys are in correct sort order and that there are no
 * duplicates.
 * For the test the Max_key_count is set to 1/100th of its normal
 * value to force lots of node splits.
 */

int
btree_test_insert( const char *treename, const char *inputname )
{
	ys_btree_t *tree = NULL;
	ys_file_t *fp = NULL;
	ys_rec_t *rec = NULL;
	ys_uchar_t key[YS_MAXKEYSIZE+1];

	tree = ys_btree_open( treename );
	fp = ys_file_open( inputname, "r", YS_FILE_ABORT_ON_ERROR );

	if (tree == NULL || fp == NULL) {
		fprintf(stderr, "Failed to open file %s\n",
			inputname );
		return -1;
	}

	while ( (rec = ys_get_record(fp, BOOL_TRUE)) != NULL ) {

		key[0] = strlen(rec->word);
		memcpy(key+1, rec->word, key[0]+1);
		if ( ys_btree_insert( tree, key, rec->posting_offset, 
			rec->doccnt ) != 0 ) {
			fprintf(stderr, "Failed to insert key '%s'\n",
				rec->word);
		}	
	}
	ys_file_close(fp);
	ys_btree_close(tree);

	return 0;
}

int
btree_test_create( const char *treename, const char *inputname )
{
	if ( ys_btree_create( treename, inputname ) != 0 ) {
		fprintf(stderr, "Failed to create a btree\n");
		return -1;
	}
	return 0;
}

ys_bool_t evaluate(ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t value, 
	ys_doccnt_t doccnt, void *arg)
{
	static ys_uchar_t key[YS_MAXKEYSIZE+1];
	if (key[0] != 0) {
		int rc = ys_key_compare(key2, key);
		if (rc <= 0) {
			printf("Comparison of %.*s with previous key "
				"%.*s failed\n", key2[0], key2+1, 
				key[0], key+1);
			return BOOL_FALSE;
		}
	}
	key[0] = key2[0];
	memcpy(key+1, key2+1, key[0]);
		
	/* printf("%.*s\n", key2[0], key2+1); */
	Keys_counted++;
	return BOOL_TRUE;
}

int
btree_test_count( const char *treename )
{
	ys_uchar_t key[YS_MAXKEYSIZE+1];
	ys_btree_t *tree = NULL;

	tree = ys_btree_open( treename );

	if (tree == NULL) {
		fprintf(stderr, "Failed to open file %s\n",
			treename );
		return -1;
	}

	strcpy(key+1, "");
	key[0] = strlen(key+1);
	ys_btree_iterate(tree, key, evaluate, 0);
	ys_btree_close(tree);

	return 0;
}

int main(int argc, const char *argv[]) 
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <input_create> [<input_insert>]" 
			" [<input_insert2>]\n",
			argv[0]);
		exit(1);
	}

	printf("Creating btree from %s ... ", argv[1]);
	if (btree_test_create( "test.btree", argv[1] ) != 0)
		goto error_exit;
	printf("%d records processed\n", Num_records);
	Num_records = 0;
	if (argc >= 3) {
		printf("Inserting keys from %s ... ", argv[2]);
		if (btree_test_insert( "test.btree", argv[2] ) != 0)
			goto error_exit;
		printf("%d records processed\n", Num_records);
		Num_records = 0;
	}
	if (argc == 4) {
		printf("Inserting more keys from %s ... ", argv[3]);
		if (btree_test_insert( "test.btree", argv[3] ) != 0)
			goto error_exit;
		printf("%d records processed\n", Num_records);
		Num_records = 0;
	}
	printf("Counting keys added\n");
	btree_test_count( "test.btree" );

	printf("Max_key_count was set to %d\n", Max_key_count);
	printf("Total number of keys added = %d\n", Keys_added);
	printf("Number of key insert operations = %d\n", Keys_inserted);
	printf("Number of key rewrite operations = %d\n", Keys_rewritten);
	printf("Number of nodes split = %d\n", Nodes_split);
	printf("Number of keys counted = %d\n", Keys_counted);
	if (Keys_added == Keys_counted) {
		printf("Test succeeded\n\n");
		return 0;
	}

error_exit:
	printf("\nTest failed\n");
	return 1;
}

#endif
