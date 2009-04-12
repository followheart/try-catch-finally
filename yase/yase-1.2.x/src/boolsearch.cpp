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

// 25-12-02: C++ version started on
// 31-12-02: Revised (first working version)
// 02-01-03: Revised to use re-written tokenizer classes

#include "boolsearch.h"

enum {
	T_LPAREN = 1,
	T_RPAREN,
	T_AND,
	T_OR,
	T_NOT,
	T_TERM,
	T_EOI
};

YASE_NS_BEGIN

class BoolSearchResultItem : public SearchResultItem {
public:
	void setDocnum(ys_docnum_t docnum) {
		this->docnum = docnum;
	}
};

class BoolSearchResultSet : public SearchResultSet {
	ys_bitset_t *bitset;
	YASENS BitSetIterator* iterator;
	BoolSearchResultItem item;
public:
	BoolSearchResultSet(ys_bitset_t *bitset, double e) {
		this->bitset = bitset;
		elapsed = e;
		iterator = new YASENS BitSetIterator(bitset);
		count = iterator->getCount();  
	}
	~BoolSearchResultSet() {
		delete iterator;
	}
	SearchResultItem *getNext() {
		int doc = iterator->getNext();
		if (doc == -1)
			return 0;
		item.setDocnum(doc);
		return &item;
	}
	bool contains(ys_docnum_t docnum)
	{
		return ys_bs_ismember(bitset, docnum) != 0;
	}
};

YASE_NS_END

YASENS BoolSearch::BoolSearch(YASENS Collection *collection)
	: YASENS Search(collection)
{
	tokptr = 0;
	curtok = 0;
	bitset = 0;
	resultSet = 0;
	termbuf[0] = 0;
}

YASENS BoolSearch::~BoolSearch()
{
}

int
YASENS BoolSearch::getToken()
{
	ys_uchar_t *cp = tokptr;
	ys_uchar_t ch;
	int tok;
	int len = 0;

	termbuf[0] = 0;

	while (true) {
		ch = *cp;
		if (ch == '(') {
			tok = T_LPAREN;
			strcpy((char *)termbuf, "(");
			cp++;
			break;
		}
		else if (ch == ')') {
			tok = T_RPAREN;
			strcpy((char *)termbuf, ")");
			cp++;
			break;
		}
		else if (ch == 0) {
			tok = T_EOI;
			break;
		}
		else if (tokenizer.isWordChar(ch)) {
			tokenizer.reset();
			int state = YASENS CharTokenizer::TC_AGAIN;
			do {
				state = tokenizer.addCh(*cp);
				if (state == YASENS CharTokenizer::TC_WORD_COMPLETED)
					break;
				cp++;
			} while (*cp != 0);
			if (*cp == 0 && state == YASENS CharTokenizer::TC_AGAIN)
				state = tokenizer.endInput();
			if (state == YASENS CharTokenizer::TC_WORD_COMPLETED) {
				strncpy((char *)termbuf, (const char *)tokenizer.getWord(), 
					sizeof termbuf);
				if (strcmp((const char *)termbuf, "and") == 0)
					tok = T_AND;
				else if (strcmp((const char *)termbuf, "or") == 0)
					tok = T_OR;
				else if (strcmp((const char *)termbuf, "not") == 0)
					tok = T_NOT;
				else
					tok = T_TERM;
			}
			else
				tok = T_EOI;
			break;
		}
		else {
			cp++;
		}
	}
	tokptr = cp;
	curtok = tok;

	if (Ys_debug > 0)
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
		printf("TOKEN = %s(%s)\n", tokname, termbuf);
	}
	return tok;
}

void
YASENS BoolSearch::matchToken(int tok)
{
	if (curtok == tok)
		getToken();
	else {
		// ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Syntax error: Unexpected token '%s'\n", query->termbuf);
		curtok = T_EOI;
	}
}

/**
 * Parse OR expressions.
 */
ys_bitset_t *
YASENS BoolSearch::parseOrExpr()
{
	ys_bitset_t *bs1, *bs2;

	bs1 = parseAndExpr();
	while (bs1 != 0 && curtok == T_OR) {
		matchToken(T_OR);
		bs2 = parseAndExpr();
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
ys_bitset_t *
YASENS BoolSearch::parseAndExpr()
{
	ys_bitset_t *bs1, *bs2;

	bs1 = parseNotExpr();
	while (bs1 != 0 && curtok == T_AND) {
		matchToken(T_AND);
		bs2 = parseNotExpr();
		if (bs2 == 0) {
			return 0;
		}
		/* printf("performing AND\n"); */
		ys_bs_intersect(bs1,bs2);
		ys_bs_destroy(bs2);
	}
	return bs1;
}

ys_bitset_t *
YASENS BoolSearch::parseNotExpr()
{
	ys_bitset_t *bs1 = 0;

	if (curtok == T_NOT) {
		matchToken(T_NOT);
		bs1 = parseNotExpr();
		if (bs1 == 0) {
			return 0;
		}
		/* printf("performing NOT\n"); */
		ys_bs_complement(bs1);
	}
	else if (curtok == T_LPAREN) {
		matchToken(T_LPAREN);
		bs1 = parseExpr();
		matchToken(T_RPAREN);
	}
	else if (curtok == T_TERM) {
		bs1 = ys_bs_alloc(collection->getN());
		bitset = bs1;
		findTerms();
		bitset = 0;
		matchToken(T_TERM);
	}
	else {
		// ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Syntax error: Unexpected input\n", query->termbuf);
		bs1 = 0;
	}
		
	return bs1;
}

ys_bitset_t *
YASENS BoolSearch::parseExpr()
{
	return parseOrExpr();
}

bool 
YASENS BoolSearch::selectDocument(ys_docnum_t docnum, ys_doccnt_t dtf)
{
	assert(bitset != 0);
	/* Add document to the bitmap for this term */
	ys_bs_addmember(bitset, docnum);
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
	YASENS BoolSearch *search = (YASENS BoolSearch *)arg1;
	return search->selectDocument(docnum, dtf);
}

/**
 * Called for each term found in the index. Retrieves all documents
 * for the term and saves them in the result tree.
 * Note that this function may be called to process exact as well as partial
 * matches. It is called for all terms after the first term that is found
 * - until it returns BOOL_FALSE. 
 */
bool 
YASENS BoolSearch::findDocs(ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t position, 
					   ys_doccnt_t tf)
{
	if (key1[0] != key2[0])
		return false;

	if (strncmp( (const char *)key1+1, (const char *)key2+1, key1[0] ) == 0) {
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
	YASENS BoolSearch *search = (YASENS BoolSearch *)arg;
	return search->findDocs(key1, key2, position, tf);
}

void
YASENS BoolSearch::findTerms()
{
 	ys_uchar_t key[YS_MAXKEYSIZE+1];
	ys_uchar_t *cp = termbuf;
		
	/* printf("searching term %s\n", query->termbuf); */
	key[0] = strlen((const char *)cp);
	memcpy(key+1, cp, key[0]+1);
	if (collection->isStemmed())
		stem(key);
	ys_btree_iterate( collection->getIndex(), key, ys_find_docs, this );
}


YASENS SearchResultSet *
YASENS BoolSearch::executeQuery()
{
	startTimer();
	tokptr = input;
	getToken();
	/* printf("parsing expr\n"); */
	bitset = 0;
	if (curtok != T_EOI) {
		bitset = parseExpr();
		if (curtok != T_EOI) {
			// ys_query_output_message(query->outp, YS_QRY_MSG_ERROR, "Syntax error in expression at '%s'\n", query->termbuf);
			bitset = 0;
		}
	}
	stopTimer();
	return new YASENS BoolSearchResultSet(bitset, elapsed);
}

