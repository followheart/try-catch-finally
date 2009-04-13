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
// 10 Dec 2002: Created

#ifndef search_h
#define search_h

#include "yase.h"
#include "tokenizer.h"
#include "collection.h"
#include "util.h"

enum {
	YS_SEARCH_MAXTERMS = YS_QUERY_MAXTERMS
};

YASE_NS_BEGIN

class SearchResultItem {
protected:
	ys_docnum_t docnum;
	float rank;
	int hits;
	SearchResultItem() { docnum = 0; rank = 0.0; hits = 0; }
public:
	virtual ~SearchResultItem() {}
	double getScore() const { return rank; }
	int getHits() const { return hits; }
	ys_docnum_t getDocnum() const { return docnum; }
	void dump(FILE *file) {
		fprintf(file, "{ Doc(%lu), Score(%.2f), Hits(%d) }\n", docnum, rank, hits);
	}
};

class SearchResultSet {
protected:
	int count;
	double elapsed;
	SearchResultSet() { count = 0; elapsed = 0.0; }
public:
	virtual ~SearchResultSet() {}
	virtual SearchResultItem *getNext() = 0;
	int getCount() const { return count; }
	double getElapsedTime() const { return elapsed; }
	virtual bool contains(ys_docnum_t docnum) { return false; }
};

class Search {
protected:
	ys_uchar_t *input;
	YASENS Collection *collection;
	char message[1024];
	struct timeval start;
	struct timeval stop;
	double elapsed;
protected:
	Search(YASENS Collection *collection);
	void startTimer() { gettimeofday(&start, (struct timezone *)0); }
	void stopTimer() { 
		gettimeofday(&stop, (struct timezone *)0); 
		elapsed = ys_calculate_elapsed_time(&start, &stop);
	}
private:
	Search(const Search&);
	Search& operator=(const Search&);
public:
	enum {
		SM_RANKED = 1,
		SM_BOOLEAN = 2
	};
	virtual ~Search();
	virtual void reset();
	void addInput(const ys_uchar_t *text);
	virtual SearchResultSet* executeQuery() = 0;
	virtual bool parseQuery() = 0;
	double getElapsedTime() const { return elapsed; }
	static Search* createSearch(YASENS Collection *collection, int method);
};

YASE_NS_END

#endif

