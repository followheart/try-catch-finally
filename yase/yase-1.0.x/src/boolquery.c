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
 * 15 April 2001 Created
 * 19-20 Feb 2002 Changed ys_get_token() to understand new 
 * keywords "and", "or" and "not".
 * Older tokens (&,| and !) are no longer supported.
 * 16 Mar 2002 Renamed functions.
 */

#include "query.h"
#include "boolquery.h"
#include "queryin.h"
#include "queryout.h"
#include "makedb.h"
#include "stem.h"
#include "util.h"

#include <ctype.h>

enum {
	T_LPAREN = 1,
	T_RPAREN,
	T_AND,
	T_OR,
	T_NOT,
	T_TERM,
	T_EOI
};

static ys_bitset_t *ys_expr(ys_collection_t *collection);
static ys_bitset_t *ys_or_expr(ys_collection_t *collection);
static ys_bitset_t *ys_and_expr(ys_collection_t *collection);
static ys_bitset_t *ys_not_expr(ys_collection_t *collection);
static int ys_get_token(ys_query_t *query);
static void ys_match_token(ys_query_t *query, int tok);
static ys_bool_t ys_select_document( void *arg1, ys_docnum_t docnum, ys_doccnt_t dtf );
static ys_bool_t ys_find_docs( ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t position, ys_doccnt_t tf, 
	void *arg );
static void ys_find_terms(ys_collection_t *collection, ys_query_t *query);
static ys_bool_t ys_boolean_iterator( void *argp, unsigned docnum, ys_bool_t is_set );

static struct timeval start, stop;

static int
ys_get_token(ys_query_t *query)
{
	char *cp = query->tokptr;
	char ch;
	int tok;
	int len = 0;

	query->termbuf[0] = 0;
	while (*cp && isspace(*cp))
		cp++;
	if (*cp == 0) {
		query->tokptr = cp;
		query->curtok = T_EOI;
		/* printf("EOI\n"); */
		return T_EOI;
	}

	len = 0;
	switch (ch = *cp++) {
	case '(': tok = T_LPAREN; query->termbuf[len++] = ch; break;
	case ')': tok = T_RPAREN; query->termbuf[len++] = ch; break;
	case '&': tok = T_AND; query->termbuf[len++] = ch; break;
	case '|': tok = T_OR; query->termbuf[len++] = ch; break;
	case '!': tok = T_NOT; query->termbuf[len++] = ch; break;
	default:
		if ((ch == 'a' || ch == 'A') && 
		    (strncasecmp(cp, "nd", 2) == 0 && 
		    (isspace(cp[2]) || cp[2] == '(' || cp[2] == ')'))) {
			tok = T_AND; 	
			strncpy(query->termbuf, "and", sizeof query->termbuf);
			cp += 2;
			len = 3;
		} 
		else if ((ch == 'o' || ch == 'O') && 
			 (*cp == 'r' || *cp == 'R') && 
		    	 (isspace(cp[1]) || cp[1] == '(' || cp[1] == ')')) {
			tok = T_OR; 	
			strncpy(query->termbuf, "or", sizeof query->termbuf);
			cp++;
			len = 2;
		}
		else if ((ch == 'n' || ch == 'N') &&
			 (strncasecmp(cp, "ot", 2) == 0 && 
			 (isspace(cp[2]) || cp[2] == '(' || cp[2] == ')'))) {
			tok = T_NOT; 	
			strncpy(query->termbuf, "not", sizeof query->termbuf);
			cp += 2;
			len = 3;
		}
		else {
			query->termbuf[len++] = ch;
			while (*cp && !isspace(*cp) &&
				*cp != '(' && *cp != ')') {

				query->termbuf[len++] = *cp++;
				if (len+1 >= sizeof query->termbuf) {
					ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Error: Term is too long\n");
					return T_EOI;
				}
			}
			tok = T_TERM;
		}
	}
	query->termbuf[len] = 0;
	query->tokptr = cp;
	query->curtok = tok;

	if (Ys_debug)
	{
		const char *tokname;
		switch (tok) {
			case T_LPAREN: tokname = "T_LPAREN"; break;
			case T_RPAREN: tokname = "T_RPAREN"; break;
			case T_AND: tokname = "T_AND"; break;
			case T_OR: tokname = "T_OR"; break;
			case T_NOT: tokname = "T_NOT"; break;
			case T_TERM: tokname = "T_TERM"; break;
			default: tokname = "T_EOI"; break;
		}

		printf("%s: %s:%s\n", __func__, tokname, query->termbuf);
	}

	return tok;
}

/**
 * Advance to next token if current token is matched.
 */
static void
ys_match_token(ys_query_t *query, int tok)
{
	if (query->curtok == tok)
		ys_get_token(query);
	else {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Syntax error: Unexpected token '%s'\n", query->termbuf);
		query->curtok = T_EOI;
	}
}

/**
 * Parse OR expressions.
 */
static ys_bitset_t *
ys_or_expr(ys_collection_t *collection)
{
	ys_bitset_t *bs1, *bs2;
	ys_query_t *query = (ys_query_t *)collection->query;

	bs1 = ys_and_expr(collection);
	while (bs1 != 0 && query->curtok == T_OR) {
		ys_match_token(query, T_OR);
		bs2 = ys_and_expr(collection);
		if (bs2 == 0) {
			return 0;
		}
		/* printf("performing OR\n"); */
		ys_bs_union(bs1,bs2);
		ys_bs_destroy(bs2);
	}
	return bs1;
}

/**
 * Parse AND expressions.
 */
static ys_bitset_t *
ys_and_expr(ys_collection_t *collection)
{
	ys_bitset_t *bs1, *bs2;
	ys_query_t *query = (ys_query_t *)collection->query;

	bs1 = ys_not_expr(collection);
	while (bs1 != 0 && query->curtok == T_AND) {
		ys_match_token(query, T_AND);
		bs2 = ys_not_expr(collection);
		if (bs2 == 0) {
			return 0;
		}
		/* printf("performing AND\n"); */
		ys_bs_intersect(bs1,bs2);
		ys_bs_destroy(bs2);
	}
	return bs1;
}

static ys_bitset_t *
ys_not_expr(ys_collection_t *collection)
{
	ys_bitset_t *bs1 = 0;
	ys_query_t *query = (ys_query_t *)collection->query;

	if (query->curtok == T_NOT) {
		ys_match_token(query, T_NOT);
		bs1 = ys_not_expr(collection);
		if (bs1 == 0) {
			return 0;
		}
		/* printf("performing NOT\n"); */
		ys_bs_complement(bs1);
	}
	else if (query->curtok == T_LPAREN) {
		ys_match_token(query, T_LPAREN);
		bs1 = ys_expr(collection);
		ys_match_token(query, T_RPAREN);
	}
	else if (query->curtok == T_TERM) {
		bs1 = ys_bs_alloc(collection->N);
		query->bitset = bs1;
		ys_find_terms(collection, query);
		query->bitset = 0;
		ys_match_token(query, T_TERM);
	}
	else {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Syntax error: Unexpected input\n", query->termbuf);
		bs1 = 0;
	}
		
	return bs1;
}

static ys_bitset_t *
ys_expr(ys_collection_t *collection)
{
	return ys_or_expr(collection);
}

/**
 * This function is called for each document found for a term.
 */
static ys_bool_t
ys_select_document( void *arg1, ys_docnum_t docnum, ys_doccnt_t dtf )
{
	ys_collection_t *collection = (ys_collection_t *) arg1;
	ys_query_t *query = collection->query;

	assert(query->bitset != 0);
	/* Add document to the bitmap for this term */
	ys_bs_addmember(query->bitset, docnum);

	return BOOL_TRUE;
}

/**
 * Called for each term found in the index. Retrieves all documents
 * for the term and saves them in the result bitset.
 * Note that this function may be called to process exact as well as partial
 * matches. It is called for all terms after the first term that is found
 * - until it returns BOOL_FALSE. 
 */
static ys_bool_t 
ys_find_docs( ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t position, ys_doccnt_t tf, 
	void *arg )
{
	ys_collection_t *collection = (ys_collection_t *)arg;
	ys_query_t *query = (ys_query_t *)collection->query;

	if (query->exact_match && key1[0] != key2[0])
		return BOOL_FALSE;

	if (strncmp( key1+1, key2+1, key1[0] ) == 0) {
		ys_mkdb_foreachposting( collection->postings_file, position, 
			ys_select_document, collection );
		return BOOL_TRUE;
	}

	return BOOL_FALSE;
}

/**
 * Search for a term.
 */
static void
ys_find_terms(ys_collection_t *collection, ys_query_t *query)
{
 	ys_uchar_t key[YS_MAXKEYSIZE+1];
	char *cp = query->termbuf;
		
	/* printf("searching term %s\n", query->termbuf); */
	key[0] = strlen(cp);
	memcpy(key+1, cp, key[0]+1);
	if (collection->stemmed)
		stem(key);
	ys_btree_iterate( collection->tree, key, ys_find_docs, collection );
}

int 
ys_boolean_query_evaluate( ys_collection_t *collection, ys_query_t *query )
{
	collection->query = query;
	query->tokptr = query->boolean_query_expr;
	gettimeofday(&start, (struct timezone *)0);
	ys_get_token(query);
	/* printf("parsing expr\n"); */
	query->bitset = 0;
	if (query->curtok != T_EOI) {
		query->bitset = ys_expr(collection);
		if (query->curtok != T_EOI) {
			ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Syntax error in expression at '%s'\n", query->termbuf);
			query->bitset = 0;
		}
	}
	collection->query = 0;
	return 0;
}

typedef struct {
	int start_result;
	int end_result;
	int cur_result;
	void *outp;
	ys_collection_t *collection;
	ys_query_t *query;
} arg_t;

static ys_bool_t 
ys_boolean_iterator( void *argp, unsigned docnum, ys_bool_t is_set )
{
	arg_t *arg = (arg_t *)argp;
	ys_docdata_t docfile, doc;
	int rc;

	if (++arg->cur_result < arg->start_result) 
		/* not in the page we want to display */
		return BOOL_TRUE;
	rc = ys_dbgetdocumentref(arg->collection->doc_file, docnum,
                  &docfile, &doc);
	if (rc == 0) {
		ys_query_output_row(arg->outp, docfile.logicalname,
			doc.anchor, &docfile, &doc, 0.0, 0);
	}
	else {
		ys_query_output_message(arg->outp, YS_QRY_MSG_ERROR, "Error: cannot obtain data from YASE files\n");
		return BOOL_FALSE;
	}
	if (arg->cur_result >= arg->end_result)
		return BOOL_FALSE;
	return BOOL_TRUE;
}

int 
ys_boolean_query_output_results( void *outp, ys_collection_t *collection, ys_query_t *query )
{
	int start_result = 1;
	int end_result;
	int cur_result = 0;
	int page_count = 1;
	arg_t arg = {0};

	/* In order to support paging we need to know how many
	 * documents were found. 
	 */
	query->matches = query->bitset != 0 ? ys_bs_count(query->bitset) : 0;

	if (query->matches == 0) {
		ys_query_output_message(outp, YS_QRY_MSG_INFO, "No documents matched your query\n");
		goto exit;
	}
		
	if (query->pagesize > 0) {
		page_count = YS_DIVIDE_AND_ROUNDUP(query->matches,query->pagesize);
		if (query->curpage > page_count)
			query->curpage = page_count;
		else if (query->curpage <= 0)
			query->curpage = 1;
		start_result = query->pagesize * (query->curpage-1) + 1;
		if (start_result > query->matches)
			start_result = query->matches - query->pagesize;
		if (start_result <= 0)
			start_result = 1;
		end_result = start_result + query->pagesize - 1;
	}
	else {
		start_result = 1;
		end_result = query->matches;
	}

	arg.start_result = start_result;
	arg.end_result = end_result;
	arg.cur_result = cur_result;
	arg.outp = outp;
	arg.collection = collection;
	arg.query = query;

	ys_bs_iterate( query->bitset, YS_BS_ITER_MEMBERS, ys_boolean_iterator, &arg );

	gettimeofday(&stop, (struct timezone *)0);
	ys_query_output_summary(outp, query->matches, query->curpage,
		page_count, query->pagesize, ys_calculate_elapsed_time(&start, &stop));

exit:
	return 0;
}
