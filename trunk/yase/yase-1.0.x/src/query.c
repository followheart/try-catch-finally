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

/* created 13-Jan 2000 */
/* last modified 14-Jan 2000 */
/* 21 apr 2001 - modified the way ys_query_output_message() is used. */

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
 * DM 21-04-01 split file into rankedquery.c and merged rest with query.c
 */

#include "query.h"
#include "boolquery.h"
#include "rankedquery.h"
#include "queryin.h"
#include "queryout.h"
#include "makedb.h"
#include "stem.h"
#include "getword.h"


/**
 * Open a document collection.
 */
int 
ys_open_collection(const char *home, ys_collection_t *collection)
{
	char path[1024];

	snprintf(path, sizeof path, "%s/%s", home, "yase.btree");
	collection->tree = ys_btree_open_mode( path, "r" );
	if (collection->tree == NULL) {
		snprintf(collection->errmsg, sizeof collection->errmsg, "Error: cannot open file %s\n", path);
		return -1;
	}

	collection->doc_file = ys_dbopen( home, "r", "" );
	if (collection->doc_file == NULL) {
		snprintf(collection->errmsg, sizeof collection->errmsg, "Error: cannot open YASE files\n");
		return -1;
	}
	else {
		ys_docnum_t N = 0, numfiles;
		ys_doccnt_t maxdoctermfreq, maxtermfreq;
		ys_bool_t stemmed;
		ys_dbgetinfo(collection->doc_file, &N, &numfiles,
			&maxdoctermfreq, &maxtermfreq, &stemmed);
		collection->N = N;
		collection->stemmed = stemmed;
	}

	snprintf(path, sizeof path, "%s/%s", home, "yase.postings");
	collection->postings_file = ys_bopen( path, "rb" );
	if (collection->postings_file == NULL) {
		snprintf(collection->errmsg, sizeof collection->errmsg, "Error: cannot open file %s\n", path );
		return -1;
	}

	collection->query = 0;

	return 0;
}

/**
 * Close a document collection.
 */
int 
ys_close_collection( ys_collection_t *collection )
{
	if (collection->tree != NULL) 
		ys_btree_close( collection->tree );
	if (collection->postings_file != NULL) 
		ys_bclose(collection->postings_file);
	if (collection->doc_file != NULL) 
		ys_dbclose(collection->doc_file);
	return 0;
}
	
int 
ys_query_evaluate( ys_collection_t *collection, ys_query_t *query )
{
	if (query->method == YS_QM_BOOLEAN) {
		return ys_boolean_query_evaluate( collection, query );
	}
	else {
		return ys_ranked_query_evaluate( collection, query );
	}
}

/**
 * Output results of the query. If all_terms was specified, ignore matches
 * that didnot contain all the terms. If ranked query was requested, 
 * sort results by rank before sending them to the user.
 */
int 
ys_query_output_results( void *outp, ys_collection_t *collection, ys_query_t *query )
{
	if (query->method == YS_QM_BOOLEAN) {
		return ys_boolean_query_output_results( outp, collection, query );
	}
	else {
		return ys_ranked_query_output_results( outp, collection, query );
	}
}

/**
 * Dummy implementation so that we can avoid linking getword.c and libxml to the
 * query tool.
 */
int ys_document_process(const char *logicalname, const char *physicalname, 
	ys_docdb_t *docdb, ys_pfn_index_t ptrfunc, ys_mkdb_t *arg) 
{
	assert(0);
	return -1;
}

/**
 * Dummy implementation so that we can avoid linking wget to the
 * query tool.
 */
int wget(int argc, char **argv, int (*func)(void *, const char *, const char *), void *arg) 
{
	assert(0);
	return -1;
}

/**
 * Dummy implementation so that we can avoid linking xmlparser module to the
 * query tool.
 */
void ys_xml_release_memory(void)
{
	assert(0);
}

int Ys_debug = 0;

int main(int argc, char *argv[])
{
	ys_query_t query = {0};
	void *outp;
	int rc = 0;
	ys_collection_t collection = {0};

	query.web_query = getenv("QUERY_STRING") != 0;

	/* We have to open the ouput stream first so that we can
	 * send error messages.
	 */
	outp = ys_query_output_open(argc, argv, query.web_query, &query);
	query.outp = outp;
	rc = ys_query_input( argc, argv, &query );
	if (rc < 0) {
		rc = 1;
		goto exit;
	}
	rc = 0;
	if ( ys_query_output_header( outp ) != 0 ) {
		rc = 1;
		goto exit;
	}
	if ( ys_open_collection( query.collection_path, &collection ) != 0 ) {
		ys_query_output_message( outp, YS_QRY_MSG_ERROR, collection.errmsg );
		rc = 1;
		goto exit;
	}
	if ( ys_query_evaluate( &collection, &query ) != 0 ) {
		rc = 1;
		goto exit;
	}
	if ( ys_query_output_results( outp, &collection, &query ) != 0 )
		rc = 1;

exit:
	ys_query_output_close(outp);
	ys_close_collection( &collection );

	return rc;
} 
