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

/* created 16-Jan 2000 from wquery.c */

#ifndef query_h
#define query_h

#include "yase.h"
#include "avl3.h"
#include "btree.h"
#include "bitfile.h"
#include "docdb.h"
#include "bitset.h"

typedef struct {
	char text[YS_TERM_LEN+1];   /* term */
	int qtf;                 /* number of times term appears
                                * a query.
                                */
	float qtw;               /* query term's weight */
	float idf;               /* term weight in the collection */
} ys_term_t;

enum {
	YS_QM_RANKED = 1,
	YS_QM_ALL = 2,
	YS_QM_BOOLEAN = 3
};

typedef struct {
	int termcount;                  /* number of terms in query */
	ys_term_t terms[YS_QUERY_MAXTERMS];		  /* query terms */
	char collection_path[1024];	  /* path to yase database files */
	char *http_referer;
	char hostname[128];             /* web server host */
	ys_bool_t exact_match;               /* match terms exactly ? */
	ys_bool_t dump_env;                  /* dump environment variables ? */
	ys_bool_t web_query;                 /* is this a web query ? */
	ys_bool_t stemmed;                   /* are we searching a stemmed 
                                         * database ? 
                                         */
	int  method;			/* YS_QM_RANKED, YS_QM_ALL, YS_QM_BOOLEAN */
	char *boolean_query_expr;	  /* boolean query expression */
	char *tokptr;			  /* boolean query lexer */
	int curtok;				  /* boolean query current token */
	ys_bitset_t *bitset;			  /* boolean query bitset */
	char termbuf[YS_TERM_LEN+1];       /* boolean query term */

	char direction;
	int pagesize;
	int curpage;
	int errors;

	int matches;
	int qmf;			/* max frequency within query */
	int curterm;

	void *outp;

	char *saved_query;
	char *saved_collection_path;

	char message[4096];		/* Generic message buffer */
} ys_query_t;

typedef struct {
	AVLTree *avltree;
	ys_bitfile_t *postings_file;
	ys_docdb_t *doc_file;
	ys_btree_t *tree;
	ys_query_t *query;
	ys_bool_t stemmed;
	ys_doccnt_t N;
	char errmsg[256];
} ys_collection_t;

extern int ys_open_collection( const char *home, ys_collection_t *collection );
extern int ys_close_collection( ys_collection_t *collection );
extern int ys_query_evaluate( ys_collection_t *collection, ys_query_t *query );
extern int ys_query_output_results( void *outp, ys_collection_t *collection, 
	ys_query_t *query );
extern void ys_prepare_output(char *buf, size_t bufsize, ys_docdata_t *docfile,
	ys_docdata_t *doc, ys_bool_t ranked, float rank, int matchcount);

#endif
