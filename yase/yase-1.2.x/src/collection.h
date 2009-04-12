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

#ifndef collection_h
#define collection_h

#include "yase.h"
#include "btree.h"
#include "postfile.h"
#include "docdb.h"

YASE_NS_BEGIN

class Collection {

private:
	YASENS PostFile *postings_file;
	ys_docdb_t *docdb;
	ys_btree_t *tree;
	ys_bool_t stemmed;
	ys_doccnt_t N;
	ys_doccnt_t maxtf;
	ys_doccnt_t maxdtf;
	ys_doccnt_t numFiles;
	char errmsg[256];

public:
	Collection();
	~Collection();

	int open(const char *home, const char *mode);
	void close();

	ys_btree_t *getIndex() const { return tree; }
	ys_docdb_t *getDocDb() const { return docdb; }
	YASENS PostFile *getPostFile() const { return postings_file; }
	const char *getError() const { return errmsg; }
	ys_bool_t isStemmed() const { return stemmed; };
	ys_doccnt_t getN() const { return N; }
	ys_doccnt_t getMaxTf() const { return maxtf; }
	ys_doccnt_t getMaxDtf() const { return maxdtf; }
	ys_doccnt_t getNumFiles() const { return numFiles; }
	static ys_btree_t *openIndex(const char *home, const char *mode);
	static YASENS PostFile *openPostings(const char *home, const char *mode);
};

YASE_NS_END

#endif
