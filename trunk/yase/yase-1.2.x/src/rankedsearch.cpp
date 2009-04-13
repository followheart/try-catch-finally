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
// 18-12-02: Created
// 02-01-03: Revised to use re-written tokenizers
// 09-01-03: The result tree must be deleted in RankedSearchResultSet
// 08-02-03: Ignore and, or and not as terms so that we can combine boolean
//           queries with ranking.

#include "rankedsearch.h"
#include "formulas.h"
#include "stem.h"

#include <new>

static int ys_compare_document(void *key, void *object);
static void ys_create_document(void *object, void *key);

static AVL_vtbl vtab = { ys_compare_document, ys_create_document, 0, 0 };

static int ys_compare_rank(void *key, void *object);
static void ys_create_rank(void *object, void *key);

static AVL_vtbl vtab_rank = { ys_compare_rank, ys_create_rank, 0, 0 };

YASE_NS_BEGIN

class QueryDocument : public SearchResultItem {
	friend class RankedDocument;
	friend class RankedSearch;
	friend class RankedSearchResultSet;
protected:
	int termcount;
	float dwt;		/* The document weight */
	ys_doccnt_t maxdtf;	/* max dtf within document */
	bool weighted;		/* Has the document weight been retrieved ? */
public:
	QueryDocument(ys_docnum_t docnum) {
		this->docnum = docnum;
		termcount = 0;
		dwt = 0.0;
		maxdtf = 0;
		weighted = false;
	}
	int compare(ys_docnum_t docnum) {
		if (this->docnum == docnum)
			return 0;
		else if (docnum > this->docnum)
			return 1;
		return -1;
	}
};

class RankedDocument {
	friend class RankedSearchResultSet;
	friend class RankedSearch;
	QueryDocument *d;
public:
	RankedDocument() { d = 0; }
	RankedDocument(const RankedDocument *other) {
		d = other->d;
	}
	int compare(const RankedDocument *k) {
	 	if (k->d->rank == d->rank) {
			if (k->d->docnum == d->docnum)
				return 0;
			else if (k->d->docnum > d->docnum)
				return 1;
			else
				return -1;
		}
		else if (k->d->rank < d->rank)
			return 1;
		return -1;
	}
};

class RankedSearchResultSet : public SearchResultSet {
	RankedDocument *current;
	AVLTree *rtree;                   /* Sorted result set */
	AVLTree *avltree;                 /* Unsorted result set */
public:
	RankedSearchResultSet() {
		rtree = AVLTree_New( &vtab_rank, sizeof(RankedDocument), 200 );
		avltree = AVLTree_New( &vtab, sizeof(QueryDocument), 200 );
		current = 0;
	}
	~RankedSearchResultSet() {
		if (rtree != 0)
			AVLTree_Destroy(rtree);
		if (avltree != 0)
			AVLTree_Destroy(avltree);
	}
	QueryDocument* add(ys_docnum_t docnum) {
		return (QueryDocument *) AVLTree_Insert(avltree, (void *)&docnum);
	}
	RankedDocument* add(RankedDocument *k) {
		count++;
		return (RankedDocument *) AVLTree_Insert(rtree, (void *)k);
	}
	SearchResultItem *getNext() {
		if (current == 0) {
			current = (RankedDocument *)AVLTree_FindFirst(rtree);
		}
		else {
			current = (RankedDocument *)AVLTree_FindNext(rtree, (void *)current);
		}
		return current != 0 ? current->d : 0;
	}
	bool sortByRank();
	void setElapsedTime(double e) { elapsed = e; }
	bool contains(ys_docnum_t docnum) {
		return AVLTree_Search(avltree, (void *) &docnum) != NULL;
	}	
};

YASE_NS_END

/**
 * Utility functions for the trees
 */
static int
ys_compare_document(void *key, void *object)
{
	ys_docnum_t *docnum = (ys_docnum_t*) key;
	YASENS QueryDocument *document = (YASENS QueryDocument *)object;
	return document->compare(*docnum);
}

static void
ys_create_document(void *object, void *key)
{
	ys_docnum_t *docnum = (ys_docnum_t*) key;
	YASENS QueryDocument *document = new (object) YASENS QueryDocument(*docnum);
}

static int
ys_compare_rank(void *key, void *object)
{
	YASENS RankedDocument *v = (YASENS RankedDocument*) object;
	YASENS RankedDocument *k = (YASENS RankedDocument*) key;
	return v->compare(k);
}

static void
ys_create_rank(void *object, void *key)
{
	YASENS RankedDocument *k = (YASENS RankedDocument*) key;
	YASENS RankedDocument *v = new (object) YASENS RankedDocument(k);
}

/**
 * Output results of the query. If all_terms was specified, ignore matches
 * that didnot contain all the terms. If ranked query was requested,
 * sort results by rank before sending them to the user.
 */
bool
YASENS RankedSearchResultSet::sortByRank()
{
	QueryDocument *document;

	/* Process all document matches */
	for (document = (QueryDocument *) AVLTree_FindFirst(avltree);
		document != NULL;
		document = (QueryDocument *) AVLTree_FindNext(avltree, document)) {

		RankedDocument key;

		/* Normalise rank */
		document->rank = document->rank / document->dwt;
		key.d = document;

		/* Sort the document by rank */
		if (!add(&key)) {
			// snprintf(message, sizeof message,
			//	"Error: cannot insert data into a tree\n");
			return false;
		}
	}
	return true;
}

YASENS RankedSearch::RankedSearch(YASENS Collection *collection) :
	YASENS Search(collection)
{
	termcount = 0;
	matches = 0;
	qmf = 0;
	curterm = 0;
	resultSet = 0;
}

YASENS RankedSearch::~RankedSearch()
{
}

/**
 * Called for each document found for a query term. If processing a
 * ranked query - document weights are retrieved and stored. Data about
 * the document is stored in the result tree.
 */
bool
YASENS RankedSearch::selectDocument(ys_docnum_t docnum, ys_doccnt_t dtf)
{
	QueryDocument *document;

	/* Add/update data about this document */
	document = resultSet->add(docnum);
	if (document != 0) {

		if (document->termcount != curterm) {
			document->termcount = curterm;
			document->hits++;
		}

		float idf, qtw;

		if (!document->weighted) {
			/* Retrieve document weight */
			if (ys_dbgetdocwtdtf(collection->getDocDb(),
				document->docnum, &document->dwt, &document->maxdtf) != 0 ) {
				// snprintf(message, sizeof message, "Error: cannot retrieve document weight\n");
				return false;
			}
#if _DUMP_RANKING
			printf("Document # %ld, wt=%.2f, maxdtf=%lu\n",
				document->docnum, document->dwt, document->maxdtf);
#endif
			document->weighted = true;
			matches++;
		}

		/* Calculate rank */
		idf = terms[curterm-1].idf;
		qtw = terms[curterm-1].qtw;
		document->rank = document->rank +
			(float)ys_inner_product(ys_dtw(idf, dtf, document->maxdtf), qtw);
#if _DUMP_RANKING
		printf("Term # %d, Document # %ld, idf=%.2f, dtw=%.2f, qtw=%.2f, ip=%.2f\n",
			curterm, document->docnum, idf, ys_dtw(idf, dtf, 0),
			qtw, ys_inner_product(ys_dtw(idf, dtf, 0), qtw));
#endif
	}
	else  {
		// snprintf(message, sizeof message, "Error: cannot insert into result tree\n");
		return false;
	}
	return true;
}


/**
 * Called for each document found for a query term. If processing a
 * ranked query - document weights are retrieved and stored. Data about
 * the document is stored in the result tree.
 */
static ys_bool_t
ys_select_document( void *arg1, ys_docnum_t docnum, ys_doccnt_t dtf )
{
	YASENS RankedSearch *search = (YASENS RankedSearch *)arg1;
	return search->selectDocument(docnum, dtf);
}

/**
 * Calculate query term weight.
 */
void
YASENS RankedSearch::calculateWeight(ys_doccnt_t tf, ys_docnum_t N)
{
	int i = curterm-1;

	terms[i].idf = (float) ys_idf(N,tf,N);
	terms[i].qtw = (float) ys_qtw(terms[i].idf,
			terms[i].qtf, qmf);
#if _DUMP_RANKING
	printf("Term %s tf=%ld idf=%.2f qtf = %d, qtw = %.2f\n",
		terms[i].text, tf, terms[i].idf,
		terms[i].qtf, terms[i].qtw );
#endif
}

/**
 * Called for each term found in the index. Retrieves all documents
 * for the term and saves them in the result tree.
 * Note that this function may be called to process exact as well as partial
 * matches. It is called for all terms after the first term that is found
 * - until it returns BOOL_FALSE.
 */
bool
YASENS RankedSearch::findDocs(ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t position,
					   ys_doccnt_t tf)
{
	/* If the keys are not of equal length, then we return false */
	if (key1[0] != key2[0])
		return false;

	if (strncmp( (const char *)key1+1, (const char *)key2+1, key1[0] ) == 0) {
#if _TEST_FREQ_IN_TREE
		ys_doccnt_t fq = 0;
		fq = collection->postings_file->get_term_frequency( position );
		assert(tf == fq);
#endif
		calculateWeight(tf, collection->getN());
		//snprintf(query->message+strlen(query->message),
		//	sizeof query->message-strlen(query->message),
		//	 "%.*s (%ld) ",
		//	key1[0], key1+1, tf);
		collection->getPostFile()->iterate( position, ys_select_document, this );
		return true;
	}

	return false;
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
	YASENS RankedSearch *search = (YASENS RankedSearch *)arg;
	return search->findDocs(key1, key2, position, tf);
}

/**
 * Evaluate a query. Search the terms and locate all documents
 * containing those terms.
 */
bool
YASENS RankedSearch::evaluateQuery()
{
	matches = 0;
	curterm = 0;

	/* Process each term in sequence */
	for (int i = 0; i < termcount; i++) {
	 	ys_uchar_t key[YS_MAXKEYSIZE+1];
		ys_uchar_t *cp = terms[i].text;

		key[0] = strlen((const char *)cp);
		memcpy(key+1, cp, key[0]+1);
		if (collection->isStemmed())
			stem(key);
		curterm++;
		ys_btree_iterate( collection->getIndex(), key, ys_find_docs, this );
	}
	return true;
}

/**
 * Output results of the query. If all_terms was specified, ignore matches
 * that didnot contain all the terms. If ranked query was requested,
 * sort results by rank before sending them to the user.
 */
YASENS SearchResultSet *
YASENS RankedSearch::executeQuery()
{
	resultSet = new YASENS RankedSearchResultSet();
	if ( resultSet == 0 ) {
		snprintf(message, sizeof message,
			"Error: cannot create result tree\n");
	}
	else {
		startTimer();
		if (evaluateQuery() && matches > 0)
			resultSet->sortByRank();
		stopTimer();
		resultSet->setElapsedTime(elapsed);
	}
	return resultSet;
}

/**
 * Add a query term to the list. If the term has already been added,
 * we increment the term.qtf - and update maximum query term frequency.
 */
void
YASENS RankedSearch::saveTerm(const ys_uchar_t *word)
{
	// printf("Term: %s\n", word);
	if (strcmp((const char *) word, "and") == 0 ||
	    strcmp((const char *) word, "or") == 0 ||
	    strcmp((const char *) word, "not") == 0)
		return;
	
	int i;
	for (i = 0; i < termcount; i++) {
		if (strcmp((const char *)terms[i].text, (const char *)word) == 0) {
			terms[i].qtf++;
			if (terms[i].qtf > qmf)
				qmf = terms[i].qtf;
			return;
		}
	}
	if (termcount < YS_SEARCH_MAXTERMS) {
		strncpy((char *)terms[termcount].text, (const char *)word,
			sizeof terms[termcount].text);
		terms[termcount].qtf = 1;
		terms[termcount].qtw = 0.0;
		terms[termcount].idf = 0.0;
		termcount++;
		if (qmf == 0)
			qmf = 1;
	}
}

bool
YASENS RankedSearch::parseQuery()
{
	YASENS StringTokenizer st;
	st.setInput(input);
	const ys_uchar_t *term = st.nextToken();
	while (term != 0) {
		saveTerm(term);
		term = st.nextToken();
	}
	term = st.endInput();
	if (term != 0)
		saveTerm(term);
	return true;
}

