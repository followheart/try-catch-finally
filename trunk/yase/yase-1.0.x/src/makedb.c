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
* Modification history:
* DM 07-01-00 added ys_merge() and supporting routines.
* DM 08-01-00 tested ys_merge() - fixed various problems.
* DM 09-01-00 getword() created, scan_directory() moved out.
*             globals grouped into structures.
* DM 15-01-00 modified to store gaps betweeen documents in the postings
*             file. 
* DM 16-01-00 open_document() and close_document() used instead of
*             fopen() and fclose().
* DM 16-01-00 scan_directory() renamed as ys_locate_documents().
* DM 17-01-00 added rootpath, dbpath and memlimit to extract().
*             created ys_read_long() and ys_write_long().
* DM 18-01-00 added support for stemming words.
* DM 19-01-00 added support for storing word frequencies.
*             moved opendocdb(), closedocdb() and adddocument() to
*             docdb.c. see docdb.c for more details.
*             fixed bug which caused the first document in yase.docs to
*             be skipped when building the database.
*             storing doc numbers instead of doc offsets has signficantly
*             improved compression of postings.
* DM 12-05-00 eliminated most of the globals, added ystdio
*             changed ys_read_long to return success/error.
* DM 17-01-00 added various command line options.
* DM 18-01-00 added create_btree().
* DM 29-11-00 added support for wget options.
* DM 02-12-00 renamed create_btree() to ys_mkdb_create_btree().
* DM 02-12-00 renamed create_database() to ys_mkdb_create_database().
* DM 02-12-00 renamed foreachposting() to ys_mkdb_foreachposting().
* DM 14-04-01 renamed a number of variables.
* DM 19-02-02 introduced global variable Ys_debug
*
* NOTE: Twice suffered from a bug in fclose() - if you do fclose() on
* an already closed file, it screws up the memory allocation system
* totally - and you get a segmentation fault sometime later in malloc()
* or calloc() or realloc().
*/

#include "yase.h"
#include "makedb.h"
#include "avl3.h"
#include "compress.h"
#include "getword.h"
#include "locator.h"
#include "docdb.h"
#include "stem.h"
#include "ystdio.h"
#include "btree.h"
#include "wgetargs.h"
#include "version.h"

#include "getopt.h"

/**
 * When building the index, terms are read and stored in a binary tree.
 * I chose to use binary tree instead of hash table because it allows
 * me to skip the sorting. Each term is stored in a structure. The
 * documents it appeared in is stored in an array. A parallel array is
 * used to store the frequency of the term in each document. A count is
 * kept of the total number of documents the term has appeared in.
 */
typedef struct {
	char *word;			/* term */
	ys_docnum_t *doclist;		/* document array */
	ys_doccnt_t *dtflist;		/* document term frequency (dtf) array */
	ys_docnum_t lastdoc;		/* last document this term appeared
					 */
	ys_doccnt_t tf;			/* total number of documents this
					 * term appears in.
					 */
} word_t;

typedef struct {
	ys_doccnt_t maxtf;		
	ys_doccnt_t maxdtf;
	unsigned char maxword[256];	/* most popular word */
	unsigned long maxmem;		/* maximum memory used */
	unsigned long totaldocs;	/* total number of documents */
} statistics_t;

typedef struct {
	ys_file_t *prev_words_file;	
	ys_bitfile_t *prev_postings_file;
	ys_file_t *current_words_file;
	ys_bitfile_t *current_postings_file;
	char prev_word[256];
	char dbpath[1024];
	size_t memlimit;
} mergedata_t;

typedef struct {
	int prefix_len;
	int word_len;
	char word[256];
	ys_doccnt_t tf;
	ys_filepos_t posting_offset;
} rec_t;

struct ys_mkdb_t {
	char *rootpath;
	const char *dbpath;
	ys_bool_t stem;
	mergedata_t mergedata;
	statistics_t statistics;
	AVLTree *wordtree;
	ys_docdb_t *docfile;
	ys_list_t *wget_opts;
};

typedef struct docwt_t {
	float *weights;
	ys_docnum_t N;
} docwt_t;

static void ys_add_to_doclist(word_t *w, ys_docnum_t docnum);
static int ys_compare_word(void *key, void *object);
static void ys_create_word(void *object, void *key);
static int ys_comp( rec_t *r, word_t *w );
static int ys_calc_prefixlen(char *s1, char *s2);
static inline int ys_read_long( ys_bitfile_t *file, ys_docnum_t *n );
static int ys_write_long( ys_bitfile_t *file, ys_docnum_t n );
static int ys_write_rec( ys_mkdb_t *, rec_t *r, word_t *w, docwt_t *dw );
static int ys_write_word( ys_mkdb_t *, word_t *w, docwt_t *dw );
static rec_t * ys_get_record(ys_file_t *fp);
static int ys_merge(ys_mkdb_t *, int final);
static int ys_extract_words(ys_mkdb_t *arg, const char *logicalname, const char *name);
static docwt_t *ys_docweight_allocate( ys_docnum_t N );
static void ys_docweight_add_tf( docwt_t *dw, ys_docnum_t docnum, 
	ys_doccnt_t dtf, ys_doccnt_t tf );
static int ys_docweight_calculate_and_write( docwt_t *dw, ys_docdb_t *db );
static void ys_add_wget_option( ys_list_t * list, const char *option, 
	const char *optarg );
static void ys_print_usage(void);
static void ys_print_yase_help(void);
static void ys_print_yase_version(void);
static void ys_print_wget_help (void);
static int ys_index_word(ys_mkdb_t *mkdb, char *word, ys_docnum_t docnum);


static AVL_vtbl vtab = { ys_compare_word, ys_create_word, 0, 0 };
static ys_allocator_t *String_allocator;

/**
 * For each term a list of documents (the term appears in) is maintained.
 * This function adds a new document to the list, or increments the
 * document-term frequency if the document has already been added.
 */
static void 
ys_add_to_doclist(word_t *w, ys_docnum_t docnum)
{
	if (w->lastdoc == docnum) {
		/* Document already added, so increment document term frequency. */
		w->dtflist[w->tf-1]++;
		return;
	}
	/* New document - add it */
	w->doclist = ys_reallocmem(w->doclist, w->tf*sizeof docnum, 
		(w->tf+1)*sizeof docnum);
	w->dtflist = ys_reallocmem(w->dtflist, w->tf*sizeof docnum, 
		(w->tf+1)*sizeof docnum);
	w->dtflist[w->tf] = 1;		/* Set DTF to 1 */
	w->lastdoc = docnum;
	w->doclist[w->tf] = docnum;
	w->tf++;				/* Increment TF */
}

static int 
ys_compare_word(void *key, void *object)
{
	word_t *w = (word_t*) object;
	return strcmp((char *)key,w->word);
}

/**
 * Initializes a new term that has just been added to the binary tree.
 */
static 
void ys_create_word(void *object, void *key)
{
	word_t *w = (word_t*) object;
	w->word = ys_allocate(String_allocator, strlen(key)+1); 
	strcpy(w->word, (char *)key);
	w->doclist = 0;
	w->dtflist = 0;
	w->lastdoc = ~0;
	w->tf = 0;
}

static int 
ys_comp( rec_t *r, word_t *w )
{
	return strcmp( r->word, w->word );
}

/**
 * Calculates the size of the common prefix in two strings.
 */
static int 
ys_calc_prefixlen(char *s1, char *s2)
{
	size_t n = 0;
	while (*s1 && *s1++ == *s2++)
		n++;
	return n;
}

/**
 * Reads a document number from the postings file.
 */
static inline int
ys_read_long( ys_bitfile_t *file, ys_docnum_t *n_ptr )
{
	int rc;
	ys_docnum_t n;
#if _USE_COMPRESSION
#if _USE_GAMMA
	rc = ys_gamma_decode(file, &n);
	n -= 1;
#else
	rc = ys_delta_decode(file, &n);
	n -= 1;
#endif
#else
	rc = ys_bgetbits(file, &n, sizeof(ys_docnum_t) * 8);
#endif
	*n_ptr = n;
	return rc;
}

/**
 * Writes a document number to the postings file.
 */
static int
ys_write_long( ys_bitfile_t *file, ys_docnum_t n )
{
	int rc;
#if _USE_COMPRESSION
#if _USE_GAMMA
	rc = ys_gamma_encode(file, n+1);
#else
	rc = ys_delta_encode(file, n+1);
#endif
#else
	rc = ys_bputbits(file, n, 32);
#endif
	return rc;
}

/**
 * During a merge run, the data from temporary files is merged with
 * data in the binary tree. When a term appears in both the temporary files
 * and in the tree, this function is called to write out a merged version.
 * It is also called in case the term only appears in the temporary files.
 * The term, and its document list is written out.
 * NOTE: During the final merge process, this function is passed a non-null
 * dw object. This causes it to calculate the term/document weight and invoke the
 * appropriate function to accumulate the weight for each document.
 * The dw argument is null during all but the final merge.
 */
static int 
ys_write_rec( ys_mkdb_t *mkdb, rec_t *r, word_t *w, docwt_t *dw )
{
	int prefixlen, wordlen;
	ys_doccnt_t tf, prev_tf;
	ys_docnum_t docnum;
	ys_doccnt_t dtf;
	int i;
	ys_docnum_t n, d;
	ys_filepos_t pos;

	/* First write out the term */
	prefixlen = ys_calc_prefixlen(r->word, mkdb->mergedata.prev_word);
	wordlen = strlen(r->word)-prefixlen;
	ys_file_putc(prefixlen, mkdb->mergedata.current_words_file);
	ys_file_putc(wordlen, mkdb->mergedata.current_words_file);
	ys_file_write(r->word+prefixlen, 1, wordlen, mkdb->mergedata.current_words_file);

	/* Now write the document list to the postings file */
	tf = r->tf;
	if ( w )
		tf += w->tf;
	ys_bgetppos( mkdb->mergedata.current_postings_file, &pos );
	ys_file_write( &pos, 1, sizeof pos, mkdb->mergedata.current_words_file );
	ys_file_write( &tf, 1, sizeof tf, mkdb->mergedata.current_words_file );
#if _DUMP_MERGE
	printf("%s:%lu:%lu:", r->word, tf, pos);
#endif
	if (tf > mkdb->statistics.maxtf) {
		mkdb->statistics.maxtf = tf;
		memcpy(mkdb->statistics.maxword, r->word, sizeof mkdb->statistics.maxword);
	}
	ys_bsetgpos( mkdb->mergedata.prev_postings_file, &r->posting_offset );
	ys_read_long(mkdb->mergedata.prev_postings_file, &prev_tf);
	assert(prev_tf == r->tf);
	ys_write_long(mkdb->mergedata.current_postings_file, tf);

	for (i = 0, n = 0; i < r->tf; i++) {
		ys_read_long(mkdb->mergedata.prev_postings_file, &docnum);
		ys_read_long(mkdb->mergedata.prev_postings_file, &dtf);
#if _DUMP_MERGE
		printf("%lu:", docnum);
#endif
		d = docnum;
		n += docnum;
		ys_write_long(mkdb->mergedata.current_postings_file, d);
		ys_write_long(mkdb->mergedata.current_postings_file, dtf);
		if (dw != 0) {
			ys_docweight_add_tf( dw, n, dtf, tf );
		}
		if (dtf > mkdb->statistics.maxdtf) {
			mkdb->statistics.maxdtf = dtf;
		}
	}	
	if ( w ) {
		for (i = 0; i < w->tf; i++) {
#if _DUMP_MERGE
			printf("%lu:", w->doclist[i]);
#endif
			d = w->doclist[i]-n;
			n = w->doclist[i];
			ys_write_long(mkdb->mergedata.current_postings_file, d);
			ys_write_long(mkdb->mergedata.current_postings_file, 
				w->dtflist[i]);
			if (dw != 0) {
				ys_docweight_add_tf( dw, n, w->dtflist[i], 
					tf );
			}
			if (w->dtflist[i] > mkdb->statistics.maxdtf) {
				mkdb->statistics.maxdtf = w->dtflist[i];
			}
		}
	}
	ys_bflush(mkdb->mergedata.current_postings_file);
	memcpy(mkdb->mergedata.prev_word, r->word, sizeof mkdb->mergedata.prev_word);
#if _DUMP_MERGE
	printf("\n");
#endif
	return 0;
}

/**
 * This function writes out a term and its asociated document list to the
 * files used during merging.
 * NOTE: During the final merge process, this function is passed a non-null
 * dw object. This causes it to calculate the term/document weight and invoke the
 * appropriate function to accumulate the weight for each document.
 * The dw argument is null during all but the final merge.
 */
static int 
ys_write_word( ys_mkdb_t *mkdb, word_t *w, docwt_t *dw )
{
	int prefixlen, wordlen;
	ys_doccnt_t tf;
	int i;
	ys_docnum_t n, d;
	ys_filepos_t pos;

	prefixlen = ys_calc_prefixlen(w->word, mkdb->mergedata.prev_word);
	wordlen = strlen(w->word)-prefixlen;
	ys_file_putc(prefixlen, mkdb->mergedata.current_words_file);
	ys_file_putc(wordlen, mkdb->mergedata.current_words_file);
	ys_file_write(w->word+prefixlen, 1, wordlen, mkdb->mergedata.current_words_file);
	tf = w->tf;
	ys_bgetppos( mkdb->mergedata.current_postings_file, &pos );
	ys_file_write( &pos, 1, sizeof pos, mkdb->mergedata.current_words_file );
	ys_file_write( &tf, 1, sizeof tf, mkdb->mergedata.current_words_file );
#if _DUMP_MERGE
	printf("%s:%lu:%lu:", w->word, tf, pos);
#endif
	if (tf > mkdb->statistics.maxtf) {
		mkdb->statistics.maxtf = tf;
		memcpy(mkdb->statistics.maxword, w->word, sizeof mkdb->statistics.maxword);
	}
	ys_write_long(mkdb->mergedata.current_postings_file, tf);
	for (n = 0, i = 0; i < w->tf; i++) {
#if _DUMP_MERGE
		printf("%lu:", w->doclist[i]);
#endif
		d = w->doclist[i]-n;
		n = w->doclist[i];
		ys_write_long(mkdb->mergedata.current_postings_file, d);
		ys_write_long(mkdb->mergedata.current_postings_file, 
			w->dtflist[i]);
		if (dw != 0) {
			ys_docweight_add_tf( dw, n, w->dtflist[i], tf );
		}
		if (w->dtflist[i] > mkdb->statistics.maxdtf) {
			mkdb->statistics.maxdtf = w->dtflist[i];
		}
	}
	ys_bflush(mkdb->mergedata.current_postings_file);
	memcpy(mkdb->mergedata.prev_word, w->word, sizeof mkdb->mergedata.prev_word);
#if _DUMP_MERGE
	printf("\n");
#endif
	return 0;
}
	 
/**
 * During merging temporary files are used to store terms and
 * postings. This function reads a term from the terms file.
 */
static rec_t *
ys_get_record(ys_file_t *fp)
{
	static rec_t record;
	if (fp == NULL || ys_file_eof(fp))
		return NULL;
	record.prefix_len = ys_file_getc(fp);
	if (ys_file_eof(fp))
		return NULL;
	record.word_len = ys_file_getc(fp);
	ys_file_read(record.word+record.prefix_len, 1, record.word_len, fp);
	record.word[record.prefix_len+record.word_len] = 0;
	ys_file_read(&record.posting_offset, 1, sizeof record.posting_offset, fp);
	ys_file_read(&record.tf, 1, sizeof record.tf, fp);
	return &record;
}

/*
* Merge algorithm.
* 7 Jan 2000
* a and b are sorted lists
* merge(a, b)
* {
*	get a;
*	get b;
*	while ( a || b ) {
*		while ( a && (!b || a <= b) ) {
*			output a;
*			if ( b && a == b ) {
*				output b;
*				get b;
*			}
*			get a;
*		}
*		if (b) {
*			output b;
*			get b;
*		}
*	}
* }
*/

/**
 * This function implements the high level merging algorithm
 * shown above. The contents of the binary tree is merged with the
 * contents of temporary files (from previous merge run) and 
 * new set of temporary files are created. In the final merge run,
 * the temporary files are renamed, and become the final yase files.
 */
static int 
ys_merge(ys_mkdb_t *mkdb, int final)
{
	word_t *w;
	rec_t *r;
	static int tmp = 1;
	char name[1024];
	docwt_t *dw = 0;

	extern void ys_xml_release_memory(void); /* TODO: */

#if _NEWSTYLE_DOCWT
	if (final) {
		ys_docnum_t N;
		N = ys_dbnumdocs( mkdb->docfile );
		dw = ys_docweight_allocate( N );
		if (dw == 0)
			return -1;
	}
#endif

	if (Maxmem > mkdb->statistics.maxmem)
		mkdb->statistics.maxmem = Maxmem;

	printf("Merge run# %d\n", tmp);

	snprintf(name, sizeof name, "%s/tmp.%d.words", mkdb->mergedata.dbpath, tmp);
	mkdb->mergedata.current_words_file = ys_file_open(name, "w+", YS_FILE_ABORT_ON_ERROR);
	snprintf(name, sizeof name, "%s/tmp.%d.postings", 
		mkdb->mergedata.dbpath, tmp);
	mkdb->mergedata.current_postings_file = ys_bopen(name, "wb+");
	
	assert(mkdb->mergedata.current_words_file != 0);
	assert(mkdb->mergedata.current_postings_file != 0);

	if (mkdb->mergedata.prev_words_file != 0) {
		ys_file_rewind(mkdb->mergedata.prev_words_file);
	}

	mkdb->mergedata.prev_word[0] = 0;
	r = ys_get_record(mkdb->mergedata.prev_words_file);	
	w = AVLTree_FindFirst(mkdb->wordtree); 

	while ( r || w ) {
		while ( r && ( !w || ys_comp(r, w) <= 0 ) ) {
			if ( w && ys_comp(r, w) == 0 ) {
				ys_write_rec(mkdb, r, w, dw);
				ys_freemem(w->doclist, 
					w->tf * sizeof w->doclist[0]);
				ys_freemem(w->dtflist, 
					w->tf * sizeof w->dtflist[0]);
				w = AVLTree_FindNext(mkdb->wordtree,w);
			}
			else {
				ys_write_rec(mkdb, r, 0, dw);
			}
			r = ys_get_record(mkdb->mergedata.prev_words_file);
		}
		if ( w ) {
			ys_write_word(mkdb, w, dw);
			ys_freemem(w->doclist, 
				w->tf * sizeof w->doclist[0]);
			ys_freemem(w->dtflist, 
				w->tf * sizeof w->dtflist[0]);
			w = AVLTree_FindNext(mkdb->wordtree,w);
		}
	}

	if (mkdb->mergedata.prev_postings_file != 0) {
		ys_bclose(mkdb->mergedata.prev_postings_file);
		ys_file_close(mkdb->mergedata.prev_words_file);
		snprintf(name, sizeof name, "%s/tmp.%d.words", mkdb->mergedata.dbpath, tmp-1);
		remove(name);
		snprintf(name, sizeof name, "%s/tmp.%d.postings", mkdb->mergedata.dbpath, tmp-1);
		remove(name);
	}

	ys_destroy_allocator(String_allocator);
	AVLTree_Destroy(mkdb->wordtree);
	ys_destroyallmem();
	ys_xml_release_memory(); /* TODO: */
	assert(Maxmem == 0);
	String_allocator = ys_new_allocator(0, 10);
	mkdb->wordtree = AVLTree_New(&vtab, sizeof(word_t), 100);

	if (final) {
		char newname[sizeof name];
		ys_bclose(mkdb->mergedata.current_postings_file);
		ys_file_close(mkdb->mergedata.current_words_file);
		snprintf(name, sizeof name, "%s/tmp.%d.words", 
			mkdb->mergedata.dbpath, tmp);
		snprintf(newname, sizeof newname, "%s/yase.words", 
			mkdb->mergedata.dbpath);
		rename(name, newname);
		snprintf(name, sizeof name, "%s/tmp.%d.postings", 
			mkdb->mergedata.dbpath, tmp);
		snprintf(newname, sizeof newname, "%s/yase.postings",
			mkdb->mergedata.dbpath);
		rename(name, newname);
#if _NEWSTYLE_DOCWT
		printf("Writing document weights\n");	
		ys_docweight_calculate_and_write( dw, mkdb->docfile );
#endif
	} 
	else {
		mkdb->mergedata.prev_postings_file = mkdb->mergedata.current_postings_file;
		mkdb->mergedata.prev_words_file = mkdb->mergedata.current_words_file;
	}
	tmp += 1;

	return 0;
}

/**
 * This function adds a term to the binary tree. The associated document
 * number is recorded in the list attached to the term.
 */
static int
ys_index_word(ys_mkdb_t *mkdb, char *word, ys_docnum_t docnum) 
{
	word_t *w;
		
	if (mkdb->stem) {
		word[0] = strlen(word+1);
		stem(word);
	}
	w = AVLTree_Insert(mkdb->wordtree, word+1);
	assert(w != 0);
	ys_add_to_doclist(w, docnum);			
	return 0;
}

/**
 * This function processes a document file. It runs the document
 * processor routines which in turn index all terms occuring in the
 * file.
 */
static int 
ys_extract_words(ys_mkdb_t *mkdb, const char *logicalname, 
	const char *physicalname)
{
	int rc;

	rc = ys_document_process(logicalname, physicalname, 
		mkdb->docfile, ys_index_word, mkdb);
	if (Maxmem > mkdb->mergedata.memlimit*1024*1024) {
		
		ys_merge(mkdb, 0);
	}
	mkdb->statistics.totaldocs++;
	return 0;
}

/**
 * Calculate a documents weight. In order to calculate the document weight
 * it is necessary to know the frequency of every term within the document.
 * We already have that information in the tree, and we use the fact that
 * all terms which were visited while processing this document will have
 * lastdoc set to this document.
 * NOTE: This function is not used anymore unless you set _NEWSTYLE_DOCWT to 0.
 * The problem with this function was that document weight could not be 
 * calculated in the context of the whole collection - as it was calculated 
 * when each document was saved. The newer algorithm waits until all documents 
 * have been processed. Since more information is available at that point, 
 * the document weight can be calculated more accurately.
 */
float
ys_mkdb_calculate_document_weight(ys_mkdb_t *mkdb, ys_docnum_t docnum)
{
	double wt = 0.0;
	word_t *w;

	w = AVLTree_FindFirst(mkdb->wordtree);
	while (w != 0) {
		if (w->lastdoc == docnum) {
			ys_doccnt_t dtf = w->dtflist[w->tf-1];
			double log_dtf = CALC_LOGDTF(dtf);
#if _DUMP_CALCWEIGHT
			printf("Document %ld '%s' dtf=%ld, log_dtf=%f\n",
				docnum+1, w->word, dtf, log_dtf);
#endif
			wt += (log_dtf*log_dtf);
		}
		w = AVLTree_FindNext(mkdb->wordtree,w);
	}
	wt = sqrt(wt);
#if _DUMP_CALCWEIGHT
	printf("Document %ld weight = %.2f\n", docnum+1, wt);
#endif
	return (float)wt;
}

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
	ys_doccnt_t dtf, ys_doccnt_t tf )
{
	double log_dtf = CALC_LOGDTF(dtf);
	double idf = CALC_IDF(dw->N,tf);
	double dtw = log_dtf * idf;
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
		printf("Document %ld weight=%.2f\n",
			docnum+1, dwt);
#endif
		if (ys_dbadddocweight( db, docnum, dwt ) != 0)
			return -1;
	}
	free(dw->weights);
	free(dw);
	return 0;
}

ys_list_t *
ys_mkdb_get_wgetargs( ys_mkdb_t *mkdb ) 
{
	return mkdb->wget_opts;
}

/**
 * This is the main driving function that scans the user supplied list of 
 * directories or urls and creates the YASE database files. Only the index 
 * file is created separately.
 */
int 
ys_mkdb_create_database(char *pathname[], ys_mkdb_userargs_t *args)
{
	int rc = -1;
	ys_docdb_t *docfile;
	ys_mkdb_t mkdb = {0};

	if (args->dbpath == 0) 
		args->dbpath = ".";
	if (args->memlimit == 0)
		args->memlimit = 5;
	strncpy(mkdb.mergedata.dbpath, args->dbpath, sizeof mkdb.mergedata.dbpath);
	mkdb.mergedata.memlimit = args->memlimit;

	docfile = ys_dbopen(args->dbpath, "w", args->rootpath);
	if ( docfile == 0 ) {
		return -1;
	}

	String_allocator = ys_new_allocator(0, 10);
	mkdb.wordtree = AVLTree_New(&vtab, sizeof(word_t), 100);
	mkdb.rootpath = args->rootpath;
	mkdb.dbpath = args->dbpath;
	mkdb.stem = args->stem;
	mkdb.wget_opts = args->wget_opts;
	mkdb.docfile = docfile;

	while ( *pathname && (rc = ys_locate_documents(*pathname, ys_extract_words, 
		&mkdb)) == 0)
		pathname++;

	if (rc == 0) {
		if (ys_merge(&mkdb, 1) == 0) {
			printf("maximum memory used = %lu\n", 
				mkdb.statistics.maxmem);
			printf("total files processed = %lu\n", 
				mkdb.statistics.totaldocs);
			printf("total documents processed = %lu\n", 
				ys_dbnumdocs(docfile));
			printf("most popular term = %s\n", 
				mkdb.statistics.maxword);
			printf("maximum collection term frequency = %lu\n", 
				mkdb.statistics.maxtf);
			printf("maximum document term frequency = %lu\n", 
				mkdb.statistics.maxdtf);
			rc = ys_dbaddinfo(docfile, ys_dbnumdocs(docfile),
				mkdb.statistics.totaldocs,
				mkdb.statistics.maxdtf,
				mkdb.statistics.maxtf,
				mkdb.stem);
		}
		else
			rc = -1;
	}

	ys_destroy_allocator(String_allocator);
	AVLTree_Destroy(mkdb.wordtree);
	ys_destroyallmem();
	assert(Maxmem == 0);
	ys_dbclose(docfile);

	return rc;
}

/**
 * Retrieves a term's frequency from the postings file.
 * offset must point to the start of the terms postings.
 */
int 
ys_mkdb_get_term_frequency( ys_bitfile_t *postings_file, ys_filepos_t offset,
	ys_doccnt_t *tf )
{
	ys_bsetgpos( postings_file, &offset );
	ys_read_long( postings_file, tf );
	return 0;
}

/**
 * This function creates the YASE index file.
 */
int
ys_mkdb_create_btree(ys_mkdb_userargs_t *args)
{
	char wordfile[1024];
	char treefile[1024];

	if (args->dbpath == 0) args->dbpath = ".";
	snprintf(wordfile, sizeof wordfile, "%s/%s", args->dbpath, "yase.words");
	snprintf(treefile, sizeof treefile, "%s/%s", args->dbpath, "yase.btree");
	if (ys_btree_create( treefile, wordfile ) == 0)
		return 0;
	return -1;
}

/**
 * Evaluate a user supplied function for each posting starting
 * from the given offset. The offset must point to the start of
 * postings for a term.
 */
void 
ys_mkdb_foreachposting( ys_bitfile_t *postings_file, ys_filepos_t offset,
	ys_pfn_postings_iterator_t *pfunc, void *arg )
{
	ys_doccnt_t tf;
	ys_docnum_t docnum, n;	
	ys_doccnt_t dtf;
	int i;

	ys_bsetgpos( postings_file, &offset );
	ys_read_long( postings_file, &tf );
#if _DUMP_POSTINGS
	printf("(%lu ", tf);
#endif
	for ( n = 0, i = 0; i < tf; i++ ) {
		ys_read_long( postings_file, &docnum );
		ys_read_long( postings_file, &dtf );
#if _DUMP_POSTINGS
		printf("<%lu,%lu>", docnum, dtf); 
#endif
		n += docnum;
		if (! pfunc( arg, n, dtf ) )
			break;
	}	
#if _DUMP_POSTINGS
	printf(")\n"); 
#endif
}


/**
 * wget options requested by the user are stored in a list. This function 
 * adds an option to the list.
 */
static void 
ys_add_wget_option( ys_list_t * list, const char *option, const char *optarg )
{
	ys_wget_option_t *yo = calloc(1, sizeof *yo);
	if (yo == 0) {
		fprintf(stderr, "Error: ran out of memory\n");
		exit(EXIT_FAILURE);
	}	
	if (optarg != 0) {
		snprintf(yo->option, sizeof yo->option,
			"--%s=%s", option, optarg);
	}
	else {
		snprintf(yo->option, sizeof yo->option,
			"--%s", option);
	}
	ys_list_append(list, yo);
}

static void
ys_delete_wget_options( ys_list_t *list )
{
	/* TODO */
}

static void
ys_print_usage(void)
{
  printf("Usage: yasemakedb [OPTIONS]... [DIRECTORY/URL]...\n"); 
}

static void
ys_print_yase_version(void) 
{
  printf("YASE %s database creation utility\n", Yase_version);
  printf ("Written by Dibyendu Majumdar <dibyendu@mazumdar.demon.co.uk>.\n");
  printf("%s", "\
Copyright (C) 2000-2002 Dibyendu Majumdar\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License, Version 2, for more details.\n\n");
}

static void 
ys_print_yase_help(void) 
{
  ys_print_yase_version();
  ys_print_usage();
#ifdef USE_WGET
  printf("YASE makedb options =>\
  \n\
  -h, --help                   display this help message and exit.\n\
  -V, --version                display version of YASE and exit.\n\
  -H, --yase-home=PATH         YASE home directory.\n\
  -s, --enable-stemming        extract the stem of a word before indexing.\n\
  -m, --max-memory=N           use upto N megabytes of memory (approx).\n\
  -r, --root-directory=DIR     store document paths relative to DIR.\n\
  -w, --show-wget-options-available      display supported wget options and exit.\n\
  -W, --show-wget-options      display wget options being used and exit.\n\n\
Mail bug reports and suggestions to <dibyendu@mazumdar.demon.co.uk>.\n");
#else
  printf("YASE makedb options =>\
  \n\
  -h, --help                   display this help message and exit.\n\
  -V, --version                display version of YASE and exit.\n\
  -H, --yase-home=PATH         YASE home directory.\n\
  -s, --enable-stemming        extract the stem of a word before indexing.\n\
  -m, --max-memory=N           use upto N megabytes of memory (approx).\n\
  -r, --root-directory=DIR     store document paths relative to DIR.\n\n\
Mail bug reports and suggestions to <dibyendu@mazumdar.demon.co.uk>.\n");
#endif
}

static void
ys_print_wget_help (void)
{
  printf ("%s%s%s%s%s%s%s%s%s", "\
\n\
wget options =>\n\
\n", "\
Logging and input file:\n\
  --wget-output-file=FILE     log messages to FILE.\n\
  --wget-append-output=FILE   append messages to FILE.\n\
  --wget-verbose              be verbose.\n\
  --wget-non-verbose          turn off verboseness, without being quiet.\n\
  --wget-force-html           treat input file as HTML.\n\
\n", "\
Download:\n\
  --wget-tries=NUMBER           set number of retries to NUMBER (0 unlimits).\n\
  --wget-no-clobber             don\'t clobber existing files.\n\
  --wget-continue               restart getting an existing file.\n\
  --wget-dot-style=STYLE        set retrieval display style.\n\
  --wget-timestamping           don\'t retrieve files if older than local.\n\
  --wget-server-response        print server response.\n\
  --wget-timeout=SECONDS        set the read timeout to SECONDS.\n\
  --wget-wait=SECONDS           wait SECONDS between retrievals.\n\
  --wget-proxy=on/off           turn proxy on or off.\n\
  --wget-quota=NUMBER           set retrieval quota to NUMBER.\n\
\n",  "\
Directories:\n\
  --wget-directory-prefix=PREFIX   save files to PREFIX/...\n\
  --wget-cut-dirs=NUMBER           ignore NUMBER remote directory components.\n\
\n", "\
HTTP options:\n\
  --wget-http-user=USER      set http user to USER.\n\
  --wget-http-passwd=PASS    set http password to PASS.\n\
  --wget-cache=on/off        (dis)allow server-cached data (normally allowed).\n\
  --wget-ignore-length       ignore `Content-Length\' header field.\n\
  --wget-header=STRING       insert STRING among the headers.\n\
  --wget-proxy-user=USER     set USER as proxy username.\n\
  --wget-proxy-passwd=PASS   set PASS as proxy password.\n\
  --wget-save-headers        save the HTTP headers to file.\n\
  --wget-user-agent=AGENT    identify as AGENT instead of Wget/VERSION.\n\
\n", "\
FTP options:\n\
  --wget-retr-symlinks   retrieve FTP symbolic links.\n\
  --wget-glob=on/off     turn file name globbing on or off.\n\
  --wget-passive-ftp     use the \"passive\" transfer mode.\n\
\n", "\
Recursive retrieval:\n\
  --wget-recursive             recursive web-suck -- use with care!.\n\
  --wget-level=NUMBER          maximum recursion depth (0 to unlimit).\n\
  --wget-dont-delete-after     don\'t delete downloaded files.\n\
  --wget-convert-links         convert non-relative links to relative.\n\
  --wget-dont-remove-listing   don\'t remove `.listing\' files.\n\
\n", "\
Recursive accept/reject:\n\
  --wget-accept=LIST                list of accepted extensions.\n\
  --wget-reject=LIST                list of rejected extensions.\n\
  --wget-domains=LIST               list of accepted domains.\n\
  --wget-exclude-domains=LIST       comma-separated list of rejected domains.\n\
  --wget-relative                   follow relative links only.\n\
  --wget-follow-ftp                 follow FTP links from HTML documents.\n\
  --wget-span-hosts                 go to foreign hosts when recursive.\n\
  --wget-include-directories=LIST   list of allowed directories.\n\
  --wget-exclude-directories=LIST   list of excluded directories.\n\
  --wget-no-host-lookup             don\'t DNS-lookup hosts.\n\
  --wget-no-parent                  don\'t ascend to the parent directory.\n\
\n", "Mail bug reports and suggestions to <dibyendu@mazumdar.demon.co.uk>.\n");
}

#ifdef YASEMAKEDB

int Ys_debug = 0;

int 
main(int argc, char *argv[])
{
	int c;
	char yasehome[1024];
	static ys_list_t wget_opts = {0};
	ys_wget_option_t *opt_ptr;
	ys_bool_t wgetquiet = BOOL_TRUE, wgetdeleteafter = BOOL_TRUE;
	ys_bool_t wgetdirprefix = BOOL_TRUE, wgetaccept = BOOL_TRUE;
	ys_bool_t wgetoptions = BOOL_FALSE;
	ys_mkdb_userargs_t args = {0};

	enum {
	wget_dummy = 0,
#define x(a,b,c) a,
#define y(a,b,c) a,
#include "wgetargs.h"
	};

	static struct option long_options[] =
	{
#define x(a,b,c) { #b, no_argument, NULL, a },
#define y(a,b,c) { #b, required_argument, NULL, a },
#include "wgetargs.h"

		{ "enable-stemming", no_argument, NULL, 's' },
		{ "show-wget-options-available", no_argument, NULL, 'w' },
		{ "show-wget-options", no_argument, NULL, 'W' },
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V' },

		{ "root-directory", required_argument, NULL, 'r' },
		{ "yase-home", required_argument, NULL, 'H' },
		{ "max-memory", required_argument, NULL, 'm' },

		{ 0, 0, 0, 0 }
	};

	opterr = 0;
	ys_list_init(&wget_opts);
	while ((c = getopt_long (argc, argv, "r:H:m:swWhV",
			   long_options, (int *)0)) != EOF) {
		switch (c) {
		case 'r': args.rootpath = optarg; break;
		case 'H': args.dbpath = optarg; break;
		case 'h': ys_print_yase_help(); return EXIT_SUCCESS;
		case 'm': args.memlimit = atoi(optarg); break;
		case 's': args.stem = BOOL_TRUE; break;
		case 'V': ys_print_yase_version(); return EXIT_SUCCESS;

#ifdef USE_WGET
		case 'w': ys_print_wget_help(); return EXIT_SUCCESS;
		case 'W': wgetoptions = BOOL_TRUE; break;
#define x(a,b,c) case a: ys_add_wget_option(&wget_opts, #c, 0); break;
#define y(a,b,c) case a: ys_add_wget_option(&wget_opts, #c, optarg); break;
#include "wgetargs.h"
#endif

		case '?':
		default:
			fprintf(stderr, "Unrecognised option %c; try --help\n", optopt);
			return EXIT_FAILURE;
		}
	}
	
	if (optind == argc) {
		ys_print_yase_help();
		return EXIT_FAILURE;
	}
	assert(optind >= 1);

	for (opt_ptr = (ys_wget_option_t *)ys_list_first(&wget_opts);
		  opt_ptr != 0;
		  opt_ptr = (ys_wget_option_t *)ys_list_next(&wget_opts, opt_ptr)) {

		if (strcmp(opt_ptr->option, "--dont-delete-after") == 0)  {
			ys_wget_option_t *saved_opt_ptr = opt_ptr;
			wgetdeleteafter = BOOL_FALSE;
		  	opt_ptr = (ys_wget_option_t *)ys_list_next(&wget_opts, opt_ptr);
			ys_list_remove(&wget_opts, saved_opt_ptr);
			free(saved_opt_ptr);
			if (opt_ptr == 0) break;
		}
		else if (strcmp(opt_ptr->option, "--verbose") == 0)
			wgetquiet = BOOL_FALSE;
		else if (strcmp(opt_ptr->option, "--non-verbose") == 0)
			wgetquiet = BOOL_FALSE;
		else if (strncmp(opt_ptr->option, "--directory-prefix=", 19) == 0)
			wgetdirprefix = BOOL_FALSE;
		else if (strncmp(opt_ptr->option, "--accept=", 9) == 0)
			wgetaccept = BOOL_FALSE; 
		
	}

	if (wgetaccept)
		ys_add_wget_option(&wget_opts, "accept", "html,htm");
	if (wgetdirprefix)
		ys_add_wget_option(&wget_opts, "directory-prefix", "/tmp");
	if (wgetquiet)
		ys_add_wget_option(&wget_opts, "quiet", 0);
	if (wgetdeleteafter)
		ys_add_wget_option(&wget_opts, "delete-after", 0);
	ys_add_wget_option(&wget_opts, "recursive", 0);

	if (wgetoptions) {
		printf("Using following wget options =>\n");
		for (opt_ptr = (ys_wget_option_t *)ys_list_first(&wget_opts);
			  opt_ptr != 0;
			  opt_ptr = (ys_wget_option_t *)ys_list_next(&wget_opts, opt_ptr)) {

			printf("%s\n", opt_ptr->option);
		}
		return EXIT_SUCCESS;
	}

	if (args.dbpath == 0) {
		args.dbpath = ".";
	}
	args.wget_opts = &wget_opts;
	snprintf(yasehome, sizeof yasehome, "YASE_DBPATH=%s", args.dbpath);
	putenv(yasehome);	
	if (ys_mkdb_create_database(argv+optind, &args) == 0)
		if (ys_mkdb_create_btree(&args) == 0)
			return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

#endif
