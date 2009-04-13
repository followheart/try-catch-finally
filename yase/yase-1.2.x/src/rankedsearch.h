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

#ifndef rankedsearch_h
#define rankedsearch_h

#include "search.h"
#include "avl3.h"

YASE_NS_BEGIN

struct SearchTerm {
	ys_uchar_t text[YS_TERM_LEN+1];   /* term */
	int qtf;                 /* number of times term appears
	                          * a query.
	                          */
	float qtw;               /* query term's weight */
	float idf;               /* term weight in the collection */
};

class RankedSearchResultSet;

class RankedSearch : public Search {
private:
	int termcount;                           /* number of terms in query */
	SearchTerm terms[YS_SEARCH_MAXTERMS];    /* query terms */
	int matches;
	int qmf;                                 /* max frequency within query */
	int curterm;
	RankedSearchResultSet *resultSet;
public:
	RankedSearch(YASENS Collection *collection);
	~RankedSearch();
	bool selectDocument(ys_docnum_t docnum, ys_doccnt_t dtf);
	void calculateWeight(ys_doccnt_t tf, ys_docnum_t N);
	bool findDocs(ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t position, 
		ys_doccnt_t tf);
	bool evaluateQuery();
	void saveTerm(const ys_uchar_t *word);
public:
	bool parseQuery();
	SearchResultSet *executeQuery();
};

YASE_NS_END

#endif

