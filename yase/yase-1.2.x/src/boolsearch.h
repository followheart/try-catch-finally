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

#ifndef boolsearch_h
#define boolsearch_h

#include "search.h"
#include "bitset.h"
#include "stem.h"
#include "tokenizer.h"

YASE_NS_BEGIN

class BoolSearchResultSet;

class BoolSearch : public Search {
private:
	ys_uchar_t *tokptr;			  /* boolean query lexer */
	int curtok;				  /* boolean query current token */
	ys_bitset_t *bitset;			  /* boolean query bitset */
	ys_uchar_t termbuf[YS_TERM_LEN+1];	  /* boolean query term */
	BoolSearchResultSet *resultSet;
	YASENS CharTokenizer tokenizer;
public:
	BoolSearch(YASENS Collection *collection);
	~BoolSearch();
	bool parseQuery() { return true; }
	SearchResultSet *executeQuery();
private:
	int getToken();
	void matchToken(int tok);
	ys_bitset_t *parseOrExpr();
	ys_bitset_t *parseAndExpr();
	ys_bitset_t *parseNotExpr();
	ys_bitset_t *parseExpr();
	void findTerms();
public:
	bool selectDocument(ys_docnum_t docnum, ys_doccnt_t dtf);
	bool findDocs(ys_uchar_t *key1, ys_uchar_t *key2, ys_filepos_t position, 
					   ys_doccnt_t tf);

};

YASE_NS_END

#endif

