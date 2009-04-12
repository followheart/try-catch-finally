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
* DM 19-01-00 created from extract.c
*             added support for yase.docptrs.
* DM 21-04-00 started work on support for documents within files.
*    22-04-00 renamed functions, data structures
* DM 25-11-00 added support for logicalname, anchor
* DM 19-02-02 Changed delimiter from % to ^
*/

#include "docdb.h"
#include "ystdio.h"

struct ys_docdb_info_t {
	ys_docnum_t numdocs;
	ys_docnum_t numfiles;
	ys_doccnt_t maxdoctermfreq;
	ys_doccnt_t maxtermfreq;
	ys_bool_t   stemmed;
};

struct ys_docdb_t {
	ys_file_t *yasefiles;		/* yase.files */
	ys_file_t *yasedocs;		/* yase.docs */
	ys_file_t *yasedocptrs;		/* yase.docptrs */
	ys_file_t *yaseinfo;		/* yase.info */
	const char *rootpath;
	ys_docnum_t docnum;
	struct ys_docdb_info_t info;
};

/* Records in yase.docptrs are marked with a boolean flag */
enum {
	MULTIDOC_FILE = 1,
	SINGLEDOC_FILE = 0
};

/* Delimiter character used to separate fields. Cannot use : as urls
 * may contain them.
 */
static char DELIMITER = '^';

/**
 * Opens a document database (except the index).
 * @param   dbpath location of the database files.
 * @param   mode   "r" (read) or "w" (write).
 * @returns        handle
 */ 
ys_docdb_t *
ys_dbopen(const char *dbpath, const char *mode, const char *rootpath)
{
	ys_docdb_t *db;
	char path[1024];

	db = (ys_docdb_t *) calloc(1, sizeof *db);
	if (db == 0) {
		perror("calloc");
		fprintf(stderr, "Run out of memory\n");
		return 0;
	}

	snprintf(path, sizeof path, "%s/%s", dbpath, "yase.files");
	db->yasefiles = ys_file_open( path, mode, 0 );
	snprintf(path, sizeof path, "%s/%s", dbpath, "yase.docs");
	db->yasedocs = ys_file_open( path, mode, 0 );
	snprintf(path, sizeof path, "%s/%s", dbpath, "yase.docptrs");
	db->yasedocptrs = ys_file_open( path, mode, 0 );
	snprintf(path, sizeof path, "%s/%s", dbpath, "yase.info");
	db->yaseinfo = ys_file_open( path, mode, 0 );

	if (db->yasefiles == 0 || db->yasedocs == 0 || 
		db->yasedocptrs == 0 || db->yaseinfo == 0) {
		fprintf(stderr, "Failed to open yase.files, "
			"yase.docs, yase.docptrs or yase.info\n");
		ys_dbclose(db);
		db = 0;;
	}		
	else {
		db->rootpath = rootpath;
		db->docnum = 0;
	}
	return db;	
}

/**
 * Closes an open database
 * @param    db   handle of open database
 * @returns       0 if successful, -1 otherwise
 */
int
ys_dbclose(ys_docdb_t *db)
{
	int rc = 0;
	if (db != 0) {
		if (db->yasefiles != 0) {
			if (ys_file_close(db->yasefiles) != 0) {
				fprintf(stderr, "Error closing yase.files\n");
				rc = -1;
			}
		}
		if (db->yasedocs != 0) {
			if (ys_file_close(db->yasedocs) != 0) {
				fprintf(stderr, "Error closing yase.docs\n");
				rc = -1;
			}
		}
		if (db->yasedocptrs != 0) {
			if (ys_file_close(db->yasedocptrs) != 0) {
				fprintf(stderr, "Error closing yase.docptrs\n");
				rc = -1;
			}
		}
		if (db->yaseinfo != 0) {
			if (ys_file_close(db->yaseinfo) != 0) {
				fprintf(stderr, "Error closing yase.info\n");
				rc = -1;
			}
		}
		free(db);
	}
	return rc;
}

/**
 * Add a file to the document database.
 * @param   db       handle to the database
 * @param   docfile  structure ys_docdata_t containing information regarding
 *                   file to be added
 * @returns          0 for success, -1 otherwise
 */
int
ys_dbaddfile(ys_docdb_t *db, ys_docdata_t *docfile)
{
	ys_filepos_t pos;
	const char *filename = docfile->filename;
	const char *logicalname = docfile->logicalname;
	const char *rootpath;

	/* At least the file should have a name that can be retrieved */
	if (strlen(logicalname) == 0) {
		fprintf(stderr, "Document file data is corrupt\n");
		fprintf(stderr, "Filename='%s'\n", filename);
		fprintf(stderr, "Logicalname='%s'\n", logicalname);
		return -1;
	}

	/* If this file is relative to rootpath, we remove the common part */
	rootpath = db->rootpath;
	if (rootpath != 0) {
		size_t len = strlen(rootpath);
		if (strncmp(logicalname, rootpath, len) == 0)
			logicalname += len;
	}

	ys_file_getpos(db->yasefiles, &pos);
	ys_file_printf(db->yasefiles, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c\n",
		logicalname,
		DELIMITER, docfile->title, DELIMITER, docfile->author, 
		DELIMITER, docfile->type, DELIMITER, docfile->size, 
		DELIMITER, docfile->datecreated, 
		DELIMITER, docfile->keywords, DELIMITER);
	if (ys_file_error(db->yasefiles)) {
		fprintf(stderr, "Unable to write to yase.files\n");
		return -1;
	}
	docfile->offset = pos;
	return 0;
}

/**
 * Add a document to the document database.
 * @param db      handle to the open database
 * @param docfile file to which the document belongs
 * @param doc     document to be added
 * @returns       0 if successful, -1 if not
 */
int
ys_dbadddoc(ys_docdb_t *db, ys_docdata_t *docfile, ys_docdata_t *doc)
{
	ys_filepos_t pos;

	if (strlen(doc->title) == 0) {
		fprintf(stderr, "Document does not have a title\n");
		strncpy(doc->title, "Untitled", sizeof doc->title);
	}
	ys_file_getpos(db->yasedocs, &pos);
	ys_file_write(&docfile->offset, 1, sizeof docfile->offset, db->yasedocs);
	ys_file_printf(db->yasedocs, "%s%c%s%c%s%c\n", 
		doc->title, DELIMITER, doc->anchor, DELIMITER,
		doc->keywords, DELIMITER);
	if (ys_file_error(db->yasedocs)) {
		fprintf(stderr, "Unable to write to yase.docs\n");
		return -1;
	}
	doc->offset = pos;
	return 0;
}

/**
 * Add a file/document link to the database.
 * @param db   	  handle to the opne database
 * @param docfile file to be added
 * @param doc     document to be added
 * @param dn      placeholder for document number to be retrieved
 * @returns       0 if successful, -1 otherwise
 */
int
ys_dbadddocptr(ys_docdb_t *db, ys_docdata_t *docfile, ys_docdata_t *doc,
	ys_docnum_t *dn)
{
	static ys_docnum_t docnum = 0;
	ys_filepos_t pos;
	char type;
	float wt = 0.0;
	ys_filepos_t offset;

	if (doc != 0) {
		pos = doc->offset;
		type = MULTIDOC_FILE;
	}
	else {
		pos = docfile->offset;
		type = SINGLEDOC_FILE;
	}	
	offset = docnum * (sizeof offset + sizeof type + sizeof wt);
	ys_file_setpos(db->yasedocptrs, &offset);
	ys_file_write(&type, 1, sizeof type, db->yasedocptrs);
	ys_file_write(&pos, 1, sizeof pos, db->yasedocptrs);
	ys_file_write(&wt, 1, sizeof wt, db->yasedocptrs);
	if (ys_file_error(db->yasedocptrs)) {
		fprintf(stderr, "Unable to write to yase.docptrs\n");
		return -1;
	}
	*dn = db->docnum = docnum++;
	return 0;
}

/**
 * Record the calculated document weight for a particular document
 * @param  db      Document database
 * @param  docnum  Number identifying the document
 * @param  wt      The calculated weight
 * @returns        0 for success, -1 on failure
 */
int
ys_dbadddocweight(ys_docdb_t *db, ys_docnum_t docnum, float wt)
{
	float f = (float) wt;
	ys_filepos_t pos;
	char type;

	pos = docnum * (sizeof pos + sizeof type + sizeof f);
	pos += (sizeof type + sizeof pos);
	ys_file_setpos(db->yasedocptrs, &pos);
	ys_file_write(&f, 1, sizeof f, db->yasedocptrs);
	if (ys_file_error(db->yasedocptrs)) {
		fprintf(stderr, "Unable to write to yase.docptrs\n");
		return -1;
	}
	return 0;
}

/**
 * Retrieve the document weight for a document
 * @param   db      document database handle
 * @param   docnum  number identifying the document 
 * @param   wt      placeholder for the weight to be retrieved into
 * @returns         0 on success, -1 on failure
 */
int
ys_dbgetdocweight(ys_docdb_t *db, ys_docnum_t docnum, float *wt)
{
	float f;
	ys_filepos_t pos;
	char type;

	pos = docnum * (sizeof pos + sizeof type + sizeof f);
	pos += (sizeof type + sizeof pos);
	ys_file_setpos(db->yasedocptrs, &pos);
	ys_file_read(&f, 1, sizeof f, db->yasedocptrs);
	if (ys_file_error(db->yasedocptrs)) {
		fprintf(stderr, "Unable to read from yase.docptrs\n");
		return -1;
	}
	*wt = f;
	return 0;
}

ys_docnum_t
ys_dbnumdocs(ys_docdb_t *db)
{
	return db->docnum+1;
}

/** 
 * Similar to strtok except that the token is copied to a buffer and
 * adjacent delimiters cause multiple tokens to be returned.
 */
static const char *
ys_nexttok(char *inp, char delimiter, char *out, size_t outsize)
{
    static char *hold = "";
    size_t size_count;
    char *output = out;

    /* basic check for correct parameters */
    if (out == NULL || outsize <= 0) {
        return "";
    }

    if ( inp == NULL ) {
        inp = hold;
    }

    if ( *inp == 0 ) {
        return "";
    }

    if ( *inp == delimiter ) {
        *out = 0;
        inp++;
        hold = inp;
        return output;
    }

    size_count = 0;
    while ( *inp != delimiter && *inp != '\n' && *inp != 0 ) {
        if( size_count < (outsize -1)) {
	    *out++ = *inp++;
	    size_count++;
	}
        else {
            break;
        }
    }

    *out = 0;

    /* Remove trailing spaces */
    while (out > output && out[-1] == ' ') {
        out--;
        *out = 0;
    }

    /* skip extra characters */
    while ( *inp != delimiter && *inp != '\n' && *inp != 0 )
        inp++;

    /* skip delimiter */
    if ( *inp == delimiter || *inp == '\n' )
        inp++;
    
    hold = inp;

    return output;
}

static int
ys_read_docdata( ys_docdb_t *db, ys_docdata_t *data, ys_filepos_t pos )
{
	char buf[4096];
	ys_filepos_t offset;

	ys_file_setpos(db->yasedocs, &pos);
	ys_file_read(&offset, 1, sizeof offset, db->yasedocs);
	ys_file_gets(buf, sizeof buf, db->yasedocs);
	if (ys_file_error(db->yasedocs)) {
		fprintf(stderr, "Unable to read from yase.docs\n");
		return -1;
	}

	ys_nexttok(buf, DELIMITER, data->title, sizeof data->title);
	ys_nexttok(0, DELIMITER, data->anchor, sizeof data->anchor);
	ys_nexttok(0, DELIMITER, data->keywords, sizeof data->keywords);
	data->offset = offset;
	if (strlen(data->title) == 0) {
		fprintf(stderr, "Document data is corrupt\n");
		return -1;
	}
	return 0;
}

static int
ys_read_docfiledata( ys_docdb_t *db, ys_docdata_t *data, ys_filepos_t pos )
{
	char buf[4096];

	ys_file_setpos(db->yasefiles, &pos);
	ys_file_gets(buf, sizeof buf, db->yasefiles);
	if (ys_file_error(db->yasefiles)) {
		fprintf(stderr, "Unable to read from yase.files\n");
		return -1;
	}

	ys_nexttok(buf, DELIMITER, data->logicalname, sizeof data->logicalname);
	ys_nexttok(0, DELIMITER, data->title, sizeof data->title);
	ys_nexttok(0, DELIMITER, data->author, sizeof data->author);
	ys_nexttok(0, DELIMITER, data->type, sizeof data->type);
	ys_nexttok(0, DELIMITER, data->size, sizeof data->size);
	ys_nexttok(0, DELIMITER, data->datecreated, sizeof data->datecreated);
	ys_nexttok(0, DELIMITER, data->keywords, sizeof data->keywords);
	data->offset = 0;
	if (strlen(data->logicalname) == 0) {
		fprintf(stderr, "Document file data is corrupt\n");
		return -1;
	}
	return 0;
}

/**
 * Reads details of a document reference.
 */
int
ys_dbgetdocumentref( ys_docdb_t *db, ys_docnum_t docnum, ys_docdata_t *docfile,
	ys_docdata_t *doc )
{
	char type;
	ys_filepos_t pos;
	float wt;

	memset(docfile, 0, sizeof(ys_docdata_t));
	memset(doc, 0, sizeof(ys_docdata_t));
	pos = docnum * (sizeof pos + sizeof type + sizeof wt);
	ys_file_setpos(db->yasedocptrs, &pos);
	ys_file_read(&type, 1, sizeof type, db->yasedocptrs);
	ys_file_read(&pos, 1, sizeof pos, db->yasedocptrs);
	ys_file_read(&wt, 1, sizeof wt, db->yasedocptrs);
	if (ys_file_error(db->yasedocptrs)) {
		fprintf(stderr, "Unable to read from yase.docptrs\n");
		return -1;
	}
	if (type == MULTIDOC_FILE) {
		if (ys_read_docdata( db, doc, pos ) != 0)
			return -1;
		pos = doc->offset;
	}
	if (ys_read_docfiledata( db, docfile, pos ) != 0)
		return -1;
	return 0;
}

int
ys_dbaddinfo(ys_docdb_t *db, ys_docnum_t numdocs, ys_docnum_t numfiles,
	ys_doccnt_t maxdoctermfreq, ys_doccnt_t maxtermfreq, ys_bool_t stemmed)
{
	ys_filepos_t pos = 0;
	db->info.numdocs = numdocs;
	db->info.numfiles = numfiles;
	db->info.maxdoctermfreq = maxdoctermfreq;
	db->info.maxtermfreq = maxtermfreq;
	db->info.stemmed = stemmed;
	ys_file_setpos(db->yaseinfo, &pos);
	ys_file_write(&db->info, 1, sizeof db->info, 
		db->yaseinfo);
	if (ys_file_error(db->yaseinfo)) {
		fprintf(stderr, "Unable to write to yase.info\n");
		return -1;
	}
	return 0;
}

int
ys_dbgetinfo(ys_docdb_t *db, ys_docnum_t *numdocs, ys_docnum_t *numfiles,
	ys_doccnt_t *maxdoctermfreq, ys_doccnt_t *maxtermfreq, ys_bool_t *stemmed)
{
	ys_filepos_t pos = 0;
	ys_file_setpos(db->yaseinfo, &pos);
	ys_file_read(&db->info, 1, sizeof db->info, 
		db->yaseinfo);
	*numdocs = db->info.numdocs;
	*numfiles = db->info.numfiles;
	*maxdoctermfreq = db->info.maxdoctermfreq;
	*maxtermfreq = db->info.maxtermfreq;
	*stemmed = db->info.stemmed;
	if (ys_file_error(db->yaseinfo)) {
		fprintf(stderr, "Unable to read from yase.info\n");
		return -1;
	}
	return 0;
}

