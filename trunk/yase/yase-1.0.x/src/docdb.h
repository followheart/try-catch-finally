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
#ifndef docdb_h
#define docdb_h

#include "yase.h"

enum {
	YS_FILENAME_LEN = 1024,
	YS_TITLE_LEN = 1024,
	YS_ANCHOR_LEN = 512,
	YS_AUTHOR_LEN = 64,
	YS_SIZE_LEN = 16,
	YS_TYPE_LEN = 32,
	YS_DATECREATED_LEN = 32,
	YS_KEYWORDS_LEN = 1024
};

typedef struct {
	char filename[YS_FILENAME_LEN];	/* physical name of indexed file */
	char logicalname[YS_FILENAME_LEN];	/* logical name - can be a url */
	char title[YS_TITLE_LEN];							/* title */
	char anchor[YS_ANCHOR_LEN];	/* in case of sub-documents, an 
					 * anchor can be specified 
					 */
	char author[YS_AUTHOR_LEN];	/* author of the document */
	char size[YS_SIZE_LEN];		/* document size */
	char type[YS_TYPE_LEN];		/* document type */
	char datecreated[YS_DATECREATED_LEN]; /* date of creation - currently meaningless */
	char keywords[YS_KEYWORDS_LEN];	/* unused */

	ys_filepos_t offset;		/* internal use */
} ys_docdata_t;

typedef struct ys_docdb_t ys_docdb_t;

extern ys_docdb_t *
ys_dbopen(const char *dbpath, const char *mode, const char *rootpath);

extern int 
ys_dbclose(ys_docdb_t *db);

extern int 
ys_dbaddfile(ys_docdb_t *db, ys_docdata_t *docfile);

extern int 
ys_dbadddoc(ys_docdb_t *db, ys_docdata_t *docfile, ys_docdata_t *doc);

extern int 
ys_dbadddocptr(ys_docdb_t *db, ys_docdata_t *docfile,
		ys_docdata_t *doc, ys_docnum_t *dn); 

extern int 
ys_dbgetdocumentref(ys_docdb_t *db, ys_docnum_t docnum, 
	ys_docdata_t *docfile, ys_docdata_t *doc);

extern int 
ys_dbadddocweight(ys_docdb_t *db, ys_docnum_t docnum, float wt);

extern int 
ys_dbgetdocweight(ys_docdb_t *db, ys_docnum_t docnum, float *wt);

extern ys_docnum_t
ys_dbnumdocs(ys_docdb_t *db);

extern int
ys_dbaddinfo(ys_docdb_t *db, ys_docnum_t numdocs, ys_docnum_t numfiles,
	ys_doccnt_t maxdoctermfreq, ys_doccnt_t maxtermfreq, ys_bool_t stemmed);

extern int
ys_dbgetinfo(ys_docdb_t *db, ys_docnum_t *numdocs, ys_docnum_t *numfiles,
	ys_doccnt_t *maxdoctermfreq, ys_doccnt_t *maxtermfreq, ys_bool_t *stemmed);

#endif
