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

// Created 7-12-02

#include "collection.h"

ys_btree_t *
YASENS Collection::openIndex(const char *home, const char *mode)
{
	char path[1024];
	snprintf(path, sizeof path, "%s/%s", home, "yase.btree");
	ys_btree_t *tree = ys_btree_open_mode( path, mode );
	return tree;
}

YASENS PostFile *
YASENS Collection::openPostings(const char *home, const char *mode)
{
	char path[1024];
	snprintf(path, sizeof path, "%s/%s", home, "yase.postings");
	YASENS PostFile *postings_file = new YASENS PostFile();
	if (postings_file->open( path, mode ) != 0) {
		delete postings_file;
		return 0;
	}
	return postings_file;
}

/**
 * Open a document collection.
 */
int
YASENS Collection::open(const char *home, const char *mode)
{
	tree = openIndex( home, mode );
	if (tree == 0) {
		snprintf(errmsg, sizeof errmsg, "Error: cannot open YASE Index\n");
		return -1;
	}

	docdb = ys_dbopen( home, mode, "" );
	if (docdb == NULL) {
		snprintf(errmsg, sizeof errmsg, "Error: cannot open YASE files\n");
		return -1;
	}
	else {
		ys_docnum_t N = 0, numfiles;
		ys_doccnt_t maxdoctermfreq, maxtermfreq;
		ys_bool_t stemmed;
		ys_dbgetinfo(docdb, &N, &numfiles,
			&maxdoctermfreq, &maxtermfreq, &stemmed);
		this->N = N;
		this->stemmed = stemmed;
		this->maxtf = maxtermfreq;
		this->maxdtf = maxdoctermfreq;
		this->numFiles = numfiles;
	}

	postings_file = openPostings( home, mode );
	if (postings_file == 0) {
		snprintf(errmsg, sizeof errmsg, "Error: cannot open Postings file\n" );
		return -1;
	}

	return 0;
}

/**
 * Close a document collection.
 */
void
YASENS Collection::close()
{
	if (tree != NULL) 
		ys_btree_close( tree );
	if (postings_file != NULL) 
		delete postings_file;
	if (docdb != NULL) 
		ys_dbclose(docdb);
	tree = 0;
	postings_file = 0;
	docdb = 0;
}

YASENS Collection::Collection()
{
	tree = 0;
	postings_file = 0;
	docdb = 0;
	N = 0;
	stemmed = false;
	maxtf = 0;
	maxdtf = 0;
	numFiles = 0;
}

YASENS Collection::~Collection()
{
	close();
}

