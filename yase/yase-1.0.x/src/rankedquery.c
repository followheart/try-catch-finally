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
/* DM 16-01-00 fixed bug related to matching all terms.
 * DM 17-01-00 modified ys_open_collection() so that files are opened
 *             readonly.
 * DM 17-01-00 added support for exact match.
 * DM 18-01-00 added support for stemming words.
 * DM 19-01-00 replaced fsetpos() and fgets() with getdocumentref().
 * DM 27-04-00 added support for ranked queries
 * DM 29-04-00 output format modified
 * DM 15-05-00 added support for paging through the results
 * DM 14-04-01 renamed a number of variables so that there is better
 *             correspondence with documentation
 * DM 21-04-01 created new file rankedquery.c
 *             renamed printit() to ys_find_docs()
 * DM 17-02-02 output is now generated using new ys_query_output_row() function.
 * DM 09-05-02 measure and show query elapsed time
 */

#include "query.h"
#include "rankedquery.h"
#include "queryin.h"
#include "queryout.h"
#include "makedb.h"
#include "stem.h"
#include "getword.h"
#include "util.h"

/**
 * As documents are retrived by the query, they are stored in ys_query_document_t
 * objects within a tree.
 */
typedef struct {
	ys_docnum_t docnum;	/* Document number */
	int count;		/* Number of query terms matched */
	int termcount;		
	float dwt;		/* The document weight */
	float rank;		/* Calculated rank */
	int weighted;		/* Has the document weight been retrieved ? */
} ys_query_document_t;

typedef struct {
	ys_query_document_t *d;
} ys_query_rank_t;

static int ys_compare_document(void *key, void *object);
static void ys_create_document(void *object, void *key);
static ys_bool_t ys_select_document( void *arg1, ys_docnum_t docnum, ys_doccnt_t dtf );
static int ys_compare_rank(void *key, void *object);
static void ys_create_rank(void *object, void *key);
static ys_bool_t ys_find_docs( ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t value, 
	ys_doccnt_t tf, void *arg);

static AVL_vtbl vtab = { ys_compare_document, ys_create_document, 0, 0 };
static AVL_vtbl vtab_rank = { ys_compare_rank, ys_create_rank, 0, 0 };
static struct timeval start, stop;

/**
 * Utility functions for the trees 
 */
static int 
ys_compare_document(void *key, void *object)
{
	ys_query_document_t *document = (ys_query_document_t*) object;
	ys_docnum_t *docnum = (ys_docnum_t*) key;
	if (*docnum == document->docnum)
		return 0;
	else if (*docnum > document->docnum)
		return 1;
	return -1;
}

static void 
ys_create_document(void *object, void *key)
{
	ys_query_document_t *document = (ys_query_document_t*) object;
	ys_docnum_t *docnum = (ys_docnum_t*) key;
	document->docnum = *docnum;
	document->count = 0;
	document->termcount = 0;
	document->dwt = 0.0;
	document->rank = 0.0;
	document->weighted = 0;
}

static int 
ys_compare_rank(void *key, void *object)
{
	ys_query_rank_t *v = (ys_query_rank_t*) object;
	ys_query_rank_t *k = (ys_query_rank_t*) key;
	if (k->d->rank == v->d->rank) {
		if (k->d->docnum == v->d->docnum)
			return 0;
		else if (k->d->docnum > v->d->docnum)
			return 1;
		else
			return -1;
	}
	else if (k->d->rank < v->d->rank)
		return 1;
	return -1;
}

static void 
ys_create_rank(void *object, void *key)
{
	ys_query_rank_t *v = (ys_query_rank_t*) object;
	ys_query_rank_t *k = (ys_query_rank_t*) key;
	v->d = k->d;
}

/**
 * Called for each document found for a query term. If processing a
 * ranked query - document weights are retrieved and stored. Data about
 * the document is stored in the result tree.
 */ 
static ys_bool_t
ys_select_document( void *arg1, ys_docnum_t docnum, ys_doccnt_t dtf )
{
	ys_collection_t *collection = (ys_collection_t *) arg1;
	ys_query_t *query = collection->query;
	ys_query_document_t *document;

	/* Add/update data about this document */
	document = (ys_query_document_t *) AVLTree_Insert( collection->avltree, &docnum );
	if (document != NULL) {

		if (document->termcount != query->curterm) {
			document->termcount = query->curterm;
			document->count++;
		}

		if (query->method == YS_QM_RANKED) {
			float idf, log_dtf, qtw;

			if (document->weighted == 0) {
				/* Retrieve document weight */
				if ( ys_dbgetdocweight( collection->doc_file, 
					document->docnum, &document->dwt ) != 0 ) {
					ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Error: cannot retrieve document weight\n");
					return BOOL_FALSE;
				}
#if _DUMP_RANKING
				ys_query_output_message(query->outp, YS_QRY_MSG_INFO, "Document # %ld, wt=%.2f\n", 
					document->docnum+1, document->dwt);	
#endif
				document->weighted = 1;
				query->matches++;
			}
		
			/* Calculate rank */
			log_dtf = CALC_LOGDTF(dtf);
			idf = query->terms[query->curterm-1].idf;
			qtw = query->terms[query->curterm-1].qtw;
			document->rank = document->rank + (log_dtf * idf * qtw);
		}
		else if (document->weighted == 0) {
			document->weighted = 1;
			query->matches++;
		}

	}
	else  {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Error: cannot insert into result tree\n");
		return BOOL_FALSE;
	}
	return BOOL_TRUE;
}

/**
 * Calculate query term weight.
 */
static void
ys_query_calculate_weight( ys_query_t *query, ys_doccnt_t tf, ys_docnum_t N )
{
	int i = query->curterm-1;

	query->terms[i].idf = CALC_IDF(N,tf);
	query->terms[i].qtw = (float) 
		CALC_QTW(query->terms[i].qtf, query->qmf,
			query->terms[i].idf);
#if _DUMP_RANKING
	ys_query_output_message(query->outp, 
		YS_QRY_MSG_INFO, "Term %s tf=%ld idf=%.2f qtf = %d, qtw = %.2f\n",
		query->terms[i].text, tf, query->terms[i].idf,
		query->terms[i].qtf, query->terms[i].qtw );
#endif
}

/**
 * Called for each term found in the index. Retrieves all documents
 * for the term and saves them in the result tree.
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
#if _TEST_FREQ_IN_TREE
		ys_doccnt_t fq = 0;
		ys_mkdb_get_term_frequency( collection->postings_file, position, 
			&fq );
		assert(tf == fq);
#endif
		ys_query_calculate_weight( query, tf, collection->N );
		snprintf(query->message+strlen(query->message),
			sizeof query->message-strlen(query->message),
			 "%.*s (%ld) ", 
			key1[0], key1+1, tf);	
		ys_mkdb_foreachposting( collection->postings_file, position, 
			ys_select_document, collection );
		return BOOL_TRUE;
	}

	return BOOL_FALSE;
}

/**
 * Evaluate a query. Search the terms and locate all documents 
 * containing those terms.
 */
int 
ys_ranked_query_evaluate( ys_collection_t *collection, ys_query_t *query )
{
	int i;

	gettimeofday(&start, (struct timezone *)0);
	collection->avltree = AVLTree_New( &vtab, sizeof(ys_query_document_t), 200 );
	if (collection->avltree == NULL) {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR,
			"Error: cannot create result tree\n");
		return -1;
	}

	query->matches = 0;
	query->curterm = 0;

	collection->query = query;
	/* Process each term in sequence */
	query->message[0] = 0;
	snprintf(query->message, sizeof query->message, "Hits: ");	
	for (i = 0; i < query->termcount; i++) {
	 	ys_uchar_t key[YS_MAXKEYSIZE+1];
		char *cp = query->terms[i].text;
		
		key[0] = strlen(cp);
		memcpy(key+1, cp, key[0]+1);
		if (collection->stemmed)
			stem(key);
		query->curterm++;
		ys_btree_iterate( collection->tree, key, ys_find_docs, collection );
	}
	snprintf(query->message+strlen(query->message),
		sizeof query->message-strlen(query->message),
		 "\n");
	ys_query_output_message(query->outp, YS_QRY_MSG_INFO,
		query->message);
	collection->query = 0;
	return 0;
}

/**
 * Output results of the query. If all_terms was specified, ignore matches
 * that didnot contain all the terms. If ranked query was requested, 
 * sort results by rank before sending them to the user.
 */
int 
ys_ranked_query_output_results( void *outp, ys_collection_t *collection, ys_query_t *query )
{
	ys_query_document_t *document;
	ys_docdata_t docfile, doc;
	AVLTree *rtree = 0;
	ys_query_rank_t *r;
	int retval = 0;
	int start_result = 1;
	int end_result;
	int cur_result = 0;
	int page_count = 1;

	/* In order to support paging we need to know how many
	 * documents were found. For an 'all_terms' query, some
	 * adjustment is necessary.
	 */
	if (query->method == YS_QM_ALL) {
		for (document = AVLTree_FindFirst(collection->avltree); 
			document != NULL;
			document = AVLTree_FindNext(collection->avltree, document)) {	
			if (query->curterm != document->count) {
				query->matches--;
			}
		}
	}

	if (query->matches == 0) {
		ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
			"No documents matched your query\n");
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

	if (query->method == YS_QM_RANKED) {
		/* We need to sort the results by rank */ 
		rtree = AVLTree_New( &vtab_rank, sizeof(ys_query_rank_t), 200 );
		if (rtree == 0) {
			ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
			"Error: cannot ys_allocate a tree\n");
			return -1;
		}
	}

	/* Process all document matches */
	for (document = AVLTree_FindFirst(collection->avltree); 
		document != NULL;
		document = AVLTree_FindNext(collection->avltree, document)) {	

		if (query->method == YS_QM_ALL) {
			/* Skip those documents which do not 
			 * contain all the query terms.
			 */
			if (query->curterm != document->count)
				continue;
		}
		if (query->method == YS_QM_RANKED) {
			ys_query_rank_t key;

			/* Normalise rank */
			document->rank = document->rank / document->dwt;
			key.d = document;

			/* Sort the document by rank */
			if (AVLTree_Insert(rtree, &key) == 0) {
				ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
			"Error: cannot insert data into a tree\n");
				retval = -1;
				goto exit;
			}
		}
		else {
			/* Send the document details to the user */
			int rc;
			if (++cur_result < start_result) 
				/* not in the page we want to display */
				continue;
			rc = ys_dbgetdocumentref(collection->doc_file, document->docnum,
		                   &docfile, &doc);
			if (rc == 0) {
				ys_query_output_row(outp, docfile.logicalname,
					doc.anchor, &docfile, &doc, 0.0, document->count);
				if (cur_result >= end_result)
					break;
			}
			else {
				ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
			"Error: cannot obtain data from YASE files\n");
				retval = -1;
				goto exit;
			}
		}
	}

	if (query->method == YS_QM_RANKED) {
		/* Display documents ordered by their rank */
		for (r = AVLTree_FindFirst(rtree); 
			r != NULL && cur_result < end_result;
			r = AVLTree_FindNext(rtree, r)) {	

			int rc;

			if (++cur_result < start_result) 
				/* not in the page we want to display */
				continue;

			/* Send the document details to the user */
			rc = ys_dbgetdocumentref(collection->doc_file, 
				r->d->docnum, &docfile, &doc);
			if (rc == 0) {
				ys_query_output_row(outp, docfile.logicalname,
					doc.anchor, &docfile, &doc, r->d->rank, r->d->count);
			}
			else {
				ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, 
			"Error: cannot obtain data from YASE files\n");
				retval = -1;
				goto exit;
			}
		}
	}

	gettimeofday(&stop, (struct timezone *)0);
	ys_query_output_summary(outp, query->matches, query->curpage,
		page_count, query->pagesize, 
		ys_calculate_elapsed_time(&start, &stop));

exit:
	if (rtree != 0)
		AVLTree_Destroy(rtree);
	if (collection->avltree != NULL) 
		AVLTree_Destroy( collection->avltree );	
	return 0;
}

