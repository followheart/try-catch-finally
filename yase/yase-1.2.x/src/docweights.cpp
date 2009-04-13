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

#include "yase.h"
#include "makedb.h"
#include "docdb.h"
#include "btree.h"
#include "postfile.h"
#include "formulas.h"
#include "collection.h"
#include "docweights.h"

typedef struct docwt_t {
	float *weights;
	ys_doccnt_t *d_maxdtfs;
	ys_docnum_t N;
	ys_doccnt_t maxtf;
	ys_doccnt_t maxdtf;
	ys_uchar_t *term;
	ys_filepos_t position;
	ys_doccnt_t tf;
	ys_doccnt_t dtf;
	ys_docdb_t *docdb;
	YASENS PostFile *postings;
} docwt_t;

static docwt_t *ys_docweight_allocate( ys_docnum_t N );
static void ys_docweight_add_tf( docwt_t *dw, ys_docnum_t docnum, 
	ys_doccnt_t dtf, ys_doccnt_t tf, char *word );
static int ys_docweight_calculate_and_write( docwt_t *dw, ys_docdb_t *db );

/**
 * This function allocates an array of floats used to accumulate document 
 * weights.
 * NOTE: This function assumes the ALL documents have been scanned and therefore
 * doc->N provides N (collection size).
 */
static docwt_t *
ys_docweight_allocate( ys_docnum_t N )
{
	docwt_t * dw = (docwt_t *) calloc(1, sizeof(docwt_t));
	if (dw == 0) {
		fprintf(stderr, "Error allocating memory\n");
		return 0;
	}
	dw->weights = (float *) calloc(N, sizeof(float));
	dw->d_maxdtfs = (ys_doccnt_t *) calloc(N, sizeof(ys_doccnt_t));
	if (dw->weights == 0) {
		fprintf(stderr, "Error allocating memory\n");
		free(dw);
		return 0;
	}
	dw->N = N;
	return dw;
}

/**
* This function adds a weight to the document based upon the document term 
* frequency (dtf) and inverse document frequency (idf).
* This function is called for every document/term combination during the final 
* merge process.
* NOTE: This function assumes the ALL documents have been scanned and therefore
* doc->N provides N (collection size).
*/
static void
ys_docweight_add_tf( docwt_t *dw, ys_docnum_t docnum,
	ys_doccnt_t dtf, ys_doccnt_t tf, char *word )
{
	double idf = ys_idf(dw->N,tf,dw->N);
	double dtw = ys_dtw(idf, dtf, 0);
#if _DUMP_CALCWEIGHT
	printf("term(%s) doc(%lu) tf(%lu) dtf(%lu) idf(%.2f) dtw(%.2f)\n",
		word, docnum, tf, dtf, idf, dtw);
#endif
	dw->weights[docnum] += (float)(dtw*dtw);
}

/**
 * This function calculates the document weight for each document and saves
 * it to the database.
 */
static int
ys_docweight_calculate_and_write( docwt_t *dw, ys_docdb_t *db )
{
	ys_docnum_t docnum;

	for (docnum = 0; docnum < dw->N; docnum++) {
		float dwt = (float) sqrt(dw->weights[docnum]);
#if _DUMP_CALCWEIGHT
		printf("doc(%ld) weight(%.2f)\n",
			docnum, dwt);
#endif
		if (ys_dbadddocweight( db, docnum, dwt ) != 0)
			return -1;
	}
	free(dw->weights);
	free(dw->d_maxdtfs);
	free(dw);
	return 0;
}

static ys_bool_t
ys_select_document( void *arg, ys_docnum_t docnum, ys_doccnt_t dtf )
{
	docwt_t *dw = (docwt_t *)arg;
	if (dw->d_maxdtfs[docnum] == 0) {
		ys_doccnt_t maxdtf = 0;
		ys_dbgetdocmaxdtf(dw->docdb, docnum, &maxdtf);
		dw->d_maxdtfs[docnum] = maxdtf;
#if _DUMP_CALCWEIGHT
		printf("Document # %lu, Max DTF = %lu\n",
			docnum, maxdtf);
#endif
	}
	double idf = ys_idf(dw->N,dw->tf,dw->maxtf);
	double dtw = ys_dtw(idf, dtf, dw->d_maxdtfs[docnum]);
#if _DUMP_CALCWEIGHT
	printf("term(%.*s) doc(%lu) tf(%lu) dtf(%lu) idf(%.2f) dtw(%.2f)\n",
		dw->term[0], dw->term+1, docnum, dw->tf, dtf, idf, dtw);
#endif
	dw->weights[docnum] += (float)(dtw*dtw);
	return BOOL_TRUE;
}

static ys_bool_t 
evaluate(ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t value, 
	ys_doccnt_t doccnt, void *arg)
{
	docwt_t *dw = (docwt_t *)arg;
	dw->term = key2;
	dw->position = value;
	dw->tf = doccnt;
	dw->postings->iterate(value, ys_select_document, arg);
	return BOOL_TRUE;
}

int 
ys_build_docweights( const char *home )
{
	ys_doccnt_t N;
	ys_uchar_t key[YS_MAXKEYSIZE+1];

	strcpy((char *)key+1, "");
	key[0] = strlen((char *)key+1);

	YASENS Collection collection;
	if (collection.open( home, "r+" ) != 0)
		return -1;
	ys_btree_t *tree = collection.getIndex();
	N = collection.getN();
	docwt_t *dw = ys_docweight_allocate( N );
	if (dw == 0) {
		return -1;
	}
	dw->postings = collection.getPostFile();
	dw->docdb = collection.getDocDb();
	dw->maxdtf = collection.getMaxDtf();
	dw->maxtf = collection.getMaxTf();
	ys_btree_iterate( tree, key, evaluate, (void *)dw);
	ys_docweight_calculate_and_write( dw, dw->docdb );
	return 0;
}
